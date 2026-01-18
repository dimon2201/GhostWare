// thread_pool.cpp

#pragma once

#include <iostream>
#include "application.hpp"
#include "thread_manager.hpp"
#include "buffer.hpp"

using namespace types;

namespace arachne
{
    cTask::cTask(cBuffer* data, TaskFunction&& function) : _data(data), _function(std::make_shared<TaskFunction>(std::move(function)))
    {
    }

    void cTask::Run()
    {
        if (_function)
            _function->operator()(_data);
    }

    cThread::cThread(cContext* context, usize threadCount) : cObject(context), _stop(K_FALSE)
    {
        for (usize i = 0; i < threadCount; ++i)
        {
            _threads.emplace_back([this] {
                while (K_TRUE)
                {
                    if (_pause.load() == K_TRUE)
                        continue;

                    cTask task;
                    {
                        std::unique_lock<std::mutex> lock(_mtx);
                        _cv.wait(lock, [this] {
                            return !_tasks.empty() || _stop;
                        });
                        if (_stop && _tasks.empty())
                            return;
                        task = _tasks.front();
                        _tasks.pop();
                    }
                    task.Run();
                }
            });
        }
    }

    cThread::~cThread()
    {
        Stop();

        _cv.notify_all();

        for (auto& thread : _threads)
            thread.join();
    }

    void cThread::Pause()
    {
        _pause.store(K_TRUE);
    }

    void cThread::Resume()
    {
        _pause.store(K_FALSE);
    }

    void cThread::Submit(cTask& task)
    {
        {
            std::unique_lock<std::mutex> lock(_mtx);
            _tasks.emplace(task);
        }

        _cv.notify_one();
    }

    void cThread::Stop()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _stop = K_TRUE;
    }
}