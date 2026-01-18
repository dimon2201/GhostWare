// thread_pool.hpp

#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class cApplication;
    class cBuffer;

    using TaskFunction = std::function<void(cBuffer* const data)>;

    class cTask
    {
    public:
        cTask() = default;
        explicit cTask(cBuffer* data, TaskFunction&& function);
        ~cTask() = default;

        void Run();
        inline cBuffer* GetData() const { return _data; }
        inline std::shared_ptr<TaskFunction> GetFunction() const { return _function; }

    private:
        cBuffer* _data = nullptr;
        std::shared_ptr<TaskFunction> _function;
    };

    class cThread : public cObject
    {
        REALWARE_OBJECT(cThread)

    public:
        explicit cThread(cContext* context, types::usize threadCount = std::thread::hardware_concurrency());
        ~cThread();
        
        void Submit(cTask& task);
        void Pause();
        void Resume();
        void Stop();

    private:
        std::vector<std::thread> _threads = {};
        std::queue<cTask> _tasks = {};
        std::mutex _mtx;
        std::condition_variable _cv;
        std::atomic<types::boolean> _pause = types::K_FALSE;
        types::boolean _stop = types::K_FALSE;
    };
}