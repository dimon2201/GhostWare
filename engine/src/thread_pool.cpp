// thread_pool.cpp

#pragma once

#include <iostream>
#include "application.hpp"
#include "thread_pool.hpp"
#include "buffer.hpp"

namespace realware
{
    using namespace app;
    using namespace types;

    namespace utils
    {
        cTask::cTask(const cBuffer* const data, const TaskFunction& function) : _data((cBuffer*)data), _function(function)
        {
        }

        cTask::cTask(cTask&& task) noexcept : _data(task.GetData()), _function(std::move(task.GetFunction()))
        {
        }

        cTask& cTask::operator=(cTask&& task) noexcept
        {
            if (this != &task)
            {
                _data = task.GetData();
                _function = std::move(task.GetFunction());
            }

            return *this;
        }

        void cTask::Run()
        {
            _function(_data);
        }

        mThread::mThread(const cApplication* const app, const usize threadCount) : _app((cApplication*)app), _stop(K_FALSE)
        {
            for (usize i = 0; i < threadCount; ++i)
            {
                _threads.emplace_back([this] {
                    while (K_TRUE)
                    {
                        cTask task;
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
                        task.Run();
                    }
                });
            }
        }

        mThread::~mThread()
        {
            Stop();

            _cv.notify_all();

            for (auto& thread : _threads)
                thread.join();
        }

        void mThread::Submit(cTask& task)
        {
            {
                std::unique_lock<std::mutex> lock(_mtx);
                _tasks.emplace(std::move(task));
            }

            _cv.notify_one();
        }

        void mThread::Stop()
        {
            std::unique_lock<std::mutex> lock(_mtx);
            _stop = K_TRUE;
        }
    }
}