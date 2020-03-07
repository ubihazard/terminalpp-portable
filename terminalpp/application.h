#pragma once

#include "helpers/helpers.h"
#include "helpers/json.h"

namespace tpp2 {

    class Window;

    class Application {
    public:

        static Application * Instance() {
            return Singleton_();
        }

        virtual ~Application() {
        }

        /** Displays an alert box with single button to dismiss. 
         */
        virtual void alert(std::string const & message) = 0;

        /** Opens given local filename using the system viewer or editor. 
         
            Application implementations must provide native implementation of this functionality. 
         */
        virtual void openLocalFile(std::string const & filename, bool edit) = 0;

        /** Creates new renderer window. 
         */
        virtual Window * createWindow(std::string const & title, int cols, int rows) = 0;

        /** The main event loop of the application. 
         */
        virtual void mainLoop() = 0;

    protected:
        Application() {
            ASSERT(Singleton_() == nullptr) << "Application assumed to be singleton";
            Singleton_() = this;
        }

    private:

        static Application *& Singleton_() {
            static Application * singleton_ = nullptr;
            return singleton_;
        }

    }; // tpp::Application

} // namespace tpp

namespace tpp {

    class Window;

    class Application {
    public:
        static Application * Instance() {
            return Singleton();
        }

        static void Alert(std::string const & message) {
            Instance()->alert(message);
        }

        /** Opens given local filename using the system viewer or editor. 
         */
        static void Open(std::string const & localFile, bool edit = false) {
            Instance()->openLocalFile(localFile, edit);
        }

        virtual ~Application() {
        }

        virtual Window * createWindow(std::string const & title, int cols, int rows, unsigned cellHeightPx) = 0;

        /** The main event loop of the application. 
         */
        virtual void mainLoop() = 0;

    protected:

        friend class Config;

        Application() {
            ASSERT(Singleton() == nullptr) << "Application assumed to be singleton";
            Singleton() = this;
        }

        /** Displays an alert box with single button to dismiss. 
         */
        virtual void alert(std::string const & message) = 0;

        /** Opens given local filename using the system viewer or editor. 
         
            Application implementations must provide native implementation of this functionality. 
         */
        virtual void openLocalFile(std::string const & filename, bool edit) = 0;

    private:

        static Application *& Singleton() {
            static Application * singleton_ = nullptr;
            return singleton_;
        }

    }; // tpp::Application


} // namespace tpp