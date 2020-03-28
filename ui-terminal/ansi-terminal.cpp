#include "ansi-terminal.h"

namespace ui2 {

    namespace {

        std::unordered_map<Key, std::string> InitializeVT100KeyMap() {
            #define KEY(K, ...) ASSERT(km.find(K) == km.end()) << "Key " << K << " altrady defined"; km.insert(std::make_pair(K, STR(__VA_ARGS__)))
            #define VT_MODIFIERS(K, SEQ1, SEQ2) KEY(Key(K) + Key::Shift, SEQ1 << 2 << SEQ2); \
                                                KEY(Key(K) + Key::Alt, SEQ1 << 3 << SEQ2); \
                                                KEY(Key(K) + Key::Shift + Key::Alt, SEQ1 << 4 << SEQ2); \
                                                KEY(Key(K) + Key::Ctrl, SEQ1 << 5 << SEQ2); \
                                                KEY(Key(K) + Key::Ctrl + Key::Shift, SEQ1 << 6 << SEQ2); \
                                                KEY(Key(K) + Key::Ctrl + Key::Alt, SEQ1 << 7 << SEQ2); \
                                                KEY(Key(K) + Key::Ctrl + Key::Alt + Key::Shift, SEQ1 << 8 << SEQ2);

            using namespace ui2;

            std::unordered_map<Key, std::string> km;
            // first add letter keys in their modifications
            for (unsigned k = 'A'; k <= 'Z'; ++k) {
                // ctrl + letter and ctrl + shift + letter are the same
                KEY(Key(k) + Key::Ctrl, static_cast<char>(k + 1 - 'A'));
                KEY(Key(k) + Key::Ctrl + Key::Shift, static_cast<char>(k + 1 - 'A'));
                // alt simply prepends escape to whatever the non-alt key would be
                KEY(Key(k) + Key::Alt, '\033' << static_cast<char>(k + 32));
                KEY(Key(k) + Key::Shift + Key::Alt, '\033' << static_cast<char>(k));
                KEY(Key(k) + Key::Ctrl + Key::Alt, '\033' << static_cast<char>(k + 1 - 'A'));
                KEY(Key(k) + Key::Ctrl + Key::Shift + Key::Alt, '\033' << static_cast<char>(k + 1 - 'A'));
            }

            // modifiers + numbers
            for (unsigned k = '0'; k <= '9'; ++k) {
                // alt + key prepends escape character
                KEY(Key(k) + Key::Alt, '\033' << static_cast<char>(k));
            }

            // ctrl + 2 is 0
            KEY(Key(Key::Num0) + Key::Ctrl, "\000");
            // alt + shift keys and other extre keys
            KEY(Key(Key::Num0) + Key::Shift + Key::Alt, "\033)");
            KEY(Key(Key::Num1) + Key::Shift + Key::Alt, "\033!");
            KEY(Key(Key::Num2) + Key::Shift + Key::Alt, "\033@");
            KEY(Key(Key::Num3) + Key::Shift + Key::Alt, "\033#");
            KEY(Key(Key::Num4) + Key::Shift + Key::Alt, "\033$");
            KEY(Key(Key::Num5) + Key::Shift + Key::Alt, "\033%");
            KEY(Key(Key::Num6) + Key::Shift + Key::Alt, "\033^");
            KEY(Key(Key::Num7) + Key::Shift + Key::Alt, "\033&");
            KEY(Key(Key::Num8) + Key::Shift + Key::Alt, "\033*");
            KEY(Key(Key::Num9) + Key::Shift + Key::Alt, "\033(");
            // other special characters with alt
            KEY(Key(Key::Tick) + Key::Alt, "\033`");
            KEY(Key(Key::Tick) + Key::Shift + Key::Alt, "\033~");
            KEY(Key(Key::Minus) + Key::Alt, "\033-");
            KEY(Key(Key::Minus) + Key::Alt + Key::Shift, "\033_");
            KEY(Key(Key::Equals) + Key::Alt, "\033=");
            KEY(Key(Key::Equals) + Key::Alt + Key::Shift, "\033+");
            KEY(Key(Key::SquareOpen) + Key::Alt, "\033[");
            KEY(Key(Key::SquareOpen) + Key::Alt + Key::Shift, "\033{");
            KEY(Key(Key::SquareClose) + Key::Alt, "\033]");
            KEY(Key(Key::SquareClose) + Key::Alt + Key::Shift, "\033}");
            KEY(Key(Key::Backslash) + Key::Alt, "\033\\");
            KEY(Key(Key::Backslash) + Key::Alt + Key::Shift, "\033|");
            KEY(Key(Key::Semicolon) + Key::Alt, "\033;");
            KEY(Key(Key::Semicolon) + Key::Alt + Key::Shift, "\033:");
            KEY(Key(Key::Quote) + Key::Alt, "\033'");
            KEY(Key(Key::Quote) + Key::Alt + Key::Shift, "\033\"");
            KEY(Key(Key::Comma) + Key::Alt, "\033,");
            KEY(Key(Key::Comma) + Key::Alt + Key::Shift, "\033<");
            KEY(Key(Key::Dot) + Key::Alt, "\033.");
            KEY(Key(Key::Dot) + Key::Alt + Key::Shift, "\033>");
            KEY(Key(Key::Slash) + Key::Alt, "\033/");
            KEY(Key(Key::Slash) + Key::Alt + Key::Shift, "\033?");
            // arrows, fn keys & friends
            KEY(Key::Up, "\033[A");
            KEY(Key::Down, "\033[B");
            KEY(Key::Right, "\033[C");
            KEY(Key::Left, "\033[D");
            KEY(Key::Home, "\033[H"); // also \033[1~
            KEY(Key::End, "\033[F"); // also \033[4~
            KEY(Key::PageUp, "\033[5~");
            KEY(Key::PageDown, "\033[6~");
            KEY(Key::Insert, "\033[2~");
            KEY(Key::Delete, "\033[3~");
            KEY(Key::F1, "\033OP");
            KEY(Key::F2, "\033OQ");
            KEY(Key::F3, "\033OR");
            KEY(Key::F4, "\033OS");
            KEY(Key::F5, "\033[15~");
            KEY(Key::F6, "\033[17~");
            KEY(Key::F7, "\033[18~");
            KEY(Key::F8, "\033[19~");
            KEY(Key::F9, "\033[20~");
            KEY(Key::F10, "\033[21~");
            KEY(Key::F11, "\033[23~");
            KEY(Key::F12, "\033[24~");

            KEY(Key::Enter, "\r"); // carriage return, not LF
            KEY(Key::Tab, "\t");
            KEY(Key::Esc, "\033");
            KEY(Key::Backspace, "\x7f");

            VT_MODIFIERS(Key::Up, "\033[1;", "A");
            VT_MODIFIERS(Key::Down, "\033[1;", "B");
            VT_MODIFIERS(Key::Left, "\033[1;", "D");
            VT_MODIFIERS(Key::Right, "\033[1;", "C");
            VT_MODIFIERS(Key::Home, "\033[1;", "H");
            VT_MODIFIERS(Key::End, "\033[1;", "F");
            VT_MODIFIERS(Key::PageUp, "\033[5;", "~");
            VT_MODIFIERS(Key::PageDown, "\033[6;", "~");

            VT_MODIFIERS(Key::F1, "\033[1;", "P");
            VT_MODIFIERS(Key::F2, "\033[1;", "Q");
            VT_MODIFIERS(Key::F3, "\033[1;", "R");
            VT_MODIFIERS(Key::F4, "\033[1;", "S");
            VT_MODIFIERS(Key::F5, "\033[15;", "~");
            VT_MODIFIERS(Key::F6, "\033[17;", "~");
            VT_MODIFIERS(Key::F7, "\033[18;", "~");
            VT_MODIFIERS(Key::F8, "\033[19;", "~");
            VT_MODIFIERS(Key::F9, "\033[20;", "~");
            VT_MODIFIERS(Key::F10, "\033[21;", "~");
            VT_MODIFIERS(Key::F11, "\033[23;", "~");
            VT_MODIFIERS(Key::F12, "\033[24;", "~");

            KEY(Key(Key::SquareOpen) + Key::Ctrl, "\033");
            KEY(Key(Key::Backslash) + Key::Ctrl, "\034");
            KEY(Key(Key::SquareClose) + Key::Ctrl, "\035");

            return km;
            #undef KEY
            #undef VT_MODIFIERS
        }

    } // anonymous namespace

