#if (defined ARCH_UNIX && defined RENDERER_NATIVE)

#include "helpers/log.h"
#include "helpers/filesystem.h"

#include "x11_window.h"

#include "x11_application.h"

namespace tpp {

    namespace {

        int X11ErrorHandler(Display * display, XErrorEvent * e) {
            MARK_AS_UNUSED(display);
            LOG() << "X error: " << static_cast<unsigned>(e->error_code);
            return 0;
        }

    } // anonymous namespace

    X11Application::X11Application():
		xDisplay_{nullptr},
		xScreen_{0},
	    xIm_{nullptr}, 
        selectionOwner_{nullptr} {
        XInitThreads();
		xDisplay_ = XOpenDisplay(nullptr);
		if (xDisplay_ == nullptr) 
			THROW(helpers::Exception()) << "Unable to open X display";
		xScreen_ = DefaultScreen(xDisplay_);

		XSetErrorHandler(X11ErrorHandler);

        // create X Input Method, each window then has its own context
		openInputMethod();

		primaryName_ = XInternAtom(xDisplay_, "PRIMARY", false);
		clipboardName_ = XInternAtom(xDisplay_, "CLIPBOARD", false);
		formatString_ = XInternAtom(xDisplay_, "STRING", false);
		formatStringUTF8_ = XInternAtom(xDisplay_, "UTF8_STRING", false);
		formatTargets_ = XInternAtom(xDisplay_, "TARGETS", false);
		clipboardIncr_ = XInternAtom(xDisplay_, "INCR", false);
		wmDeleteMessage_ = XInternAtom(xDisplay_, "WM_DELETE_WINDOW", false);
		xAppEvent_ = XInternAtom(xDisplay_, "_APP_EVT", false);
		motifWmHints_ = XInternAtom(xDisplay_, "_MOTIF_WM_HINTS", false);
		netWmIcon_ = XInternAtom(xDisplay_, "_NET_WM_ICON", false);

		unsigned long black = BlackPixel(xDisplay_, xScreen_);	/* get color black */
		unsigned long white = WhitePixel(xDisplay_, xScreen_);  /* get color white */
		x11::Window parent = XRootWindow(xDisplay_, xScreen_);
		broadcastWindow_ = XCreateSimpleWindow(xDisplay_, parent, 0, 0, 1, 1, 1, white, black);

		if (
			primaryName_ == x11::None ||
			clipboardName_ == x11::None ||
			formatString_ == x11::None ||
			formatStringUTF8_ == x11::None ||
			formatTargets_ == x11::None ||
			clipboardIncr_ == x11::None ||
			wmDeleteMessage_ == x11::None ||
			xAppEvent_ == x11::None ||
			broadcastWindow_ == x11::None ||
			motifWmHints_ == x11::None ||
			netWmIcon_ == x11::None
		) THROW(helpers::Exception()) << "X11 Atoms instantiation failed";

        fcConfig_ = FcInitLoadConfigAndFonts();

        Renderer::Initialize([this](){
            XEvent e;
            memset(&e, 0, sizeof(XEvent));
            e.type = ClientMessage;
            e.xclient.send_event = true;
            e.xclient.display = xDisplay_;
            e.xclient.window = broadcastWindow_;
            e.xclient.message_type = xAppEvent_;
            e.xclient.format = 32;
            //e.xclient.data.l[0] = wmUserEventMessage_;
            // send the message that informs the renderer to process the queue
            XSendEvent(xDisplay_, broadcastWindow_, false, NoEventMask, &e);
    		XFlush(xDisplay_);
        });

		X11Window::StartBlinkerThread();
    }

	X11Application::~X11Application() {
		XCloseDisplay(xDisplay_);
		xDisplay_ = nullptr;
	}

    void X11Application::alert(std::string const & message) {
		if (system(STR("xmessage -center \"" << message << "\"").c_str()) != EXIT_SUCCESS) 
            std::cout << message << std::endl;
    }

    void X11Application::openLocalFile(std::string const & filename, bool edit) {
		if (edit) {
			if (system(STR("x-terminal-emulator -e editor \"" << filename << "\" &").c_str()) != EXIT_SUCCESS)
				alert(STR("unable to open file " << filename << " with default editor"));
		} else {
			if (system(STR("xdg-open \"" << filename << "\" &").c_str()) != EXIT_SUCCESS)
				alert(STR("xdg-open not found or unable to open file:\n" << filename));
		}
    }

    Window * X11Application::createWindow(std::string const & title, int cols, int rows) {
		return new X11Window{title, cols, rows};
    }

    void X11Application::mainLoop() {
        XEvent e;
        try {
            while (true) { 
                XNextEvent(xDisplay_, &e);
                if (XFilterEvent(&e, x11::None))
                    continue;
                if (e.type == ClientMessage && e.xclient.window == broadcastWindow_) {
                    if (static_cast<unsigned long>(e.xclient.message_type) == X11Application::Instance()->xAppEvent_) 
                        Renderer::ExecuteUserEvent();
                } else {
                    X11Window::EventHandler(e);
                }
            }
        } catch (TerminateException const &) {
            // don't do anything
        }
    }

    void X11Application::xSendEvent(X11Window * window, XEvent & e, long mask) {
		if (window != nullptr)
            XSendEvent(xDisplay_, window->window_, false, mask, &e);
		else 
			XSendEvent(xDisplay_, broadcastWindow_, false, mask, &e);
		XFlush(xDisplay_);
    }

    void X11Application::openInputMethod() {
		// set the default machine locale instead of the "C" locale
		setlocale(LC_CTYPE, "");
		XSetLocaleModifiers("");
		// create X Input Method, each window then has its own context
		xIm_ = XOpenIM(xDisplay_, nullptr, nullptr, nullptr);
		if (xIm_ != nullptr)
			return;
		XSetLocaleModifiers("@im=local");
		xIm_ = XOpenIM(xDisplay_, nullptr, nullptr, nullptr);
		if (xIm_ != nullptr)
			return;
		XSetLocaleModifiers("@im=");
		xIm_ = XOpenIM(xDisplay_, nullptr, nullptr, nullptr);
		//if (xIm_ != nullptr)
		//	return;
		//OSCHECK(false) << "Cannot open input device (XOpenIM failed)";
    }

}

#endif