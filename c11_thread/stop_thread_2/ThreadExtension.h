//
// Created by DEV on 8/24/23.
//

#ifndef STOP_THREAD_2_THREADEXTENSION_H
#define STOP_THREAD_2_THREADEXTENSION_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


class ThreadExtension {
    enum class ThreadState {
        Stopped = 0,
        Running = 1,
        Paused = 2
    };
public:
    ThreadExtension();

    virtual~ThreadExtension();

public:
    void StartThread();

    void PauseThread();

    void ResumeThread();

    void StopThread();

    int GetThreadState() const;

private:
    void Run();

protected:
    virtual void CustomRun() {};

private:
    ThreadState m_Thread_State;
    std::shared_ptr<std::thread> m_pThread;
    std::mutex m_Mutex;
    std::condition_variable m_Condition_Variable;
    std::atomic<bool> m_Thread_Pause_Flag;
    std::atomic<bool> m_Thread_Stop_Flag;

};


#endif //STOP_THREAD_2_THREADEXTENSION_H
