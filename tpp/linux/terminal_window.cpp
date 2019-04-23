#ifdef __linux__

#include "helpers/helpers.h"
#include "helpers/log.h"

#include "application.h"

#include "terminal_window.h"

namespace tpp {

    std::unordered_map<Window, TerminalWindow *> TerminalWindow::Windows_;


	// http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/xintro.html
	// https://keithp.com/~keithp/talks/xtc2001/paper/
    // https://en.wikibooks.org/wiki/Guide_to_X11/Fonts
	// https://keithp.com/~keithp/render/Xft.tutorial


	TerminalWindow::TerminalWindow(Application* app, TerminalSettings* settings) :
		BaseTerminalWindow(settings),
		display_(Application::XDisplay()),
		screen_(Application::XScreen()),
	    visual_(DefaultVisual(display_, screen_)),
	    colorMap_(DefaultColormap(display_, screen_)),
	    buffer_(0),
	    draw_(nullptr) {
		unsigned long black = BlackPixel(display_, screen_);	/* get color black */
		unsigned long white = WhitePixel(display_, screen_);  /* get color white */
        Window parent = RootWindow(display_, screen_);

		LOG << widthPx_ << " x " << heightPx_;

		window_ = XCreateSimpleWindow(display_, parent, 0, 0, widthPx_, heightPx_, 1, white, black);

		// from http://math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/xintro.html

		/* here is where some properties of the window can be set.
			   The third and fourth items indicate the name which appears
			   at the top of the window and the name of the minimized window
			   respectively.
			*/
		XSetStandardProperties(display_, window_, title_.c_str(), nullptr, None, nullptr, 0, nullptr);

		/* this routine determines which types of input are allowed in
		   the input.  see the appropriate section for details...
		*/
		XSelectInput(display_, window_, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | VisibilityChangeMask | ExposureMask | FocusChangeMask);

        XGCValues gcv;
        memset(&gcv, 0, sizeof(XGCValues));
    	gcv.graphics_exposures = False;
        gc_ = XCreateGC(display_, parent, GCGraphicsExposures, &gcv);


        Windows_[window_] = this;
	}

	void TerminalWindow::show() {
        XMapWindow(display_, window_);
		//XMapRaised(display_, window_);
	}

	TerminalWindow::~TerminalWindow() {
        Windows_.erase(window_);
		doInvalidate();
		XFreeGC(display_, gc_);
		XDestroyWindow(display_, window_);
	}

	void TerminalWindow::repaint(vterm::Terminal::RepaintEvent& e) {
		doInvalidate();
		// TODO actually invalidate the window, is this really doUpdateBuffer
		doPaint();
	}

	void TerminalWindow::doSetFullscreen(bool value) {

	}

	void TerminalWindow::doTitleChange(vterm::VT100::TitleEvent& e) {

	}