    // ============================================================================================

    // TODO remove the underscores
    helpers::Log AnsiTerminal::SEQ("VT100_");
    helpers::Log AnsiTerminal::SEQ_UNKNOWN("VT100_UNKNOWN_");
    helpers::Log AnsiTerminal::SEQ_ERROR("VT100_ERROR_");
    helpers::Log AnsiTerminal::SEQ_WONT_SUPPORT("VT100_WONT_SUPPORT_");

    std::unordered_map<Key, std::string> AnsiTerminal::KeyMap_(InitializeVT100KeyMap());

    char32_t AnsiTerminal::LineDrawingChars_[15] = {0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0, 0, 0x2500, 0, 0, 0x251c, 0x2524, 0x2534, 0x252c, 0x2502};

    AnsiTerminal::AnsiTerminal(Palette * palette, int width, int height, int x, int y):
        Widget{width, height, x, y}, 
        Scrollable{width, height},
        fps_{0},
        repaint_{false},
        palette_{palette},
        mouseButtonsDown_{0},
        mouseLastButton_{0},
        state_{width, height},
        alternateMode_{false} {
        setFps(60);
    }

    AnsiTerminal::~AnsiTerminal() {
        setFps(0);
        // wait for the repainter thread to terminate
        if (repainter_.joinable())
            repainter_.join();
    }

