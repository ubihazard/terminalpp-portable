#pragma once

#include "ui/widgets/window.h"
#include "ui/widgets/pager.h"
#include "ui/widgets/panel.h"
#include "ui/widgets/label.h"
#include "ui/widgets/dialog.h"
#include "ui-terminal/ansi_terminal.h"
#include "tpp-lib/local_pty.h"
#include "tpp-lib/bypass_pty.h"
#include "tpp-lib/remote_files.h"

#include "../config.h"
#include "../window.h"

#include "about_box.h"

namespace tpp {

    /** New Version Dialog. 
     */
    class NewVersionDialog : public ui::Dialog::Cancel {
    public:
        NewVersionDialog(std::string const & message):
            ui::Dialog::Cancel{"New Version"},
            contents_{new ui::Label{message}} {
            setBody(contents_);
        }

    private:
        Label * contents_;
    };

    /** The terminal window. 
     
        
     */
    class TerminalWindow : public ui::Window /*, public ui::AutoScroller<TerminalWindow> */ {
    public:

        TerminalWindow(tpp::Window * window):
            window_{window}, 
            main_{new Panel{}},
            pager_{new Pager{}} {

            window_->onClose.setHandler(&TerminalWindow::windowCloseRequest, this);
            window_->onKeyDown.setHandler(&TerminalWindow::windowKeyDown, this);

            main_->setLayout(new Layout::Column{VerticalAlign::Top});
            main_->setBackground(Color::Red);
            main_->attach(pager_);
            setContents(main_);

            Config const & config = Config::Instance();
            remoteFiles_ = new RemoteFiles(config.remoteFiles.dir());

            pager_->onPageChange.setHandler(&TerminalWindow::activeSessionChanged, this);

            window_->setRoot(this);
            versionChecker_ = std::thread{[this](){
                std::string channel = Config::Instance().version.checkChannel();
                // don't check if empty channel
                if (channel.empty())
                    return;
                std::string newVersion = Application::Instance()->checkLatestVersion(channel);
                if (!newVersion.empty()) {
                    schedule([this, newVersion]() {
                        NewVersionDialog * d = new NewVersionDialog{STR("New version " << newVersion << " is available")};
                        showModal(d);
                    });
                }
            }};

            setFocusable(true);

            //setName("TerminalWindow");
        }

        ~TerminalWindow() override {
            versionChecker_.join();
            delete remoteFiles_;
        }

        void newSession(Config::sessions_entry const & session);


    private:
        class SessionInfo {
        public:
            std::string name;
            std::string title;
            AnsiTerminal * terminal;
            bool terminateOnKeyPress;

            SessionInfo(Config::sessions_entry const & session):
                name{session.name()},
                title{session.name()},
                terminateOnKeyPress{false} {
            }

            ~SessionInfo() {
                delete terminal;
            }
        }; 

        /** The window has been requested to close. 
         */
        void windowCloseRequest(tpp::Window::CloseEvent::Payload & e) {
            // TODO Determine what to do here. 
        }

        /** Global hotkeys handling. 
         */
        void windowKeyDown(tpp::Window::KeyEvent::Payload & e) {
            if (*e == SHORTCUT_FULLSCREEN) {
                window_->setFullscreen(! window_->fullscreen());
            } else if (*e == SHORTCUT_SETTINGS) {
                Application::Instance()->openLocalFile(Config::GetSettingsFile(), /* edit = */ true);
            } else if (*e == SHORTCUT_ZOOM_IN || *e == SHORTCUT_ZOOM_IN_ALT) {
                if (window_->zoom() < 10)
                    window_->setZoom(window_->zoom() * 1.25);
            } else if (*e == SHORTCUT_ZOOM_OUT || *e == SHORTCUT_ZOOM_OUT_ALT) {
                if (window_->zoom() > 1)
                    window_->setZoom(std::max(1.0, window_->zoom() / 1.25));
            } else if (*e == SHORTCUT_ABOUT) {
                showModal(new AboutBox{});
            } else {
                return;
            }
            e.stop();
        }

        SessionInfo * sessionInfo(Widget * terminal) {
            AnsiTerminal * t = dynamic_cast<AnsiTerminal*>(terminal);
            ASSERT(t != nullptr);
            auto i = sessions_.find(t);
            ASSERT(i != sessions_.end());
            return (i->second);
        }

        void closeSession(SessionInfo * session) {
            sessions_.erase(session->terminal);
            pager_->removePage(session->terminal);
            delete session;
            // if this was the last session, close the window
            if (sessions_.empty())
                window_->requestClose();
            // otherwise focus the active session instead
            // TODO do we want this always, or should we actually check if the remove session was focused first? 
            else
                window_->setKeyboardFocus(pager_->activePage());
        }

        void sessionTitleChanged(ui::StringEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            si->title = *e;
            if (activeSession_ == si)
                window_->setTitle(*e);
        }

        void sessionNotification(ui::VoidEvent::Payload & event) {
            MARK_AS_UNUSED(event);
            window_->setIcon(tpp::Window::Icon::Notification);
        }



        void keyDown(KeyEvent::Payload & e) override {
            if (activeSession_->terminateOnKeyPress) 
                closeSession(activeSession_);
            else 
                Widget::keyDown(e);
        }

        /*



        bool autoScrollStep(Point by) override {
            return activeSession_->terminal->scrollBy(by);
        }

        */

        void activeSessionChanged(ui::Event<Widget*>::Payload & e) {
            activeSession_ = *e == nullptr ? nullptr : sessionInfo(*e);
        }


