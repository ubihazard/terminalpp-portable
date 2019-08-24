﻿#include <iostream>

#include "helpers/log.h"
#include "helpers/char.h"
#include "helpers/args.h"
#include "helpers/time.h"

#include "config.h"

#if (defined ARCH_WINDOWS)

#include "directwrite/directwrite_application.h"
#include "directwrite/directwrite_window.h"
// link to directwrite
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#elif (defined ARCH_UNIX)

#include "x11/x11_application.h"
#include "x11/x11_window.h"

#else
#error "Unsupported platform"
#endif

#include "forms/session.h"

#include "ui/widgets/panel.h"
#include "ui/widgets/label.h"
#include "ui/widgets/scrollbox.h"
#include "ui/builders.h"
#include "ui/layout.h"

#include "vterm/terminal.h"
#include "vterm/bypass_pty.h"
#include "vterm/local_pty.h"
#include "vterm/vt100.h"
#include <thread>


#include "stamp.h"
#include "helpers/stamp.h"

#include "helpers/json.h"

using namespace tpp;

void reportError(std::string const & message) {
#if (defined ARCH_WINDOWS)
    helpers::utf16_string text = helpers::UTF8toUTF16(message);
	MessageBox(nullptr, text.c_str(), L"Fatal Error", MB_ICONSTOP);
#else
	std::cout << message << std::endl;
#endif
}


// https://www.codeguru.com/cpp/misc/misc/graphics/article.php/c16139/Introduction-to-DirectWrite.htm

// https://docs.microsoft.com/en-us/windows/desktop/gdi/windows-gdi

// https://docs.microsoft.com/en-us/windows/desktop/api/_gdi/

// https://github.com/Microsoft/node-pty/blob/master/src/win/conpty.cc

/** Terminal++ App Entry Point

    For now creates single terminal window and one virtual terminal. 
 */
#ifdef ARCH_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MARK_AS_UNUSED(hPrevInstance);
	MARK_AS_UNUSED(lpCmdLine);
	MARK_AS_UNUSED(nCmdShow);
	int argc = __argc;
	char** argv = __argv;
	DirectWriteApplication::Initialize(hInstance);
#else
int main(int argc, char* argv[]) {
	X11Application::Initialize();
#endif
	//helpers::Arguments::SetVersion(STR("t++ :" << helpers::Stamp::Stored()));
	//helpers::Arguments::Parse(argc, argv);
	Config const & config = Config::Initialize(argc, argv);
	try {
		//helpers::Log::RegisterLogger(new helpers::StreamLogger(vterm::VT100::SEQ, std::cout));
		//helpers::Log::RegisterLogger(new helpers::StreamLogger(vterm::VT100::SEQ_UNKNOWN, std::cout));
		//helpers::Log::RegisterLogger(new helpers::StreamLogger(vterm::VT100::SEQ_WONT_SUPPORT, std::cout));

		// Create the palette & the pty - TODO this should be more systematic
		vterm::VT100::Palette palette(vterm::VT100::Palette::XTerm256());
		vterm::PTY * pty;
#if (defined ARCH_WINDOWS)
		if (config.sessionPTY() != "bypass") { 
			if (!config::Command.specified())
			    (*config::Command) = std::vector<std::string>{DEFAULT_CONPTY_COMMAND};
		    pty = new vterm::LocalPTY(helpers::Command(*config::Command));
		} else {
		    pty = new vterm::BypassPTY(helpers::Command(*config::Command));
		}
#else
		pty = new vterm::LocalPTY(helpers::Command(*config::Command));
#endif

		// create the session
		Session * session = new Session(pty, & palette);

		// and create the main window

	    tpp::Window * w = Application::Instance()->createWindow(DEFAULT_WINDOW_TITLE, config.sessionCols(), config.sessionRows(), config.fontSize());
		w->setRootWindow(session);
		w->show();

    	Application::Instance()->mainLoop();
		delete session;

	    return EXIT_SUCCESS;
	} catch (helpers::Exception const& e) {
		reportError(STR(e));
	} catch (std::exception const& e) {
		reportError(e.what());
	} catch (...) {
		reportError("Unknown error.");
	} 
	return EXIT_FAILURE;
}