    /** The method initializes a repainter thread which sleeps for the appropriate time and then checks whether the repaint has been requested in the meantime, clearing the flag and issuing the actual repaint if so. 
     */
    void AnsiTerminal::setFps(unsigned value) {
        UI_THREAD_CHECK;
        if (fps_ != value) {
            // if the previously set fps is 0, that change would make active repainter terminate itself, so we must wait & make sure this is the case as the repainter will be restarted
            if (fps_ == 0 && repainter_.joinable())
                repainter_.join();
            size_t oldFps = fps_;
            fps_ = value;
            // if previous fps value was 0, the repainter thread must be restarted
            if (oldFps == 0) {
                repainter_ = std::thread([this](){
                    while (fps_ > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps_));
                        if (repaint_) {
                            repaint_ = false;
                            // trigger the repaint by calling widget's repaint implementation
                            Widget::repaint();
                        }
                    }
                });
            }
        }
    }

    void AnsiTerminal::paint(Canvas & canvas) {
        Canvas c{getContentsCanvas(canvas)};
        // lock the buffer
        bufferLock_.priorityLock();
        helpers::SmartRAIIPtr<helpers::PriorityLock> g{bufferLock_, false};
        // draw the history 
        {
            int firstVisible = 0;
            int lastVisible = state_.historyRows();
            c.setBg(palette_->defaultBackground());
            c.fillRect(Rect::FromTopLeftWH(0, firstVisible, c.width(), lastVisible - firstVisible));
            state_.drawHistoryRows(c, firstVisible, lastVisible);
        }
        // draw the buffer
        c.drawBuffer(state_.buffer, Point{0,state_.historyRows()});
        // draw the scrollbars if any
        Scrollable::paint(canvas);
    }

    void AnsiTerminal::setRect(Rect const & value) {
        // if resized, resize the terminal buffers and the pty
        if (value.width() != width() || value.height() != height()) {
            // lock the buffer
            bufferLock_.priorityLock();
            helpers::SmartRAIIPtr<helpers::PriorityLock> g{bufferLock_, false};
            // resize the state
            state_.resize(value.width(), value.height());
            // TODO resize the alternate buffer as well
            // resize the PTY
            ptyResize(value.width(), value.height());
            // update the scrolling information            
            Scrollable::setRect(value);
            setScrollWidth(value.width());
            setScrollHeight(value.height() + state_.historyRows());

        }
        Widget::setRect(value);
    }

    void AnsiTerminal::mouseMove(Event<MouseMoveEvent>::Payload & event) {
        if (state_.mouseMode != MouseMode::Off 
            && (state_.mouseMode != MouseMode::ButtonEvent || mouseButtonsDown_ != 0)
            // only send the mouse information if the mouse is in the range of the window
            && Rect::FromWH(width(), height()).contains(event->coords)) {
                // mouse move adds 32 to the last known button press
                sendMouseEvent(mouseLastButton_ + 32, event->coords, 'M');
                LOG(SEQ) << "Mouse moved to " << event->coords;
        }
        /*
        if (modifiers == 0) {
            if (updatingSelection()) {
                updateSelection(Point{col, row} + scrollOffset(), clientSize());
                if (scrollable_) {
                    if (row < 0 || row >= height()) 
                        startAutoScroll(Point{0, row < 0 ? -1 : 1});
                    else 
                        stopAutoScroll();
                }
            }
            bool x = col == width() - 1;
            if (x != scrollBarActive_) {
                scrollBarActive_ = x;
                repaint();
            }
        }
        */
        Widget::mouseMove(event);
    }

    void AnsiTerminal::mouseDown(Event<MouseButtonEvent>::Payload & event) {
        ++mouseButtonsDown_;
		if (state_.mouseMode != MouseMode::Off) {
            mouseLastButton_ = encodeMouseButton(event->button, event->modifiers);
            sendMouseEvent(mouseLastButton_, event->coords, 'M');
            LOG(SEQ) << "Button " << event->button << " down at " << event->coords;
        }
        /*
        if (modifiers == 0) {
            if (button == MouseButton::Left) {
                startSelection(Point{col, row} + scrollOffset());
            } else if (button == MouseButton::Wheel) {
                requestSelectionContents(); 
            } else if (button == MouseButton::Right && ! selection().empty()) {
                setClipboardContents(getSelectionContents());
                clearSelection();
            }
        }
        */
        Widget::mouseDown(event);
    }

    void AnsiTerminal::mouseUp(Event<MouseButtonEvent>::Payload & event) {
        // a bit of defensive programming
        if (mouseLastButton_ > 0) {
            --mouseButtonsDown_;
            if (state_.mouseMode != MouseMode::Off) {
                mouseLastButton_ = encodeMouseButton(event->button, event->modifiers);
                sendMouseEvent(mouseLastButton_, event->coords, 'm');
                LOG(SEQ) << "Button " << event->button << " up at " << event->coords;
            }
        }
        /*
        if (modifiers == 0) {
            if (button == MouseButton::Left) 
                endSelection();
        }
        */
        Widget::mouseUp(event);
    }

    void AnsiTerminal::mouseWheel(Event<MouseWheelEvent>::Payload & event) {
		if (state_.mouseMode != MouseMode::Off) {
    		// mouse wheel adds 64 to the value
            mouseLastButton_ = encodeMouseButton((event->by > 0) ? MouseButton::Left : MouseButton::Right, event->modifiers) + 64;
            sendMouseEvent(mouseLastButton_, event->coords, 'M');
            LOG(SEQ) << "Wheel offset " << event->by << " at " << event->coords;
        }
        if (state_.historyRows() > 0) {
            if (event->by > 0)
                scrollBy(Point{0, -1});
            else 
                scrollBy(Point{0, 1});
        }
        Widget::mouseWheel(event);
    }

    unsigned AnsiTerminal::encodeMouseButton(MouseButton btn, Key modifiers) {
		unsigned result =
			((modifiers & Key::Shift) ? 4 : 0) +
			((modifiers & Key::Alt) ? 8 : 0) +
			((modifiers & Key::Ctrl) ? 16 : 0);
		switch (btn) {
			case MouseButton::Left:
				return result;
			case MouseButton::Right:
				return result + 1;
			case MouseButton::Wheel:
				return result + 2;
			default:
				UNREACHABLE;
		}
    }

    void AnsiTerminal::sendMouseEvent(unsigned button, Point coords, char end) {
		// first increment col & row since terminal starts from 1
        coords += Point{1,1};
		switch (state_.mouseEncoding) {
			case MouseEncoding::Default: {
				// if the event is release, button number is 3
				if (end == 'm')
					button |= 3;
				// increment all values so that we start at 32
				button += 32;
                coords += Point{32, 32};
				// if the col & row are too high, ignore the event
				if (coords.x() > 255 || coords.y() > 255)
					return;
				// otherwise output the sequence
				char buffer[6];
				buffer[0] = '\033';
				buffer[1] = '[';
				buffer[2] = 'M';
				buffer[3] = button & 0xff;
				buffer[4] = static_cast<char>(coords.x());
				buffer[5] = static_cast<char>(coords.y());
				ptySend(buffer, 6);
				break;
			}
			case MouseEncoding::UTF8: {
				LOG(SEQ_WONT_SUPPORT) << "utf8 mouse encoding";
				break;
			}
			case MouseEncoding::SGR: {
				std::string buffer = STR("\033[<" << button << ';' << coords.x() << ';' << coords.y() << end);
				ptySend(buffer.c_str(), buffer.size());
				break;
			}
		}
    }

    void AnsiTerminal::keyChar(Event<Char>::Payload & event) {
        ASSERT(event->codepoint() >= 32);
        ptySend(event->toCharPtr(), event->size());
        Widget::keyChar(event);
    }

    void AnsiTerminal::keyDown(Event<Key>::Payload & event) {
		std::string const* seq = GetSequenceForKey_(*event);
		if (seq != nullptr) {
			switch (event->code()) {
			case Key::Up:
			case Key::Down:
			case Key::Left:
			case Key::Right:
			case Key::Home:
			case Key::End:
				if (event->modifiers() == 0 && state_.cursorMode == CursorMode::Application) {
					std::string sa(*seq);
					sa[1] = 'O';
					ptySend(sa.c_str(), sa.size());
					return;
				}
				break;
			default:
				break;
			}
			ptySend(seq->c_str(), seq->size());
		}
        // only scroll to prompt if the key down is not a simple modifier key
        if (*event != Key::Shift + Key::ShiftKey && *event != Key::Alt + Key::AltKey && *event != Key::Ctrl + Key::CtrlKey && *event != Key::Win + Key::WinKey)
            setScrollOffset(Point{0, state_.historyRows()});
        Widget::keyDown(event);
    }

    size_t AnsiTerminal::processInput(char const * buffer, char const * bufferEnd) {
        // lock the buffer first
        helpers::SmartRAIIPtr<helpers::PriorityLock> g{bufferLock_};
        // then process the input
        char const * x = buffer;
        while (x != bufferEnd) {
            switch (*x) {
                /* Parse the escape sequence */
                case Char::ESC: {
                    size_t processed = parseEscapeSequence(x, bufferEnd);
                    // if no characters were processed, the sequence was incomplete and we should end processing
                    if (processed == 0)
                        return x - buffer;
                    // move past the sequence
                    x += processed;
                    break;
                }
                /* BEL triggers the notification */
                case Char::BEL:
                    parseNotification();
                    ++x;
                    break;
                case Char::TAB:
                    parseTab();
                    ++x;
                    break;
                case Char::LF:
                    parseLF();
                    ++x;
                    break;
                case Char::CR:
                    parseCR();
                    ++x;
                    break;
                case Char::BACKSPACE:
                    parseBackspace();
                    ++x;
                    break;
                default: {
                    // while this is a code duplication from the helpers::Char class, since this code is a bottleneck for processing large ammounts of text, the code is copied for performance
                    char32_t cp = 0;
                    unsigned char const * ux = pointer_cast<unsigned char const *>(x);
                    if (*ux < 0x80) {
                        cp = *ux;
                        ++x;
                    } else if (*ux < 0xe0) {
                        if (x + 2 > bufferEnd)
                            return x - buffer;
                        cp = ((ux[0] & 0x1f) << 6) + (ux[1] & 0x3f);
                        x += 2;
                    } else if (*ux < 0xf0) {
                        if (x + 3 > bufferEnd)
                            return x - buffer;
                        cp = ((ux[0] & 0x0f) << 12) + ((ux[1] & 0x3f) << 6) + (ux[2] & 0x3f);
                        x += 3;
                    } else {
                        if (x + 4 > bufferEnd)
                            return x - buffer;
                        cp = ((ux[0] & 0x07) << 18) + ((ux[1] & 0x3f) << 12) + ((ux[2] & 0x3f) << 6) + (ux[3] & 0x3f);
                        x += 4;
                    }
                    parseCodepoint(cp);
                    break;
                }
            }
        }
        repaint();
        return bufferEnd - buffer;
    }

    void AnsiTerminal::parseCodepoint(char32_t codepoint) {
        if (state_.lineDrawingSet && codepoint >= 0x6a && codepoint < 0x79)
            codepoint = LineDrawingChars_[codepoint-0x6a];
        LOG(SEQ) << "codepoint " << codepoint << " " << static_cast<char>(codepoint & 0xff);
        updateCursorPosition();
        // set the cell state
        Cell & cell = state_.buffer.at(state_.cursor);
        cell = state_.cell;
        cell.setCodepoint(codepoint);
        // advance cursor's column
        state_.cursor += Point{1, 0};

        // what's left is to deal with corner cases, such as larger fonts & double width characters
        int columnWidth = Char::ColumnWidth(codepoint);

        // if the character's column width is 2 and current font is not double width, update to double width font
        if (columnWidth == 2 && ! cell.font().doubleWidth()) {
            columnWidth = 1;
            cell.setFont(cell.font().setDoubleWidth(true));
        }

        // TODO do double width & height characters properly for the per-line 

        /*
        // if the character's column width is 2 and current font is not double width, update to double width font
        // if the font's size is greater than 1, copy the character as required (if we are at the top row of double height characters, increase the size artificially)
        int charWidth = state_.doubleHeightTopLine ? cell.font().width() * 2 : cell.font().width();

        while (columnWidth > 0 && state_.cursor.x() < state_.buffer.width()) {
            for (int i = 1; (i < charWidth) && state_.cursor.x() < state_.buffer.width(); ++i) {
                Cell& cell2 = buffer_.at(buffer_.cursor().pos);
                // copy current cell properties
                cell2 = cell;
                // make sure the cell's font is normal size and width and display a space
                cell2.setCodepoint(' ').setFont(cell.font().setSize(1).setDoubleWidth(false));
                ++state_.cursor.x();
            } 
            if (--columnWidth > 0 && state_.cursor.x() < state_.buffer.width()) {
                Cell& cell2 = buffer_.at(buffer_.cursor().pos);
                // copy current cell properties
                cell2 = cell;
                cell2.setCodepoint(' ');
                ++state_.cursor.x();
            } 
        }
        */
    }

    void AnsiTerminal::parseNotification() {
        // NOT_IMPLEMENTED;
    }

    void AnsiTerminal::parseTab() {
        updateCursorPosition();
        if (state_.cursor.x() % 8 == 0)
            state_.cursor += Point{8, 0};
        else
            state_.cursor += Point{8 - state_.cursor.x() % 8, 0};
        LOG(SEQ) << "Tab: cursor col is " << state_.cursor.x();
    }

    void AnsiTerminal::parseLF() {
        LOG(SEQ) << "LF";
        // mark the last character position as line end
        // TODO
        // disable double width and height chars
        state_.cell.setFont(state_.cell.font().setSize(1).setDoubleWidth(false));
        state_.cursor += Point{0, 1};
        // determine if region should be scrolled
        if (state_.cursor.y() == state_.scrollEnd) {
            deleteLines(1, state_.scrollStart, state_.scrollEnd, state_.cell);
            state_.cursor -= Point{0, 1};
        }
        // update the cursor position as LF takes immediate effect
        updateCursorPosition();
    }

    void AnsiTerminal::parseCR() {
        LOG(SEQ) << "CR";
        // mark the last character as line end? 
        // TODO
        state_.cursor.setX(0);
    }

    void AnsiTerminal::parseBackspace() {
        LOG(SEQ) << "BACKSPACE";
        if (state_.cursor.x() == 0) {
            if (state_.cursor.y() > 0)
                state_.cursor -= Point{0, 1};
            state_.cursor.setX(state_.buffer.width() - 1);
        } else {
            state_.cursor -= Point{1, 0};
        }
    }

    size_t AnsiTerminal::parseEscapeSequence(char const * buffer, char const * bufferEnd) {
        ASSERT(*buffer == Char::ESC);
        // if we have nothing after the escape character, it is incomplete sequence
        char const * x = buffer;
        if (++x == bufferEnd)
            return 0;
        switch (*x++) {
            /* CSI Sequence. */
            case '[': {
                CSISequence seq{CSISequence::Parse(x, bufferEnd)};
                // if the sequence is not valid, it has been reported already and we should just exit
                if (!seq.valid())
                    break;
                // if the sequence is not complete, return false and do not advance the buffer
                if (!seq.complete())
                    return false;
                // otherwise parse the CSI sequence
                parseCSISequence(seq);
                break;
            }
            /* OSC (Operating System Command) */
            case ']': {
                OSCSequence seq{OSCSequence::Parse(x, bufferEnd)};
                // if the sequence is not valid, it has been reported already and we should just exit
                if (!seq.valid())
                    break;
                // if the sequence is not complete, return false and do not advance the buffer
                if (!seq.complete())
                    return false;
                parseOSCSequence(seq);
				break;
            }
			/* Save Cursor. */
			case '7':
				LOG(SEQ) << "DECSC: Cursor position saved";
                state_.saveCursor();
				break;
			/* Restore Cursor. */
			case '8':
                LOG(SEQ) << "DECRC: Cursor position restored";
                state_.restoreCursor();
				break;
			/* Reverse line feed - move up 1 row, same column.
			 */
			case 'M':
				LOG(SEQ) << "RI: move cursor 1 line up";
				if (state_.cursor.y() == state_.scrollStart) 
					insertLines(1, state_.scrollStart, state_.scrollEnd, state_.cell);
				else
                    state_.setCursor(state_.cursor.x(), state_.cursor.y() - 1);
				break;
            /* Device Control String (DCS). 
             */
            /*case 'P':
                if (x == bufferEnd)
                    return false;
                if (*x == '+') {
                    ++x;
                    tpp::Sequence seq(tpp::Sequence::Parse(x, bufferEnd));
                    if (!seq.complete())
                        return false;
                    if (!seq.valid())
                        break;
                    parseTppSequence(std::move(seq));
                }
                break;
            */
    		/* Character set specification - most cases are ignored, with the exception of the box drawing and reset to english (0 and B) respectively. 
             */
			case '(':
                if (x != bufferEnd) {
                    if (*x == '0') {
                        ++x;
                        state_.lineDrawingSet = true;
                        LOG(SEQ) << "Line drawing set selected";
                        break;
                    } else if (*x == 'B') {
                        ++x;
                        state_.lineDrawingSet = false;
                        LOG(SEQ) << "Normal character set selected";
                        break;
                    }
                }
                // fallthrough
			case ')':
			case '*':
			case '+':
				// missing character set specification
				if (x == bufferEnd)
					return false;
				if (*x == 'B') { // US
					++x;
					break;
				}
				LOG(SEQ_WONT_SUPPORT) << "Unknown (possibly mismatched) character set final char " << *x;
				++x;
				break;
			/* ESC = -- Application keypad */
			case '=':
				LOG(SEQ) << "Application keypad mode enabled";
                state_.keypadMode = KeypadMode::Application;
				break;
			/* ESC > -- Normal keypad */
			case '>':
				LOG(SEQ) << "Normal keypad mode enabled";
                state_.keypadMode = KeypadMode::Normal;
				break;
            /* ESC # number -- font size changes */
            /*
            case '#':
                if (x == bufferEnd)
                    return false;
                parseFontSizeSpecifier(*x);
                ++x;
                break;
                */
            default:
				LOG(SEQ_UNKNOWN) << "Unknown escape sequence \x1b" << *(x-1);
				break;
        }
        return x - buffer;
    }

    void AnsiTerminal::parseCSISequence(CSISequence & seq) {
        switch (seq.firstByte()) {
            // the "normal" CSI sequences
            case 0: 
                switch (seq.finalByte()) {
                    // CSI <n> @ -- insert blank characters (ICH)
                    case '@':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "ICH: deleteCharacter " << seq[0];
                        insertCharacters(seq[0]);
                        return;
                    // CSI <n> A -- moves cursor n rows up (CUU)
                    case 'A': {
                        seq.setDefault(0, 1);
                        if (seq.numArgs() != 1)
                            break;
                        unsigned r = state_.cursor.y() >= seq[0] ? state_.cursor.y() - seq[0] : 0;
                        LOG(SEQ) << "CUU: setCursor " << state_.cursor.x() << ", " << r;
                        state_.setCursor(state_.cursor.x(), r);
                        return;
                    }
                    // CSI <n> B -- moves cursor n rows down (CUD)
                    case 'B':
                        seq.setDefault(0, 1);
                        if (seq.numArgs() != 1)
                            break;
                        LOG(SEQ) << "CUD: setCursor " << state_.cursor.x() << ", " << state_.cursor.y() + seq[0];
                        state_.setCursor(state_.cursor.x(), state_.cursor.y() + seq[0]);
                        return;
                    // CSI <n> C -- moves cursor n columns forward (right) (CUF)
                    case 'C':
                        seq.setDefault(0, 1);
                        if (seq.numArgs() != 1)
                            break;
                        LOG(SEQ) << "CUF: setCursor " << state_.cursor.x() + seq[0] << ", " << state_.cursor.y();
                        state_.setCursor(state_.cursor.x() + seq[0], state_.cursor.y());
                        return;
                    // CSI <n> D -- moves cursor n columns back (left) (CUB)
                    case 'D': {// cursor backward
                        seq.setDefault(0, 1);
                        if (seq.numArgs() != 1)
                            break;
                        unsigned c = state_.cursor.x() >= seq[0] ? state_.cursor.x() - seq[0] : 0;
                        LOG(SEQ) << "CUB: setCursor " << c << ", " << state_.cursor.y();
                        state_.setCursor(c, state_.cursor.y());
                        return;
                    }
                    /* CSI <n> G -- set cursor character absolute (CHA)
                    */
                    case 'G':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "CHA: set column " << seq[0] - 1;
                        state_.setCursor(seq[0] - 1, state_.cursor.y());
                        return;
                    /* set cursor position (CUP) */
                    case 'H': // CUP
                    case 'f': // HVP
                        seq.setDefault(0, 1).setDefault(1, 1);
                        if (seq.numArgs() != 2)
                            break;
                        seq.conditionalReplace(0, 0, 1);
                        seq.conditionalReplace(1, 0, 1);
                        LOG(SEQ) << "CUP: setCursor " << seq[1] - 1 << ", " << seq[0] - 1;
                        state_.setCursor(seq[1] - 1, seq[0] - 1);
                        return;
                    /* CSI <n> J -- erase display, depending on <n>:
                        0 = erase from the current position (inclusive) to the end of display
                        1 = erase from the beginning to the current position(inclusive)
                        2 = erase entire display
                    */
                    case 'J':
                        if (seq.numArgs() > 1)
                            break;
                        switch (seq[0]) {
                            case 0:
                                updateCursorPosition();
                                fillRect(Rect::FromCorners(state_.cursor.x(), state_.cursor.y(), state_.buffer.width(), state_.cursor.y() + 1), state_.cell);
                                fillRect(Rect::FromCorners(0, state_.cursor.y() + 1, state_.buffer.width(), state_.buffer.height()), state_.cell);
                                return;
                            case 1:
                                updateCursorPosition();
                                fillRect(Rect::FromCorners(0, 0, state_.buffer.width(), state_.cursor.y()), state_.cell);
                                fillRect(Rect::FromCorners(0, state_.cursor.y(), state_.cursor.x() + 1, state_.cursor.y() + 1), state_.cell);
                                return;
                            case 2:
                                fillRect(Rect::FromWH(state_.buffer.width(), state_.buffer.height()), state_.cell);
                                return;
                            default:
                                break;
                        }
                        break;
                    /* CSI <n> K -- erase in line, depending on <n>
                        0 = Erase to Right
                        1 = Erase to Left
                        2 = Erase entire line
                    */
                    case 'K':
                        if (seq.numArgs() > 1)
                            break;
                        switch (seq[0]) {
                            case 0:
                                updateCursorPosition();
                                fillRect(Rect::FromCorners(state_.cursor.x(), state_.cursor.y(), state_.buffer.width(), state_.cursor.y() + 1), state_.cell);
                                return;
                            case 1:
                                updateCursorPosition();
                                fillRect(Rect::FromCorners(0, state_.cursor.y(), state_.cursor.x() + 1, state_.cursor.y() + 1), state_.cell);
                                return;
                            case 2:
                                updateCursorPosition();
                                fillRect(Rect::FromCorners(0, state_.cursor.y(), state_.buffer.width(), state_.cursor.y() + 1), state_.cell);
                                return;
                            default:
                                break;
                        }
					break;
                    /* CSI <n> L -- Insert n lines. (IL)
                     */
                    case 'L':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "IL: scrollUp " << seq[0];
                        insertLines(seq[0], state_.cursor.y(), state_.scrollEnd, state_.cell);
                        return;
                    /* CSI <n> M -- Remove n lines. (DL)
                     */
                    case 'M':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "DL: scrollDown " << seq[0];
                        deleteLines(seq[0], state_.cursor.y(), state_.scrollEnd, state_.cell);
                        return;
                    /* CSI <n> P -- Delete n charcters. (DCH) 
                     */
                    case 'P':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "DCH: deleteCharacter " << seq[0];
                        deleteCharacters(seq[0]);
                        return;
                    /* CSI <n> S -- Scroll up n lines
                     */
                    case 'S':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "SU: scrollUp " << seq[0];
                        deleteLines(seq[0], state_.scrollStart, state_.scrollEnd, state_.cell);
                        return;
                    /* CSI <n> T -- Scroll down n lines
                     */
                    case 'T':
                        seq.setDefault(0, 1);
                        LOG(SEQ) << "SD: scrollDown " << seq[0];
                        insertLines(seq[0], state_.cursor.y(), state_.scrollEnd, state_.cell);
                        return;
                    /* CSI <n> X -- erase <n> characters from the current position
                     */
                    case 'X': {
                        seq.setDefault(0, 1);
                        if (seq.numArgs() != 1)
                            break;
                        updateCursorPosition();
                        // erase from first line
                        int n = static_cast<unsigned>(seq[0]);
                        int l = std::min(state_.buffer.width() - state_.cursor.x(), n);
                        fillRect(Rect::FromTopLeftWH(state_.cursor, l, 1), state_.cell);
                        n -= l;
                        // while there is enough stuff left to be larger than a line, erase entire line
                        l = state_.cursor.y() + 1;
                        while (n >= state_.buffer.width() && l < state_.buffer.height()) {
                            fillRect(Rect::FromCorners(0, l, state_.buffer.width(), l + 1), state_.cell);
                            ++l;
                            n -= state_.buffer.width();
                        }
                        // if there is still something to erase, erase from the beginning
                        if (n != 0 && l < state_.buffer.height())
                            fillRect(Rect::FromCorners(0, l, n, l + 1), state_.cell);
                        return;
                    }
                    /* CSI <n> b - repeat the previous character n times (REP)
                     */
                    case 'b': {
                        seq.setDefault(0, 1);
                        if (state_.cursor.x() == 0 || state_.cursor.x() + seq[0] >= state_.buffer.width()) {
                            LOG(SEQ_ERROR) << "Repeat previous character out of bounds";
                        } else {
                            LOG(SEQ) << "Repeat previous character " << seq[0] << " times";
                            Cell const & prev = state_.buffer.at(state_.cursor - Point{1, 0});
                            for (size_t i = 0, e = seq[0]; i < e; ++i) {
                                state_.buffer.at(state_.cursor) = prev;
                                state_.cursor += Point{1,0};
                            }
                        }
                        return;
                    }
                    /* CSI <n> c - primary device attributes.
                     */
                    case 'c': {
                        if (seq[0] != 0)
                            break;
                        LOG(SEQ) << "Device Attributes - VT102 sent";
                        ptySend("\033[?6c", 5); // send VT-102 for now, go for VT-220? 
                        return;
                    }
                    /* CSI <n> d -- Line position absolute (VPA)
                     */
                    case 'd': {
                        seq.setDefault(0, 1);
                        if (seq.numArgs() != 1)
                            break;
                        int r = seq[0];
                        if (r < 1)
                            r = 1;
                        else if (r > state_.buffer.height())
                            r = state_.buffer.height();
                        LOG(SEQ) << "VPA: setCursor " << state_.cursor.x() << ", " << r - 1;
                        state_.setCursor(state_.cursor.x(), r - 1);
                        return;
                    }
                    /* CSI <n> h -- Reset mode enable
                      
                       Depending on the argument, certain things are turned on. None of the RM settings are currently supported.
                     */
                    case 'h':
                        break;
                    /* CSI <n> l -- Reset mode disable
                    
                       Depending on the argument, certain things are turned off. Turning the features on/off is not allowed, but if the client wishes to disable something that is disabled, it's happily ignored.
                     */
                    case 'l':
                        seq.setDefault(0, 0);
                        // enable replace mode (IRM) since this is the only mode we allow, do nothing
                        if (seq[0] == 4)
                            return;
                        break;
                    /* SGR
                     */
                    case 'm':
                        return parseSGR(seq);
                    /* CSI <n> ; <n> r -- Set scrolling region (default is the whole window) (DECSTBM)
                     */
                    case 'r':
                        seq.setDefault(0, 1); // inclusive
                        seq.setDefault(1, state_.cursor.y()); // inclusive
                        if (seq.numArgs() != 2)
                            break;
                        // This is not proper 
                        seq.conditionalReplace(0, 0, 1);
                        seq.conditionalReplace(1, 0, 1);
                        if (seq[0] > state_.buffer.height())
                            break;
                        if (seq[1] > state_.buffer.height())
                            break;
                        state_.scrollStart = std::min(seq[0] - 1, state_.buffer.height() - 1); // inclusive
                        state_.scrollEnd = std::min(seq[1], state_.buffer.height()); // exclusive 
                        state_.setCursor(0, 0);
                        LOG(SEQ) << "Scroll region set to " << state_.scrollStart << " - " << state_.scrollEnd;
                        return;
                    /* CSI <n> : <n> : <n> t -- window manipulation (xterm)

                        We do nothing for these at the moment, just recognize the few possibly interesting ones.
                     */
                    case 't':
                        seq.setDefault(0, 0).setDefault(1, 0).setDefault(2, 0);
                        switch (seq[0]) {
                        case 22:
                            // 22;0;0 -- save xterm icon and window title on stack
                            if (seq[1] == 0 && seq[2] == 0)
                                return;
                            break;
                        case 23:
                            // 23;0;0 -- restore xterm icon and window title from stack
                            if (seq[1] == 0 && seq[2] == 0)
                                return;
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            // getters and setters
            case '?':
                switch (seq.finalByte()) {
                    case 'h':
                        return parseCSIGetterOrSetter(seq, true);
                    case 'l':
                        return parseCSIGetterOrSetter(seq, false);
                    case 's':
                    case 'r':
                        return parseCSISaveOrRestore(seq);
                    default:
                        break;
                }
                break;
            // other CSI sequences
            case '>':
                switch (seq.finalByte()) {
    				/* CSI > 0 c -- Send secondary device attributes.
                     */
                    case 'c':
                        if (seq[0] != 0)
                            break;
					LOG(SEQ) << "Secondary Device Attributes - VT100 sent";
					ptySend("\033[>0;0;0c", 9); // we are VT100, no version third must always be zero (ROM cartridge)
					return;
				default:
					break;
                }
                break;
            default:
                break;
        }
		LOG(SEQ_UNKNOWN) << " Unknown CSI sequence " << seq;
    }

    void AnsiTerminal::parseCSIGetterOrSetter(CSISequence & seq, bool value) {
		for (size_t i = 0; i < seq.numArgs(); ++i) {
			int id = seq[i];
			switch (id) {
				/* application cursor mode on/off
				 */
				case 1:
					state_.cursorMode = value ? CursorMode::Application : CursorMode::Normal;
					LOG(SEQ) << "application cursor mode: " << value;
					continue;
				/* Smooth scrolling -- ignored*/
				case 4:
					LOG(SEQ_WONT_SUPPORT) << "Smooth scrolling: " << value;
					continue;
				/* DECAWM - autowrap mode on/off */
				case 7:
					if (value) {
						LOG(SEQ) << "autowrap mode enable (by default)";
                    } else {
						LOG(SEQ_UNKNOWN) << "CSI?7l, DECAWM does not support being disabled";
                    }
					continue;
				// cursor blinking
				case 12:
                    state_.cursorBlink = value;
					LOG(SEQ) << "cursor blinking: " << value;
					continue;
				// cursor show/hide
				case 25:
                    state_.cursorVisible = value;
					LOG(SEQ) << "cursor visible: " << value;
					continue;
				/* Mouse tracking movement & buttons.

				https://stackoverflow.com/questions/5966903/how-to-get-mousemove-and-mouseclick-in-bash
				*/
				/* Enable normal mouse mode, i.e. report button press & release events only.
				 */
				case 1000:
                    state_.mouseMode = value ? MouseMode::Normal : MouseMode::Off;
					LOG(SEQ) << "normal mouse tracking: " << value;
					continue;
				/* Mouse highlighting - will not support because it requires supporting application and may hang terminal if not used properly, which sounds rather dangerous.
				 */
				case 1001:
					LOG(SEQ_WONT_SUPPORT) << "hilite mouse mode";
					continue;
				/* Mouse button events (report mouse button press & release and mouse movement if any of the buttons is down.
				 */
				case 1002:
                    state_.mouseMode = value ? MouseMode::ButtonEvent : MouseMode::Off;
					LOG(SEQ) << "button-event mouse tracking: " << value;
					continue;
				/* Report all mouse events (i.e. report mouse move even when buttons are not pressed).
				 */
				case 1003:
                    state_.mouseMode = value ? MouseMode::All : MouseMode::Off;
					LOG(SEQ) << "all mouse tracking: " << value;
					continue;
				/* UTF8 encoded tracking.
				 */
				case 1005:
					//mouseEncoding_ = value ? MouseEncoding::UTF8 : MouseEncoding::Default;
					LOG(SEQ_WONT_SUPPORT) << "UTF8 mouse encoding: " << value;
					continue;
				/* SGR mouse encoding.
				 */
				case 1006: // 
					state_.mouseEncoding = value ? MouseEncoding::SGR : MouseEncoding::Default;
					LOG(SEQ) << "UTF8 mouse encoding: " << value;
					continue;
				/* Enable or disable the alternate screen buffer.
				 */
				case 47:
				case 1049: /*
                    // stop autoscrolling so that we don't scroll the alternate buffer at all
                    stopAutoScroll();
					if (value) {
						// flip to alternate buffer and clear it
						if (!alternateBufferMode_) {
							alternateBuffer_ = buffer_;
							std::swap(state_, alternateState_);
							invalidateLastCharPosition();
                            // disable terminal history for alternate mode
                            enableScrolling(false);
						}
                        state_.cell.setFg(palette_->defaultForeground()) 
                                   .setDecoration(palette_->defaultForeground()) 
                                   .setBg(palette_->defaultBackground())
                                   .setFont(Font{})
                                   .setAttributes(Attributes{});
						fillRect(Rect::FromWH(buffer_.cols(), buffer_.rows()), state_.cell);
						buffer_.cursor().pos = Point{0,0};
                        buffer_.cursor().visible = true;
						LOG(SEQ) << "Alternate screen on";
					} else {
						// go back from alternate buffer
						if (alternateBufferMode_) {
							buffer_ = alternateBuffer_;
							std::swap(state_, alternateState_);
                            // enable history for normal mode
	                        enableScrolling();
					    }
						LOG(SEQ) << "Alternate screen off";
					}
					alternateBufferMode_ = value;
                    */
					continue;
				/* Enable/disable bracketed paste mode. When enabled, if user pastes code in the window, the contents should be enclosed with ESC [200~ and ESC[201~ so that the client app can determine it is contents of the clipboard (things like vi might otherwise want to interpret it. 
				 */
				case 2004:
					state_.bracketedPaste = value;
					continue;
				default:
					break;
			}
			LOG(SEQ_UNKNOWN) << "Invalid Get/Set command: " << seq;
		}
    }

    void AnsiTerminal::parseCSISaveOrRestore(CSISequence & seq) {
		for (size_t i = 0; i < seq.numArgs(); ++i)
			LOG(SEQ_WONT_SUPPORT) << "Private mode " << (seq.finalByte() == 's' ? "save" : "restore") << ", id " << seq[i];
    }

    void AnsiTerminal::parseSGR(CSISequence & seq) {
        seq.setDefault(0, 0);
		for (size_t i = 0; i < seq.numArgs(); ++i) {
			switch (seq[i]) {
				/* Resets all attributes. */
				case 0:
                    state_.cell.setFg(palette_->defaultForeground()) 
                               .setDecor(palette_->defaultForeground()) 
                               .setBg(palette_->defaultBackground())
                               .setFont(Font{});
                    state_.inverseMode = false;
                    LOG(SEQ) << "font fg bg reset";
					break;
				/* Bold / bright foreground. */
				case 1:
					state_.cell.setFont(state_.cell.font().setBold());
					LOG(SEQ) << "bold set";
					break;
				/* faint font (light) - won't support for now, though in theory we easily can. */
				case 2:
					LOG(SEQ_WONT_SUPPORT) << "faint font";
					break;
				/* Italics */
				case 3:
					state_.cell.setFont(state_.cell.font().setItalic());
					LOG(SEQ) << "italics set";
					break;
				/* Underline */
				case 4:
                    state_.cell.setFont(state_.cell.font().setUnderline());
					LOG(SEQ) << "underline set";
					break;
				/* Blinking text */
				case 5:
                    state_.cell.setFont(state_.cell.font().setBlink());
					LOG(SEQ) << "blink set";
					break;
				/* Inverse on */
				case 7:
                    if (! state_.inverseMode) {
                        state_.inverseMode = true;
                        Color fg = state_.cell.fg();
                        Color bg = state_.cell.bg();
                        state_.cell.setFg(bg).setDecor(bg).setBg(fg);
    					LOG(SEQ) << "inverse mode on";
                    }
                    break;
				/* Strikethrough */
				case 9:
                    state_.cell.setFont(state_.cell.font().setStrikethrough());
					LOG(SEQ) << "strikethrough";
					break;
				/* Bold off */
				case 21:
					state_.cell.setFont(state_.cell.font().setBold(false));
					LOG(SEQ) << "bold off";
					break;
				/* Normal - neither bold, nor faint. */
				case 22:
					state_.cell.setFont(state_.cell.font().setBold(false).setItalic(false));
					LOG(SEQ) << "normal font set";
					break;
				/* Italics off. */
				case 23:
					state_.cell.setFont(state_.cell.font().setItalic(false));
					LOG(SEQ) << "italics off";
					break;
				/* Disable underline. */
				case 24:
                    state_.cell.setFont(state_.cell.font().setUnderline(false));
					LOG(SEQ) << "undeline off";
					break;
				/* Disable blinking. */
				case 25:
                    state_.cell.setFont(state_.cell.font().setBlink(false));
					LOG(SEQ) << "blink off";
					break;
                /* Inverse off */
				case 27: 
                    if (state_.inverseMode) {
                        state_.inverseMode = false;
                        Color fg = state_.cell.fg();
                        Color bg = state_.cell.bg();
                        state_.cell.setFg(bg).setDecor(bg).setBg(fg);
    					LOG(SEQ) << "inverse mode off";
                    }
                    break;
				/* Disable strikethrough. */
				case 29:
                    state_.cell.setFont(state_.cell.font().setStrikethrough(false));
					LOG(SEQ) << "Strikethrough off";
					break;
				/* 30 - 37 are dark foreground colors, handled in the default case. */
				/* 38 - extended foreground color */
				case 38: {
                    Color fg = parseSGRExtendedColor(seq, i);
                    state_.cell.setFg(fg).setDecor(fg);    
					LOG(SEQ) << "fg set to " << fg;
					break;
                }
				/* Foreground default. */
				case 39:
                    state_.cell.setFg(palette_->defaultForeground())
                               .setDecor(palette_->defaultForeground());
					LOG(SEQ) << "fg reset";
					break;
				/* 40 - 47 are dark background color, handled in the default case. */
				/* 48 - extended background color */
				case 48: {
                    Color bg = parseSGRExtendedColor(seq, i);
                    state_.cell.setBg(bg);    
					LOG(SEQ) << "bg set to " << bg;
					break;
                }
				/* Background default */
				case 49:
					state_.cell.setBg(palette_->defaultBackground());
					LOG(SEQ) << "bg reset";
					break;
				/* 90 - 97 are bright foreground colors, handled in the default case. */
				/* 100 - 107 are bright background colors, handled in the default case. */
				default:
					if (seq[i] >= 30 && seq[i] <= 37) {
                        int colorIndex = seq[i] - 30;
                        if (boldIsBright_ && state_.cell.font().bold())
                            colorIndex += 8;
						state_.cell.setFg(palette_->at(colorIndex))
                                   .setDecor(palette_->at(colorIndex));
						LOG(SEQ) << "fg set to " << palette_->at(seq[i] - 30);
					} else if (seq[i] >= 40 && seq[i] <= 47) {
						state_.cell.setBg(palette_->at(seq[i] - 40));
						LOG(SEQ) << "bg set to " << palette_->at(seq[i] - 40);
					} else if (seq[i] >= 90 && seq[i] <= 97) {
						state_.cell.setFg(palette_->at(seq[i] - 82))
                                   .setDecor(palette_->at(seq[i] - 82));
						LOG(SEQ) << "fg set to " << palette_->at(seq[i] - 82);
					} else if (seq[i] >= 100 && seq[i] <= 107) {
						state_.cell.setBg(palette_->at(seq[i] - 92));
						LOG(SEQ) << "bg set to " << palette_->at(seq[i] - 92);
					} else {
						LOG(SEQ_UNKNOWN) << "Invalid SGR code: " << seq;
					}
					break;
			}
		}
    }

    Color AnsiTerminal::parseSGRExtendedColor(CSISequence & seq, size_t & i) {
		++i;
		if (i < seq.numArgs()) {
			switch (seq[i++]) {
				/* index from 256 colors */
				case 5:
					if (i >= seq.numArgs()) // not enough args 
						break;
					if (seq[i] > 255) // invalid color spec
						break;
                    return palette_->at(seq[i]);
				/* true color rgb */
				case 2:
					i += 2;
					if (i >= seq.numArgs()) // not enough args
						break;
					if (seq[i - 2] > 255 || seq[i - 1] > 255 || seq[i] > 255) // invalid color spec
						break;
					return Color(seq[i - 2] & 0xff, seq[i - 1] & 0xff, seq[i] & 0xff);
				/* everything else is an error */
				default:
					break;
			}
		}
		LOG(SEQ_UNKNOWN) << "Invalid extended color: " << seq;
		return Color::White;
    }

    void AnsiTerminal::parseOSCSequence(OSCSequence & seq) {
        switch (seq.num()) {
            /* OSC 0 - change the terminal title.
             */
            /*
            case 0:
    			LOG(SEQ) << "Title change to " << seq.value();
                buffer_.unlock();
                try {
                    setTitle(seq.value());
                } catch (std::exception const & e) {
                    LOG(SEQ_ERROR) << e.what();
                } catch (...) {
                    LOG(SEQ_ERROR) << "unknown error";
                }
                buffer_.lock();
                break;
            */
            /* OSC 52 - set clipboard to given value. 
             */
            /*
            case 52:
                LOG(SEQ) << "Clipboard set to " << seq.value();
                buffer_.unlock();
                try {
                    setClipboardContents(seq.value());
                } catch (std::exception const & e) {
                    LOG(SEQ_ERROR) << e.what();
                } catch (...) {
                    LOG(SEQ_ERROR) << "unknown error";
                }
                buffer_.lock();
                break;
            */
            /* OSC 112 - reset cursor color. 
             */
            case 112:
                NOT_IMPLEMENTED;
            default:
        		LOG(SEQ_UNKNOWN) << "Invalid OSC sequence: " << seq;
        }
    }

    void AnsiTerminal::fillRect(Rect const& rect, Cell const & cell) {
		for (int row = rect.top(); row < rect.bottom(); ++row) {
			for (int col = rect.left(); col < rect.right(); ++col) {
				state_.buffer.at(col, row) = cell;
			}
		}
    }

    // TODO change to int 
    void AnsiTerminal::deleteCharacters(unsigned num) {
		int r = state_.cursor.y();
		for (unsigned c = state_.cursor.x(), e = state_.buffer.width() - num; c < e; ++c) 
			Cell& cell = state_.buffer.at(c, r) = state_.buffer.at(c + num, r);
		for (unsigned c = state_.buffer.width() - num, e = state_.buffer.width(); c < e; ++c)
			state_.buffer.at(c, r) = state_.cell;
    }

    void AnsiTerminal::insertCharacters(unsigned num) {
		unsigned r = state_.cursor.y();
		// first copy the characters
		for (unsigned c = state_.buffer.width() - 1, e = state_.cursor.x() + num; c >= e; --c)
			state_.buffer.at(c, r) = state_.buffer.at(c - num, r);
		for (unsigned c = state_.cursor.x(), e = state_.cursor.x() + num; c < e; ++c)
			state_.buffer.at(c, r) = state_.cell;
    }

    void AnsiTerminal::deleteLines(int lines, int top, int bottom, Cell const & fill) {
        // if we are deleting lines from the top of the screen, they can go to the history buffer if any
        if (top == 0 && ! alternateMode_) {
            int oldHistoryRows = state_.historyRows();
            state_.addHistoryRows(lines, palette_->defaultBackground());
            setScrollHeight(height() + state_.historyRows());
            // if the window was scrolled to the end, keep it scrolled to the end as well
            // this means that when the scroll buffer overflows, the scroll offset won't change, but its contents would
            // for now I think this is a feature as you then know that your scroll buffer is overflowing
            if (oldHistoryRows == scrollOffset().y()) 
                setScrollOffset(Point{0, state_.historyRows()});
        }
        // now move the lines accordingly by swapping the rows in the buffer
        // TODO this could be done faster if more than 1 line is being used
        while (lines-- > 0) {
            Cell ** rows = state_.buffer.rows_;
            Cell * x = rows[top];
            memmove(rows + top, rows + top + 1, sizeof(Cell*) * (bottom - top - 1));
            state_.buffer.fillRow(x, state_.cell, state_.buffer.width());
            rows[bottom - 1] = x;
        }
    }

    void AnsiTerminal::insertLines(int lines, int top, int bottom, Cell const & cell) {
        while (lines-- > 0) {
            Cell ** rows = state_.buffer.rows_;
            Cell * x = rows[bottom - 1];
            memmove(rows + top + 1, rows + top, sizeof(Cell*) * (bottom - top - 1));
            state_.buffer.fillRow(x, state_.cell, state_.buffer.width());
            state_.buffer.rows_[top] = x;
        }
    }

    void AnsiTerminal::updateCursorPosition() {
        while (state_.cursor.x() >= state_.buffer.width()) {
            state_.cursor -= Point{state_.buffer.width(), -1};
            // if the cursor is on the last line, evict the lines above
            if (state_.cursor.y() == state_.scrollEnd) 
                deleteLines(1, state_.scrollStart, state_.scrollEnd, state_.cell);
        }
        if (state_.cursor.y() >= state_.buffer.height())
            state_.cursor.setY(state_.buffer.height() - 1);
        // the cursor position must be valid now
        ASSERT(state_.cursor.x() < state_.buffer.width());
        ASSERT(state_.cursor.y() < state_.buffer.height());
        // set last character position to the now definitely valid cursor coordinates
        state_.lastCharacter = state_.cursor;
    }

    // ============================================================================================
    // AnsiTerminal::Palette

    AnsiTerminal::Palette AnsiTerminal::Palette::Colors16() {
		return Palette{
			Color::Black, // 0
			Color::DarkRed, // 1
			Color::DarkGreen, // 2
			Color::DarkYellow, // 3
			Color::DarkBlue, // 4
			Color::DarkMagenta, // 5
			Color::DarkCyan, // 6
			Color::Gray, // 7
			Color::DarkGray, // 8
			Color::Red, // 9
			Color::Green, // 10
			Color::Yellow, // 11
			Color::Blue, // 12
			Color::Magenta, // 13
			Color::Cyan, // 14
			Color::White // 15
		};
    }

    AnsiTerminal::Palette AnsiTerminal::Palette::XTerm256() {
        Palette result(256);
        // first the basic 16 colors
		result[0] =	Color::Black;
		result[1] =	Color::DarkRed;
		result[2] =	Color::DarkGreen;
		result[3] =	Color::DarkYellow;
		result[4] =	Color::DarkBlue;
		result[5] =	Color::DarkMagenta;
		result[6] =	Color::DarkCyan;
		result[7] =	Color::Gray;
		result[8] =	Color::DarkGray;
		result[9] =	Color::Red;
		result[10] = Color::Green;
		result[11] = Color::Yellow;
		result[12] = Color::Blue;
		result[13] = Color::Magenta;
		result[14] = Color::Cyan;
		result[15] = Color::White;
		// now do the xterm color cube
		unsigned i = 16;
		for (unsigned r = 0; r < 256; r += 40) {
			for (unsigned g = 0; g < 256; g += 40) {
				for (unsigned b = 0; b < 256; b += 40) {
					result[i] = Color(
						static_cast<unsigned char>(r),
						static_cast<unsigned char>(g),
						static_cast<unsigned char>(b)
					);
					++i;
					if (b == 0)
						b = 55;
				}
				if (g == 0)
					g = 55;
			}
			if (r == 0)
				r = 55;
		}
		// and finally do the grayscale
		for (unsigned char x = 8; x <= 238; x += 10) {
			result[i] = Color(x, x, x);
			++i;
		}
        return result;
    }

    AnsiTerminal::Palette::Palette(std::initializer_list<Color> colors, size_t defaultFg, size_t defaultBg):
        size_(colors.size()),
        defaultFg_(defaultFg),
        defaultBg_(defaultBg),
        colors_(new Color[colors.size()]) {
        ASSERT(defaultFg < size_ && defaultBg < size_);
		unsigned i = 0;
		for (Color c : colors)
			colors_[i++] = c;
    }

    AnsiTerminal::Palette::Palette(Palette const & from):
        size_(from.size_),
        defaultFg_(from.defaultFg_),
        defaultBg_(from.defaultBg_),
        colors_(new Color[from.size_]) {
		memcpy(colors_, from.colors_, sizeof(Color) * size_);
    }

    AnsiTerminal::Palette::Palette(Palette && from):
        size_(from.size_),
        defaultFg_(from.defaultFg_),
        defaultBg_(from.defaultBg_),
        colors_(from.colors_) {
        from.colors_ = nullptr;
        from.size_ = 0;
    }

    // ============================================================================================
    // AnsiTerminal::Buffer

    void AnsiTerminal::State::addHistoryRows(int numRows, Color defaultBg) {
        if (maxHistoryRows == 0)
            return;
        for (int i = 0; i < numRows; ++i) {
            Cell * row = buffer.rows_[i];
            int lastCol = buffer.width() - 1;
            while (lastCol > 0) {
                Cell const & c = row[lastCol];
                if (c.codepoint() != ' ' || c.bg() != defaultBg)
                    break;
                --lastCol;
            }
            ++lastCol;
            Cell * historyRow = new Cell[lastCol];
            memcpy(historyRow, row, sizeof(Cell) * lastCol);
            if (history_.size() >= maxHistoryRows) {
                delete history_.front().second;
                history_.pop_front();
            }
            history_.push_back(std::make_pair(lastCol, historyRow));
        }
    }

    void AnsiTerminal::State::drawHistoryRows(Canvas & canvas, int start, int end) {
        for (; start < end; ++start) {
            for (int col = 0, ce = history_[start].first; col < ce; ++col)
                canvas.set(Point{col, start}, history_[start].second[col]);
        }
    }

    // ============================================================================================
    // AnsiTerminal::Buffer

    void AnsiTerminal::Buffer::fillRow(Cell * row, Cell const & fill, unsigned cols) {
        row[0] = fill;
        size_t i = 1;
        size_t next = 2;
        while (next < cols) {
            memcpy(row + i, row, sizeof(Cell) * i);
            i = next;
            next *= 2;
        }
        memcpy(row + i, row, sizeof(Cell) * (cols - i));
    }

    // ============================================================================================
    // AnsiTerminal::CSISequence

    AnsiTerminal::CSISequence AnsiTerminal::CSISequence::Parse(char const * & start, char const * end) {
        CSISequence result;
        char const * x = start;
        // if we are at the end, return incomplete
        if (x == end) {
            result.firstByte_ = INCOMPLETE;
            return result;
        }
        // parse the first byte
        if (IsParameterByte(*x) && *x != ';' && !helpers::IsDecimalDigit(*x))
            result.firstByte_ = *x++;
        ASSERT(result.firstByte_ != INVALID);
        // parse arguments, if any
        while (x != end && IsParameterByte(*x)) {
            // semicolon separates arguments, in this case an empty argument, which is initialized to default value (0)
            if (*x == ';') {
                ++x;
                result.args_.push_back(std::make_pair(DEFAULT_ARG_VALUE, false));
            // otherwise if we see digit, parse the argument given
            } else if (helpers::IsDecimalDigit(*x)) {
                int arg = 0;
                do {
                    arg = arg * 10 + helpers::DecCharToNumber(*x++);
                } while (x != end && helpers::IsDecimalDigit(*x));
                result.args_.push_back(std::make_pair(arg, true));
                // if there is separator, parse it as well
                if (x != end && *x == ';')
                    ++x;
            // other than numeric values are not supported for now
            } else {
                ++x;
                result.firstByte_ = INVALID;
            }
        }
        // parse intermediate bytes, if there are any, the sequence is marked as invalid because these are not supported now
		while (x != end && IsIntermediateByte(*x)) {
			result.firstByte_ = INVALID;
			++x;
		}
		// parse final byte, first check we are not at the end
		if (x == end) {
            result.firstByte_ = INCOMPLETE;
            return result;
        }
		if (IsFinalByte(*x))
			result.finalByte_ = *x++;
		else
			result.firstByte_ = INVALID;
        // log the sequence if invalid
        if (! result.valid())
			LOG(SEQ_UNKNOWN) << "Unknown, possibly invalid CSI sequence: \x1b" << std::string(start - 1 , x - start + 1);
        start = x;
        return result;
    }

    // ============================================================================================

    AnsiTerminal::OSCSequence AnsiTerminal::OSCSequence::Parse(char const * & start, char const * end) {
        OSCSequence result;
        char const * x = start;
        if (x == end) {
            result.num_ = INCOMPLETE;
            return result;
        }
        // parse the number
        if (helpers::IsDecimalDigit(*x)) {
            int arg = 0;
            do {
                arg = arg * 10 + helpers::DecCharToNumber(*x++);
            } while (x != end && helpers::IsDecimalDigit(*x));
            // if there is no semicolon, keep the INVALID in the number, but continue parsing to BEL or ST
            if (x != end && *x == ';') {
                    ++x;
                    result.num_ = arg;
            } 
        }
        // parse the value, which is terminated by either BEL, or ST, which is ESC followed by backslash
        char const * valueStart = x;
        while (true) {
            if (x == end) {
                result.num_ = INCOMPLETE;
                return result;
            }
            // BEL
            if (*x == helpers::Char::BEL)
                break;
            // ST
            if (*x == helpers::Char::ESC && x + 1 != end && x[1] == '\\') {
                ++x;
                break;
            }
            // next
            ++x;
        }
        result.value_ = std::string(valueStart, x - valueStart);
        ++x; // past the terminating character
        start = x;
        return result;
    }

} // namespace ui