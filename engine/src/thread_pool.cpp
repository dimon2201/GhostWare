// thread_pool.cpp

#pragma once

#include "thread_pool.hpp"

namespace realware
{
    using namespace types;

    namespace utils
    {
        cThreadPool::cThreadPool(types::usize threadCount) : _stop(K_FALSE)
        {
            for (types::usize i = 0; i < threadCount; ++i)
            {
                _threads.emplace_back([this] {
                    while (K_TRUE)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(_mtx);
                            _cv.wait(lock, [this] {
                                return !_tasks.empty() || _stop;
                            });
                            if (_stop && _tasks.empty())
                                return;
                            task = std::move(_tasks.front());
                            _tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        cThreadPool::~cThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _stop = K_TRUE;
            }

            _cv.notify_all();

            for (auto& thread : _threads)
                thread.join();
        }

        void cThreadPool::Enqueue(std::function<void()> task)
        {
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _tasks.emplace(std::move(task));
            }

            _cv.notify_one();
        }
    }
}