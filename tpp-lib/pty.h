#pragma once 

#include <atomic>

#include "helpers/process.h"
#include "helpers/events.h"


namespace tpp {

    /** Pseudoterminal master. 
     
        The master supports 
     */
    class PTYMaster {
    public:
        /** Virtual destructor in the PTYMaster base class. 
         */
        virtual ~PTYMaster() { }

        /** Terminates the pseudoterminal. 
         */
        virtual void terminate() = 0;

        /** Sends data. 
         */
        virtual void send(char const * buffer, size_t numBytes) = 0;

        /** Blocks until data are received and returns the size of bytes received in the provided buffer. 
         
            If the pseudoterminal has been terminated returns immediately. 
         */
        virtual size_t receive(char * buffer, size_t bufferSize) = 0;

        /** Resizes the terminal. 
         */
        virtual void resize(int cols, int rows) = 0;

        /** Returns true if the slave has been terminated. 
         */
        bool terminated() const {
            return terminated_;
        }

        /** If the slave has been terminated, return its exit code. 
         */
        helpers::ExitCode exitCode() const {
            if (terminated_)
                return exitCode_;
            THROW(helpers::IOError()) << "Cannot obtain exit code of unterminated pseudoterminal's process";
        }

    protected:

        PTYMaster():
            terminated_{false},
            exitCode_{0} {
        }

        std::atomic<bool> terminated_;
        helpers::ExitCode exitCode_;

    }; // tpp::PTYMaster


    /** Pseudoterminal master. 
     * 
     */
    class PTYSlave {
    public:
        using ResizedEvent = helpers::Event<std::pair<int,int>, PTYSlave>;

        /** Virtual destructor in the PTYSlave base class. 
         */
        virtual ~PTYSlave() { }


        /** Returns the size of the terminal (cols, rows). 
         */
        virtual std::pair<int, int> size() const = 0;

        /** Sends given data. 
         */
        virtual void send(char const * buffer, size_t numBytes) = 0;

        /** Receives data from the terminal. 
         */
        virtual size_t receive(char * buffer, size_t bufferSize) = 0;

        /** An event triggered when the terminal is resized. 
         */
        ResizedEvent onResized;

    }; // tpp::PTYSlave





} // namespace tpp