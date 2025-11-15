// thread_pool.hpp

#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include "types.hpp"

namespace realware
{
    namespace utils
    {
        class cThreadPool
        {
        public:
            explicit cThreadPool(types::usize threadCount = std::thread::hardware_concurrency());
            ~cThreadPool();
            
            void Enqueue(std::function<void()> task);

        private:
            std::vector<std::thread> _threads;
            std::queue<std::function<void()>> _tasks;
            std::mutex _mtx;
            std::condition_variable _cv;
            types::boolean _stop;
        };
    }
}