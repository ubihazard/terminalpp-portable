#pragma once

#include "helpers/helpers.h"

#include "widget.h"
#include "selection.h"
#include "root_window.h"

namespace ui {

    /** Base class for user interface renderers.
     */
    class Renderer {
    protected:
        friend class RootWindow;

        Renderer():
            rootWindow_(nullptr) {
        }

        virtual ~Renderer() {
            detach();
        }

        virtual int cols() const = 0;
        virtual int rows() const = 0;

        /** Request from the root window to close & detach the renderer. 
         
            In most cases this also means that the renderer will in due time dispose of the root window. 
         */
        virtual void requestClose() = 0;

        /** Paints the window, or its part. 
         */
        virtual void requestRender(ui::Rect const & rect) = 0;

        /** Sets the title of the rendered window. 
         */
        virtual void setTitle(std::string const & title) = 0;

        virtual void setIcon(RootWindow::Icon icon) = 0;

        virtual void requestClipboardContents() = 0;

        virtual void requestSelectionContents() = 0;

        /** Sets the contents of the clipboard to the given text. 
         */
        virtual void setClipboard(std::string const & contents) = 0;

        /** Sets the selection to given value. 
         */
        virtual void setSelection(std::string const & contents) = 0;

        /** Clears the selection. 
         */
        virtual void clearSelection() = 0;

        Canvas::Buffer::Ptr bufferToRender() {
            ASSERT(attached());
            return rootWindow_->buffer(true);
        }

        Cursor cursorToRender() {
            ASSERT(attached());
            return rootWindow_->cursor();
        }

        RootWindow const * rootWindow() const {
            return rootWindow_;
        }

        RootWindow * rootWindow() {
            return rootWindow_;
        }

        void setRootWindow(RootWindow * rootWindow) {
            detach();
            attach(rootWindow);
        }

        bool attached() const {
            return rootWindow_ != nullptr;
        }

        void updateSize(int cols, int rows) {
            if (attached())
                rootWindow_->rendererResized(cols, rows);
        }

        void setFocus(bool value) {
            if (attached())
                rootWindow_->rendererFocused(value);
        }

        /** Mouse events. 
         */
        void mouseDown(int col, int row, MouseButton button, Key modifiers) {
            if (attached())
                rootWindow_->mouseDown(col, row, button, modifiers);
        }

        void mouseUp(int col, int row, MouseButton button, Key modifiers) {
            if (attached())
                rootWindow_->mouseUp(col, row, button, modifiers);
        }

        void mouseWheel(int col, int row, int by, Key modifiers) {
            if (attached())
                rootWindow_->mouseWheel(col, row, by, modifiers);
        }

        void mouseMove(int col, int row, Key modifiers) {
            if (attached())
                rootWindow_->mouseMove(col, row, modifiers);
        }

        /** Keyboard events. 
         */
        void keyChar(helpers::Char c) {
            if (attached())
                rootWindow_->keyChar(c);
        }

        void keyDown(Key k) {
            if (attached())
                rootWindow_->keyDown(k);
        }

        void keyUp(Key k) {
            if (attached())
                rootWindow_->keyUp(k);
        }

        // clipboard events

        void paste(std::string const & contents) {
            if (attached())
                rootWindow_->paste(contents);
        }

        void selectionInvalidated() {
            if (attached())
                rootWindow_->selectionInvalidated(); 
        }

    private:

        virtual void attach(RootWindow * newRootWindow);

        virtual void detach();

        RootWindow * rootWindow_;

    }; // ui::Renderer

} // namespace ui