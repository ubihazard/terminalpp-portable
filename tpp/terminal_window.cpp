#include "helpers/log.h"

#include "application.h"
#include "terminal_window.h"


namespace tpp {

	// TerminalWindow::Properties

	TerminalWindow::Properties::Properties(TerminalWindow const* tw) :
		cols(tw->cols()),
		rows(tw->rows()),
	    fontWidth(tw->cellWidthPx_ * tw->zoom_),
	    fontHeight(tw->cellHeightPx_ * tw->zoom_),
	    zoom(tw->zoom_) {
	}

	// TerminalWindow

	void TerminalWindow::keyChar(vterm::Char::UTF8 c) {
		terminal()->keyChar(c);
	}

	void TerminalWindow::keyDown(vterm::Key key) {
		if (key == vterm::Key::Enter + vterm::Key::Alt) {
			setFullscreen(!fullscreen());
		} else if (key == vterm::Key::F5) {
			LOG << "redraw...";
			redraw();
		// zoom in
		} else if (key == vterm::Key::Equals + vterm::Key::Ctrl) {
			if (zoom() < 10)
				setZoom(zoom() * 1.25);
		// zoom out
		} else if (key == vterm::Key::Minus + vterm::Key::Ctrl) {
			if (zoom() > 1)
			    setZoom(std::max(1.0, zoom() / 1.25));
		}
		else if (key == vterm::Key::F4) {
			if (zoom() == 1)
				setZoom(2);
			else
				setZoom(1);
		} else if (key == vterm::Key::V + vterm::Key::Ctrl) {
			clipboardPaste();
		} else if (key != vterm::Key::Invalid) {
			terminal()->keyDown(key);
		}
	}

	void TerminalWindow::keyUp(vterm::Key key) {
		terminal()->keyUp(key);
	}

	void TerminalWindow::mouseMove(unsigned x, unsigned y) {
		unsigned col = x;
		unsigned row = y;
		convertMouseCoordsToCells(col, row);
		// first deal with selection 
		if (selecting_) {
			if (row == selectionStart_.row && selectionStart_.col * cellWidthPx_ == x) {
				selectionEnd_.col = col;
				selectionEnd_.row = row + 1;
				doInvalidate(false);
			} else if (col != mouseCol_ || row != mouseRow_) {
				selectionEnd_.col = col + 1;
				selectionEnd_.row = row + 1;
				doInvalidate(false);
			}
		}
		// then deal with the event itself
		if (col != mouseCol_ || row != mouseRow_) {
			mouseCol_ = col;
			mouseRow_ = row;
			terminal()->mouseMove(col, row);
		}
	}

	void TerminalWindow::mouseDown(unsigned x, unsigned y, vterm::MouseButton button) {
		convertMouseCoordsToCells(x, y);
		mouseCol_ = x;
		mouseRow_ = y;
		if (!terminal()->backend()->captureMouse()) {
			switch (button) {
				case vterm::MouseButton::Left:
					selecting_ = true;
					selectionStart_.col = mouseCol_;
					selectionStart_.row = mouseRow_;
					selectionEnd_.col = mouseCol_;
					selectionEnd_.row = mouseRow_ + 1;
					doInvalidate(false);
					break;
				case vterm::MouseButton::Right:
					if (!selecting_) {
						vterm::Selection sel = selectedArea();
						if (!sel.empty()) {
							vterm::Terminal::ClipboardUpdateEvent e(nullptr, terminal()->getText(sel));
							clipboardUpdated(e);
							clearSelection();
						}
					}
					break;
				default:
					break;
			}
			if (button == vterm::MouseButton::Left && !selecting_) {
			} 
		}
		terminal()->mouseDown(x, y, button);
	}

	void TerminalWindow::mouseUp(unsigned x, unsigned y, vterm::MouseButton button) {
		convertMouseCoordsToCells(x, y);
		mouseCol_ = x;
		mouseRow_ = y;
		if (selecting_ && button == vterm::MouseButton::Left) {
			selecting_ = false;
		}
		terminal()->mouseUp(x, y, button);
	}

	void TerminalWindow::mouseWheel(unsigned x, unsigned y, int offset) {
		convertMouseCoordsToCells(x, y);
		terminal()->mouseWheel(x, y, offset);
	}


	unsigned TerminalWindow::drawBuffer() {
		// don't do anything if terminal is not attached
		if (terminal() == nullptr)
			return 0;
		vterm::Terminal::Buffer & b = terminal()->buffer();
		if (clearWindow_) {
			forceRepaint_ = true;
			doClearWindow();
			clearWindow_ = false;
		}
		// initialize the first font and colors
		vterm::Color fg;
		vterm::Color bg;
		vterm::Font font;
		unsigned numCells = 0;
		{
			vterm::Terminal::Cell& c = b.at(0, 0);
			fg = c.fg;
			bg = c.bg;
			font = DropBlink(c.font);
		}
		doSetForeground(fg);
		doSetBackground(bg);
		doSetFont(font);
		// TODO -- we are taking a copy in case the cursor is moved while the print happens - this should not happen though and needs some investigation on how it could be achieved, or if simpler synchronization should be implemented in backends... 
		vterm::Terminal::Cursor cursor = terminal()->cursor();
		// if cursor state changed, mark the cell containing it as dirty
		bool cursorInRange = cursor.col < cols() && cursor.row < rows();
		if (!forceRepaint_ && cursorInRange)
			b.at(cursor.col, cursor.row).dirty = true;
		// determine the selection boundary
		bool inSelection = false;
		vterm::Selection sel = selectedArea();
		// now loop over the entire terminal and update the cells
		for (unsigned r = 0, re = rows(); r < re; ++r) {
			for (unsigned c = 0, ce = cols(); c < ce; ++c) {
				// determine if selection should be enabled
				if (!inSelection && sel.contains(c, r)) {
					inSelection = true;
					bg = Application::Instance<>()->selectionBackgroundColor();
					doSetBackground(bg);
				}
				// and disabled
				if (inSelection && ! sel.contains(c, r)) {
					inSelection = false;
				}
				vterm::Terminal::Cell& cell = b.at(c, r);
				if (forceRepaint_ || inSelection || cell.dirty) {
					++numCells;
					// if we are in selection, mark the cell as dirty, otherwise mark as clean
					cell.dirty = inSelection;
					if (fg != cell.fg) {
						fg = cell.fg;
						doSetForeground(fg);
					}
					if (!inSelection && bg != cell.bg) {
						bg = cell.bg;
						doSetBackground(bg);
					}
					if (font != DropBlink(cell.font)) {
						font = DropBlink(cell.font);
						doSetFont(font);
					}
					doDrawCell(c, r, cell);
				}
			}
		}
		// determine whether cursor should be display and display it if so
		if (cursorInRange && cursor.visible && (blink_ || !cursor.blink)) {
			vterm::Terminal::Cell c = b.at(cursor.col, cursor.row);
			c.fg = cursor.color;
			c.bg = vterm::Color::Black();
			c.c = cursor.character;
			c.font = DropBlink(c.font);
			doDrawCursor(cursor.col, cursor.row, c);
			// mark the cursor location as dirty so that cursor is always repainted, because of subpixel renderings we also mark the next and previous cell as dirty so that the subpixel artefacts, if present will be deleted as well
			b.at(cursor.col, cursor.row).dirty = true;
			if (cursor.col > 0)
				b.at(cursor.col - 1, cursor.row).dirty = true;
			if (cursor.col < cols() - 1)
				b.at(cursor.col + 1, cursor.row).dirty = true;

		}
		return numCells;
	}

} // namespace tpp