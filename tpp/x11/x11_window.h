#pragma
#if (defined ARCH_UNIX)

#include "x11_font.h"

namespace tpp {


    class X11Window : public RendererWindow<X11Window> {
    public:
        typedef Font<XftFont*> Font;

        ~X11Window() override;
        
        void show() override;

        void hide() override {
            NOT_IMPLEMENTED;
        }

        void close() override {
            XDestroyWindow(display_, window_);            
        }

        /** Schedules the window to be repainted.

            Instead of invalidating the rectange, WM_PAINT must explicitly be sent, as it may happen that different thread is already repainting the window, and therefore the request will be silenced (the window region is validated at the end of WM_PAINT). 
         */
		void paint(ui::RectEvent & e) override {
            // trigger a refresh
            XEvent e;
            memset(&e, 0, sizeof(XEvent));
            e.xexpose.type = Expose;
            e.xexpose.display = display_;
            e.xexpose.window = window_;
            X11Application::Instance()->xSendEvent(this, e, ExposureMask);
		}

    protected:

        friend class RendererWindow<X11Window>;

        X11Window(std::string const & title, int cols, int rows, unsigned baseCellHeightPx);

        void updateSizePx(unsigned widthPx, unsigned heightPx) override {
            //widthPx -= frameWidthPx_;
            //heightPx -= frameHeightPx_;
			if (rt_ != nullptr) {
				D2D1_SIZE_U size = D2D1::SizeU(widthPx, heightPx);
				rt_->Resize(size);
			}
            Window::updateSizePx(widthPx, heightPx);
			repaint();
        }

        void updateSize(int cols, int rows) override {
			if (rt_ != nullptr) 
                updateXftStructures(cols);
            Window::updateSize(cols, rows);
			repaint();
        }


        void updateFullscreen(bool value) override;

        void updateZoom(double value) override;

        void updateXftStructures(int cols) {
            delete text_;
            text_ = new XftCharSpec[cols];
        }

        // rendering methods - we really want these to inline

        void initializeDraw() {
        }

        void finalizeDraw() {
        }

        bool shouldDraw(int col, int row, ui::Cell const & cell) {
            return true;
        }

        void initializeGlyphRun(int col, int row) {
            textSize_ = 0;
            textCol_ = col;
            textRow_ = row;
        }

        void addGlyph(ui::Cell const & cell) {
            if (textSize_ == 0) {
                text_[0].x = textCol_ * cellWidthPx_;
                text_[0].y = textRow_ * cellHeightPx_ + font_->ascent();
            } else {
                text_[textSize_].x = text_[textSize_ - 1].x + cellWidthPx_;
                text_[textSize_].y = text_[textSize_ - 1].y;
            }
            text_[textSize_].ucs4 = cell.codepoint();
            ++textSize_;
        }

        /** Updates the current font.
         */
        void setFont(ui::Font font) {
			font_ = Font::GetOrCreate(font, cellHeightPx_);
        }

        /** Updates the foreground color.
         */
        void setForegroundColor(ui::Color color) {
            fg_ = toXftColor(color);
        }

        /** Updates the background color. 
         */
        void setBackgroundColor(ui::Color color) {
            bg_ = toXftColor(color);
        }

        /** Updates the decoration color. 
         */
        void setDecorationColor(ui::Color color) {
            decor_ = toXftColor(color);
        }

        /** Sets the attributes of the cell. 
         */
        void setAttributes(ui::Attributes const & attrs) {
            attrs_ = attrs;
        }

        /** Draws the glyph run. 
         
            First clears the background with given background color, then draws the text and finally applies any decorations. 
         */
        void drawGlyphRun() {
            if (textSize_ == 0)
                return;
			// if we are drawing the last col, or row, clear remaining border as well
			if (textCol_ + textSize_ == cols() || textRow_ == rows() - 1) {
				unsigned clearW = (textCol_ + textSize_ == cols()) ? (widthPx_ - (textCol_ * cellWidthPx_)) : (textSize_ * cellWidthPx_);
				unsigned clearH = (textRow_ == rows() - 1) ? (heightPx_ - (textRow_ * cellHeightPx_)) : (cellHeightPx_);
				XftColor clearC = toXftColor(terminal()->defaultBackgroundColor());
				XftDrawRect(draw_, &clearC, textCol_ * cellWidthPx_, textRow_ * cellHeightPx_, clearW, clearH);
			}
			XftDrawRect(draw_, &bg_, textCol_ * cellWidthPx_, textRow_ * cellHeightPx_, textSize_ * cellWidthPx_, cellHeightPx_);
			// if the cell is blinking, only draw the text if blink is on
			if (!textBlink_ || blink_) {
                XftDrawCharSpec(draw_, &fg_, font_->handle(), text_, textSize_);
				// renders underline and strikethrough lines
				// TODO for now, this is just approximate values of just below and 2/3 of the font, which is blatantly copied from st and is most likely not typographically correct (see issue 12)
				if (textUnderline_)
					XftDrawRect(draw_, &fg_,textCol_ * cellWidthPx_, textRow_ * cellHeightPx_ + font_->handle()->ascent + 1, cellWidthPx_ * textSize_, 1);
				if (textStrikethrough_)
					XftDrawRect(draw_, &fg_, textCol_ * cellWidthPx_, textRow_ * cellHeightPx_ + (2 * font_->handle()->ascent / 3), cellWidthPx_ * textSize_, 1);
			}
            textSize_ = 0;
        }

		XftColor toXftColor(vterm::Color const& c) {
			XftColor result;
			result.color.red = c.red * 256;
			result.color.green = c.green * 256;
			result.color.blue = c.blue * 256;
			result.color.alpha = c.alpha * 256;
			return result;
		}

		/** Sets the window icon. 

		    The window icon must be an array of BGRA colors for the different icon sizes where the first element is the total size of the array followed by arbitrary icon sizes encoded by first 2 items representing the icon width and height followed by the actual pixels. 
		 */
		void setIcon(unsigned long * icon);

    private:

		class MotifHints {
		public:
			unsigned long   flags;
			unsigned long   functions;
			unsigned long   decorations;
			long            inputMode;
			unsigned long   status;
		};

		/** Given current state as reported from X11, translates it to vterm::Key modifiers
		 */
		static unsigned GetStateModifiers(int state);

        /** Converts the KeySym and preexisting modifiers as reported by X11 into key. 

		    Because the modifiers are preexisting, but the terminal requires post-state, Shift, Ctrl, Alt and Win keys also update the modifiers based on whether they key was pressed, or released
         */
        static vterm::Key GetKey(KeySym k, unsigned modifiers, bool pressed);

        static void EventHandler(XEvent & e);

		Window window_;
		Display* display_;
		int screen_;
		Visual* visual_;
		Colormap colorMap_;
        XIC ic_;

        GC gc_;
        Pixmap buffer_;

		XftDraw * draw_;
		XftColor fg_;
		XftColor bg_;
		Font * font_;


        XftCharSpec * text_;

        /** Text buffer rendering data.
         */
        unsigned textCol_;
        unsigned textRow_;
        unsigned textSize_;
        bool textBlink_;
        bool textUnderline_;
        bool textStrikethrough_;


        std::mutex drawGuard_;
        std::atomic<bool> invalidate_;

		/** Info about the window state before fullscreen was triggered. 
		 */
        XWindowChanges fullscreenRestore_;

		static std::unordered_map<Window, X11Window *> Windows_;

    }; // X11Window

} // namespace tpp

#endif