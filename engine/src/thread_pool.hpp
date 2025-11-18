// thread_pool.hpp

#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace utils
    {
        class cBuffer;

        using TaskFunction = std::function<void(cBuffer* const data)>;

        class cTask
        {
        public:
            cTask() = default;
            explicit cTask(const cBuffer* const data, const TaskFunction& function);
            explicit cTask(cTask&& task) noexcept;
            cTask& operator=(cTask&& task) noexcept;
            ~cTask() = default;

            void Run();
            inline cBuffer* GetData() { return _data; }
            inline TaskFunction GetFunction() { return _function; }

        private:
            cBuffer* _data = nullptr;
            TaskFunction _function;
        };

        class mThread
        {
        public:
            explicit mThread(const app::cApplication* const app, const types::usize threadCount = std::thread::hardware_concurrency());
            ~mThread();
            
            void Submit(cTask& task);
            void Stop();

        private:
            app::cApplication* _app = nullptr;
            std::vector<std::thread> _threads = {};
            std::queue<cTask> _tasks = {};
            std::mutex _mtx;
            std::condition_variable _cv;
            types::boolean _stop = types::K_FALSE;
        };
    }
}