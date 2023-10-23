//
// Created by DEV on 8/24/23.
//

#include "ThreadExtension.h"

ThreadExtension::ThreadExtension()
        : m_pThread(nullptr),
          m_Thread_Pause_Flag(false),
          m_Thread_Stop_Flag(false),
          m_Thread_State(ThreadState::Stopped) {

}

ThreadExtension::~ThreadExtension() {
    StopThread();
}

void ThreadExtension::StartThread() {
    if (m_pThread == nullptr) {
        m_pThread = std::make_shared<std::thread>(&ThreadExtension::Run, this);

        if (m_pThread != nullptr) {
            m_Thread_Pause_Flag = false;
            m_Thread_Stop_Flag = false;
            m_Thread_State = ThreadState::Running;
        }
    }
}

void ThreadExtension::PauseThread() {
    if (m_pThread != nullptr) {
        if (m_Thread_State == ThreadState::Running) {
            m_Thread_Pause_Flag = true;
            m_Thread_State = ThreadState::Paused;
        }
    }
}

void ThreadExtension::ResumeThread() {
    if (m_pThread != nullptr) {
        if (m_Thread_State == ThreadState::Paused) {
            m_Thread_Pause_Flag = false;
            m_Condition_Variable.notify_all();
            m_Thread_State = ThreadState::Running;
        }
    }
}

void ThreadExtension::StopThread() {
    if (m_pThread == nullptr) {
        return;
    }
    m_Thread_Stop_Flag = true;
    m_Thread_Pause_Flag = false;
    m_Condition_Variable.notify_all();

    if (m_pThread->joinable()) {
        m_pThread->join();
    }

    // 释放
    m_pThread.reset();

    if (m_pThread == nullptr) {
        m_Thread_State = ThreadState::Stopped;
    }

}

int ThreadExtension::GetThreadState() const {
    return (int) m_Thread_State;
}

void ThreadExtension::Run() {
    while (!m_Thread_Stop_Flag) {
        // 指定自定义线程操作
        try {
            CustomRun();
        }
        catch (std::exception &e) {
            break;
        }

        // 暂停
        if (m_Thread_Pause_Flag) {
            std::unique_lock<std::mutex> thread_locker(m_Mutex);
            if (m_Thread_Pause_Flag) {
                // 等待互斥锁
                m_Condition_Variable.wait(thread_locker);
            }

            thread_locker.unlock();

        }
    }

    m_Thread_Pause_Flag = false;
    m_Thread_Stop_Flag = false;
}

