#if (defined RENDERER_QT)
#include "qt_window.h"

namespace tpp {

    QtWindow::~QtWindow() {

    }

    void QtWindow::show() {
        NOT_IMPLEMENTED;
    }

    /** Sets the window icon. 
     */
    void QtWindow::setIcon(ui::RootWindow::Icon icon) {
        MARK_AS_UNUSED(icon);
        NOT_IMPLEMENTED;
    }

    QtWindow::QtWindow(std::string const & title, int cols, int rows, unsigned baseCellHeightPx):
        RendererWindow(cols, rows, QtFont::GetOrCreate(ui::Font(), 0, baseCellHeightPx)->widthPx(), baseCellHeightPx)
        {
        MARK_AS_UNUSED(title);
    }

    void QtWindow::requestClipboardContents() {
        NOT_IMPLEMENTED;
    }

    void QtWindow::requestSelectionContents() {
        NOT_IMPLEMENTED;
    }

    void QtWindow::setClipboard(std::string const & contents) {
        MARK_AS_UNUSED(contents);
        NOT_IMPLEMENTED;
    }

    void QtWindow::setSelection(std::string const & contents) {
        MARK_AS_UNUSED(contents);
        NOT_IMPLEMENTED;
    }

    void QtWindow::clearSelection() {
        NOT_IMPLEMENTED;
    }

} // namespace tpp

#endif