        void sessionPTYTerminated(ExitCodeEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            window_->setIcon(tpp::Window::Icon::Notification);
            si->title = STR("Terminated, exit code " << *e);
            if (activeSession_ == si)
                window_->setTitle(si->title);
            Config const & config = Config::Instance();
            if (! config.renderer.window.waitAfterPtyTerminated()) {
                closeSession(activeSession_);
            } else {
                window_->setKeyboardFocus(this);
                activeSession_->terminateOnKeyPress = true;
            }
        }

        void onKeyDown(Renderer::KeyEvent::Payload & e) {
            if (window_->icon() != tpp::Window::Icon::Default)
                window_->setIcon(tpp::Window::Icon::Default);
            // trigger paste event for ourselves so that the paste can be intercepted
            if (*e == SHORTCUT_PASTE) {
                //requestClipboard();
                e.stop();
            } else if (*e == SHORTCUT_ABOUT) {
                showModal(new AboutBox{});
                e.stop();
            } else if (*e == SHORTCUT_SETTINGS) {
                Application::Instance()->openLocalFile(Config::GetSettingsFile(), /* edit */ true); 
            }
        }

        /*

        void terminalMouseMove(UIEvent<MouseMoveEvent>::Payload & event) {
            if (activeSession_->terminal->mouseCaptured())
                return;
            if (activeSession_->terminal->updatingSelection()) {
                if (event->coords.y() < 0 || event->coords.y() >= activeSession_->terminal->height())
                    startAutoScroll(Point{0, event->coords.y() < 0 ? -1 : 1});
                else
                    stopAutoScroll();
            }
        }

        void terminalMouseDown(UIEvent<MouseButtonEvent>::Payload & event) {
            if (activeSession_->terminal->mouseCaptured())
                return;
            if (event->modifiers == 0) {
                if (event->button == MouseButton::Left) {
                    activeSession_->terminal->startSelectionUpdate(event->coords);
                } else if (event->button == MouseButton::Wheel) {
                    requestSelection(); 
                } else if (event->button == MouseButton::Right && ! activeSession_->terminal->selection().empty()) {
                    setClipboard(activeSession_->terminal->getSelectionContents());
                    activeSession_->terminal->clearSelection();
                } else {
                    return;
                }
            }
            event.stop();
        }

        void terminalMouseUp(UIEvent<MouseButtonEvent>::Payload & event) {
            if (activeSession_->terminal->mouseCaptured())
                return;
            if (event->modifiers == 0) {
                if (event->button == MouseButton::Left) 
                    activeSession_->terminal->endSelectionUpdate();
                else
                    return;
            }
            event.stop();
        }

        void terminalMouseWheel(UIEvent<MouseWheelEvent>::Payload & event) {
            AnsiTerminal * t = dynamic_cast<AnsiTerminal*>(event.sender());
            if (t->mouseCaptured())
                return;
            //if (state_.buffer.historyRows() > 0) {
                if (event->by > 0)
                    t->scrollBy(Point{0, -1});
                else 
                    t->scrollBy(Point{0, 1});
                event.stop();
            //}
        }
        */

        void terminalSetClipboard(StringEvent::Payload & e) {
            setClipboard(*e);
        }

        void terminalTppSequence(TppSequenceEvent::Payload & event) {
            SessionInfo * si = sessionInfo(event.sender());
            try {
                switch (event->kind) {
                    case tpp::Sequence::Kind::GetCapabilities:
                        si->terminal->pty()->send(tpp::Sequence::Capabilities{1});
                        break;
                    case tpp::Sequence::Kind::OpenFileTransfer: {
                        Sequence::OpenFileTransfer req(event->payloadStart, event->payloadEnd);
                        si->terminal->pty()->send(remoteFiles_->openFileTransfer(req));
                        break;
                    }
                    case tpp::Sequence::Kind::Data: {
                        Sequence::Data data{event->payloadStart, event->payloadEnd};
                        remoteFiles_->transfer(data);
                        // make sure the UI thread remains responsive
                        window_->yieldToUIThread();
                        break;
                    }
                    case tpp::Sequence::Kind::GetTransferStatus: {
                        Sequence::GetTransferStatus req{event->payloadStart, event->payloadEnd};
                        si->terminal->pty()->send(remoteFiles_->getTransferStatus(req));
                        break;
                    }
                    case tpp::Sequence::Kind::ViewRemoteFile: {
                        Sequence::ViewRemoteFile req{event->payloadStart, event->payloadEnd};
                        RemoteFiles::File * f = remoteFiles_->get(req.id());
                        if (f == nullptr) {
                            si->terminal->pty()->send(Sequence::Nack{req, "No such file"});
                        } else if (! f->ready()) {
                            si->terminal->pty()->send(Sequence::Nack(req, "File not transferred"));
                        } else {
                            // send the ack first in case there are local issues with the opening
                            si->terminal->pty()->send(Sequence::Ack{req, req.id()});
                            Application::Instance()->openLocalFile(f->localPath(), false);
                        }
                        break;
                    }
                    default:
                        LOG() << "Unknown sequence";
                        break;
                }
            } catch (std::exception const & e) {
                showError(e.what());
            }
        }


        tpp::Window * window_;

        ui::Panel * main_;
        ui::Pager * pager_;

        std::unordered_map<AnsiTerminal *, SessionInfo *> sessions_; 

        SessionInfo * activeSession_ = nullptr;

        RemoteFiles * remoteFiles_;

        std::thread versionChecker_;

    };

} // namespace tpp