	void TerminalWindow::doPaint() {
		LOG << "doPaint";
		ASSERT(draw_ == nullptr);
		bool forceDirty = false;
		if (buffer_ == 0) {
			buffer_ = XCreatePixmap(display_, window_, widthPx_, heightPx_, DefaultDepth(display_, screen_));
			ASSERT(buffer_ != 0);
			forceDirty = true;
		}
		draw_ = XftDrawCreate(display_, buffer_, visual_, colorMap_);
		doUpdateBuffer(forceDirty);
		XCopyArea(display_, buffer_, window_, gc_, 0, 0, widthPx_, heightPx_, 0, 0);
		XftDrawDestroy(draw_);
		draw_ = nullptr;
		LOG << "doPaint done";
	}


#ifdef HAHA
	void TerminalWindow::updateBuffer() {
        LOG << " update start";
        // create the buffer
        buffer_ = XCreatePixmap(display_, window_, widthPx_, heightPx_, DefaultDepth(display_, screen_));


		bool forceUpdate_ = true;
		if (terminal() == nullptr)
			return;
		vterm::Terminal::Layer l = terminal()->getDefaultLayer();
		/* Determine if cursor is to be shown by checking if the cursor is in the visible range. If it is, then make sure the cursor's position is set to dirty.
		 */
		helpers::Point cp = terminal()->cursorPos();
		bool cursorInRange = cp.col < cols() && cp.row < rows();
		if (cursorInRange)
			l->at(cp).dirty = true;
//		XftDraw* draw = XftDrawCreate(display_, window_, visual_, colorMap_);
		XftDraw* draw = XftDrawCreate(display_, buffer_, visual_, colorMap_);
		// initialize the font & colors
		vterm::Cell & c = l->at(0, 0);
		vterm::Color currentFg = c.fg;
		vterm::Color currentBg = c.bg;
        XftColor fg = toXftColor(currentFg);
		XftColor bg = toXftColor(currentBg);
		Font* font = Font::GetOrCreate(DropBlink(c.font), settings_->defaultFontHeight, zoom_);
		ASSERT(draw != nullptr);
		for (unsigned row = 0; row < rows(); ++row) {
			for (unsigned col = 0; col < cols(); ++col) {
				vterm::Cell& c = l->at(col, row);
				if (forceUpdate_ || c.dirty) {
					c.dirty = false; // clear the dirty flag
					if (currentFg != c.fg) {
						currentFg = c.fg;
					    fg = toXftColor(currentFg);
					}
					if (currentBg != c.bg) {
						currentBg = c.bg;
						bg = toXftColor(currentBg);
					}
					if (font->font() != DropBlink(c.font)) {
						font = Font::GetOrCreate(DropBlink(c.font), settings_->defaultFontHeight, zoom_);
					}
/*					if (col % 2 == 0)
						XftDrawString8(draw, &fg, font->handle(), col * cellWidthPx_, (row + 1) * cellHeightPx_ - font->handle()->descent, (XftChar8*)"A", 1);
					else 
						XftDrawString8(draw, &fg, font->handle(), col * cellWidthPx_, (row + 1) * cellHeightPx_ - font->handle()->descent, (XftChar8*)"p", 1); */
					XftDrawRect(draw, &bg, col * cellWidthPx_, row * cellHeightPx_, cellWidthPx_, cellHeightPx_);
					XftDrawStringUtf8(draw, &fg, font->handle(), col * cellWidthPx_, (row + 1) * cellHeightPx_ - font->handle()->descent, (XftChar8*)(c.c.rawBytes()), c.c.size());

					//TextOutW(bufferDC_, col * cellWidthPx_, row * cellHeightPx_, &wc, 1);
				}
			}
		}
		/* Determine whether the cursor character itself should be drawn and draw it if so.
		 * /
		if (cursorInRange && terminal()->cursorVisible() && (blink_ || !terminal()->cursorBlink())) {
			wchar_t wc = terminal()->cursorCharacter().toWChar();
			SetTextColor(bufferDC_, RGB(255, 255, 255));
			SetBkMode(bufferDC_, TRANSPARENT);
			// select default font
			// TODO - how should this change when we have multiple font sizes
			SelectObject(bufferDC_, Font::GetOrCreate(vterm::Font(), settings_->defaultFontHeight, zoom_)->handle());
			TextOutW(bufferDC_, cp.col * cellWidthPx_, cp.row * cellHeightPx_, &wc, 1);
		}
			*/
			//forceUpdate_ = false;
        XCopyArea(display_, buffer_, window_, gc_, 0, 0, widthPx_, heightPx_, 0, 0);
        XftDrawDestroy(draw);
        XFreePixmap(display_, buffer_);
		LOG << "redraw done";
	}
#endif

    void TerminalWindow::EventHandler(XEvent & e) {
        TerminalWindow * tw = nullptr;
        auto i = Windows_.find(e.xany.window);
        if (i != Windows_.end())
            tw = i->second;
        switch(e.type) {
            case Expose:
                LOG << "Exposed";
                tw->doPaint();
                break;
            /** Handles window resize, which should change the terminal size accordingly. 
             */  
            case ConfigureNotify: {
                if (tw->widthPx_ != static_cast<unsigned>(e.xconfigure.width) || tw->heightPx_ != static_cast<unsigned>(e.xconfigure.height))
                    tw->resizeWindow(e.xconfigure.width, e.xconfigure.height);
                break;
            }
            case MapNotify:
                break;
            case KeyPress:
				tw->redraw();
                return;
            case ButtonPress:
                break;
            default:
            break;
        }
    }

} // namespace tpp

#endif