#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace helpers {

    class ResourceGrabber {
    public:
        ResourceGrabber(std::atomic<unsigned> & counter, std::condition_variable & cv):
            counter_(counter),
            cv_(cv) {
            ++counter_;
        }

        ~ResourceGrabber() {
            if (--counter_ == 0)
                cv_.notify_all();
        }

    private:
        std::atomic<unsigned> & counter_;
        std::condition_variable & cv_;
    };

    /** A simple RAII pointer to a class providing unlock() method which is called when the pointer goes out of scope.
     */
    template<typename T>
    class SmartRAIIPtr {
    public:
        SmartRAIIPtr():
            value_(nullptr) {
        }

        SmartRAIIPtr(T & value, bool lock = true):
            value_( &value) {
            if (lock)
                value.lock();
        }

        SmartRAIIPtr(SmartRAIIPtr && from) noexcept:
            value_(from.value_) {
            from.value_ = nullptr;
        }

        ~SmartRAIIPtr() {
            if (value_ != nullptr)
                value_->unlock();
        }

        SmartRAIIPtr & operator = (SmartRAIIPtr && from) {
            if (value_ != nullptr)
                value_->unlock();
            value_ = from.value_;
            from.value_ = nullptr;
            return * this;
        }

        SmartRAIIPtr(SmartRAIIPtr const &) = delete;
        SmartRAIIPtr & operator = (SmartRAIIPtr const &) = delete;

        T * release() {
            T * result = value_;
            value_ = nullptr;
            return result;
        }

        T const & operator * () const {
            ASSERT(value_ != nullptr);
            return *value_;
        }

        T & operator * () {
            ASSERT(value_ != nullptr);
            return *value_;
        }

        T const * operator -> () const {
            ASSERT(value_ != nullptr);
            return value_;
        }

        T * operator -> () {
            ASSERT(value_ != nullptr);
            return value_;
        }

    private:

        T * value_;

    }; // helpers::SmartRAIIPtr


    /** A simple lock that allows locking in normal and priority modes, guaranteeing that a priority lock request will be serviced before any waiting normal locks. 
     
        The lock is reentrant, i.e. a single thread can acquire it multiple times. 
     */
    class PriorityLock {
    public:

        PriorityLock():
            depth_{0},
            priorityRequests_(0) {
        }

        /** Grabs the lock in non-priority mode.
         */
        void lock() {
            if (owner_ != std::this_thread::get_id()) {
                std::unique_lock<std::mutex> g(m_);
                while (priorityRequests_ > 0)
                    cv_.wait(g);
                g.release();
                owner_ = std::this_thread::get_id();
            }
            ++depth_;
        }

        /** Grabs the lock in priority mode.
         */
        void priorityLock() {
            ++priorityRequests_;
            if (owner_ != std::this_thread::get_id()) {
                m_.lock();
                owner_ = std::this_thread::get_id();
            }
            --priorityRequests_;
            ++depth_;
        }

        /** Releases the lock. 
         */
        void unlock() {
            ASSERT(owner_ == std::this_thread::get_id());
            ASSERT(depth_ > 0);
            if (--depth_ == 0) {
                owner_ = std::thread::id{};
                cv_.notify_all();
                m_.unlock();
            }
        }

    private:
        std::thread::id owner_;
        unsigned depth_;
        std::atomic<unsigned> priorityRequests_;
        std::mutex m_;
        std::condition_variable cv_;
    }; 

} // namespace helpers