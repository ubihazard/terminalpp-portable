#ifdef WIN32
#include "helpers/win32.h"
#elif __linux__
#include <unistd.h>
#include <pty.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#endif
#include "helpers/log.h"

#include "local_pty.h"

namespace vterm {

#ifdef WIN32

	LocalPTY::LocalPTY(helpers::Command const & command) :
		command_(command),
		startupInfo_{},
		conPTY_{ INVALID_HANDLE_VALUE },
		pipeIn_{ INVALID_HANDLE_VALUE },
		pipeOut_{ INVALID_HANDLE_VALUE } {
		startupInfo_.lpAttributeList = nullptr; // just to be sure
		createPseudoConsole();
		// start the process
		start();
		// enable the monitoring of the process termination
		monitor();
	}

	LocalPTY::~LocalPTY() {
		terminate();
		free(startupInfo_.lpAttributeList);
	}

	/** Terminates the attached process.
	
	    The termination is asynchronous. If there are any errors returned such as the process already beging terminated, we do not really care. 

		TODO care about possible other errors?
	 */
	void LocalPTY::doTerminate() {
		TerminateProcess(pInfo_.hProcess, -1);
	}

	/** Waits for the attached process to finish.
	
	    Cleans the handles so that any pendion IO operations are cancelled and we do not end up with hanging operations. 

		TODO waiting on a process that has already terminated should produce error, which we can happily ignore, correct? 
	 */
    helpers::ExitCode LocalPTY::doWaitFor() {
		size_t result = WaitForSingleObject(pInfo_.hProcess, INFINITE);
		helpers::ExitCode ec;
		GetExitCodeProcess(pInfo_.hProcess, &ec);
		// we must close the handles so that any pending reads will be interrupted
		CloseHandle(pInfo_.hProcess);
		CloseHandle(pInfo_.hThread);
		if (conPTY_ != INVALID_HANDLE_VALUE)
			ClosePseudoConsole(conPTY_);
		if (pipeIn_ != INVALID_HANDLE_VALUE)
			CloseHandle(pipeIn_);
		if (pipeOut_ != INVALID_HANDLE_VALUE)
			CloseHandle(pipeOut_);
		return ec;
	}

	size_t LocalPTY::sendData(char const * buffer, size_t size) {
		DWORD bytesWritten = 0;
		WriteFile(pipeOut_, buffer, static_cast<DWORD>(size), &bytesWritten, nullptr);
		// TODO check this properly for errors
		ASSERT(bytesWritten == size);
		return bytesWritten;
	}

	size_t LocalPTY::receiveData(char* buffer, size_t availableSize) {
		DWORD bytesRead = 0;
		bool readOk = ReadFile(pipeIn_, buffer, static_cast<DWORD>(availableSize), &bytesRead, nullptr);
		// make sure that if readOk is false nothing was read
		ASSERT(readOk || bytesRead == 0);
		return bytesRead;
	}

	void LocalPTY::resize(unsigned cols, unsigned rows) {
		// resize the underlying ConPTY
		COORD size;
		size.X = cols;
		size.Y = rows;
		ResizePseudoConsole(conPTY_, size);
	}

	void LocalPTY::createPseudoConsole() {
		HRESULT result{ E_UNEXPECTED };
		HANDLE pipePTYIn{ INVALID_HANDLE_VALUE };
		HANDLE pipePTYOut{ INVALID_HANDLE_VALUE };
		// first create the pipes we need, no security arguments and we use default buffer size for now
		if (!CreatePipe(&pipePTYIn, &pipeOut_, nullptr, 0) || !CreatePipe(&pipeIn_, &pipePTYOut, nullptr, 0))
			THROW(helpers::Win32Error("Unable to create pipes for the subprocess"));
		// determine the console size from the terminal we have
		COORD consoleSize{};
		consoleSize.X = 80;
		consoleSize.Y = 25;
		// now create the pseudo console
		result = CreatePseudoConsole(consoleSize, pipePTYIn, pipePTYOut, 0, &conPTY_);
		// delete the pipes on PTYs end, since they are now in conhost and will be deleted when the conpty is deleted
		if (pipePTYIn != INVALID_HANDLE_VALUE)
			CloseHandle(pipePTYIn);
		if (pipePTYOut != INVALID_HANDLE_VALUE)
			CloseHandle(pipePTYOut);
		if (result != S_OK)
			THROW(helpers::Win32Error("Unable to open pseudo console"));
	}

	void LocalPTY::start() {
		// first generate the startup info 
		STARTUPINFOEX startupInfo{};
		size_t attrListSize = 0;
		startupInfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		// allocate the attribute list of required size
		InitializeProcThreadAttributeList(nullptr, 1, 0, &attrListSize); // get size of list of 1 attribute
		startupInfo.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));
		// initialize the attribute list
		if (!InitializeProcThreadAttributeList(startupInfo.lpAttributeList, 1, 0, &attrListSize))
			THROW(helpers::Win32Error("Unable to create attribute list"));
		// set the pseudoconsole attribute
		if (!UpdateProcThreadAttribute(
			startupInfo.lpAttributeList,
			0,
			PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
			conPTY_,
			sizeof(HPCON),
			nullptr,
			nullptr
		))
			THROW(helpers::Win32Error("Unable to set pseudoconsole attribute"));
		std::string cmd = command_.toString();
		// finally, create the process with given commandline
		if (!CreateProcess(
			nullptr,
			&cmd[0], // the command to execute
			nullptr, // process handle cannot be inherited
			nullptr, // thread handle cannot be inherited
			false, // the new process does not inherit any handles
			EXTENDED_STARTUPINFO_PRESENT, // we have extra info 
			nullptr, // use parent's environment
			nullptr, // use parent's directory
			&startupInfo.StartupInfo, // startup info
			&pInfo_ // info about the process
		))
			THROW(helpers::Win32Error(STR("Unable to start process " << cmd)));
	}

#elif __linux__

    LocalPTY::LocalPTY(helpers::Command const & command) :
	    command_(command) {
        start();
		monitor();
	}

	LocalPTY::~LocalPTY() {
		terminate();
		waitFor();
	}

	void LocalPTY::doTerminate() {
		kill(pid_, SIGKILL);
	}

	helpers::ExitCode LocalPTY::doWaitFor() {
		helpers::ExitCode ec;
		pid_t x = waitpid(pid_, &ec, 0);
        // it is ok to see errno ECHILD, happens when process has already been terminated
		if (x < 0 && errno != ECHILD) 
			NOT_IMPLEMENTED; // error
		return ec;
	}

	size_t LocalPTY::sendData(char const* buffer, size_t size) {
		ASSERT(!terminated_) << "Terminated PTY cannot send data";
        int nw = write(pipe_, (void*) buffer, size);
        ASSERT(nw >= 0 && static_cast<unsigned>(nw) == size);
		return size;
    }

	size_t LocalPTY::receiveData(char* buffer, size_t availableSize) {
		if (terminated_)
			return 0;
		int cnt = read(pipe_, (void*)buffer, availableSize);
		// if there is an error while reading, just return it as reading 0 bytes, let the termination handling deal with the cause for the error
		if (cnt < 0)
			return 0;
        return cnt;
    }

	void LocalPTY::resize(unsigned cols, unsigned rows) {
        struct winsize s;
        s.ws_row = rows;
        s.ws_col = cols;
        s.ws_xpixel = 0;
        s.ws_ypixel = 0;
        if (ioctl(pipe_, TIOCSWINSZ, &s) < 0)
            NOT_IMPLEMENTED;
    }

    void LocalPTY::start() {
		// fork & open the pty
		switch (pid_ = forkpty(&pipe_, nullptr, nullptr, nullptr)) {
			// forkpty failed
			case -1:
				LOG << "fork fail";
				UNREACHABLE; // an error
		    // running the child process,
			case 0: {
				setsid();
				if (ioctl(1, TIOCSCTTY, nullptr) < 0)
					UNREACHABLE;
				unsetenv("COLUMNS");
				unsetenv("LINES");
				unsetenv("TERMCAP");
				setenv("TERM", "xterm-256color", /* overwrite */ true);
				setenv("COLORTERM", "truecolor", /* overwrite */ true);

				signal(SIGCHLD, SIG_DFL);
				signal(SIGHUP, SIG_DFL);
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				signal(SIGTERM, SIG_DFL);
				signal(SIGALRM, SIG_DFL);


				char** args = new char* [command_.args().size() + 2];
				args[0] = const_cast<char*>(command_.command().c_str());
				for (size_t i = 0; i < command_.args().size(); ++i)
					args[i + 1] = const_cast<char*>(command_.args()[i].c_str());
				
				args[command_.args().size() + 1] = nullptr;
				// execvp never returns
				execvp(command_.command().c_str(), args);
				UNREACHABLE;
			}
			// continuing the terminal program 
			default:
				break;
		}
    }

#else
#error "Unsupported platform"
#endif




} // namespace vterm