//
// Created by DEV on 2026/3/2.
//

#ifndef STD_FUNCTION_EVENTSYSTEM_H
#define STD_FUNCTION_EVENTSYSTEM_H
#include <vector>
#include <string>
#include <functional>

/**
 * 事件系统（观察者模式）
 */
class EventSystem {
private:
    std::vector<std::function<void (const std::string&)>> listeners;
public:
    void addListener(const std::function<void (const std::string&)>& listener) {
        listeners.push_back(listener);
    }
    void  triggerEvent(const std::string& msg) {
        for (auto& listener : listeners) {
            listener(msg);
        }
    }
};


#endif //STD_FUNCTION_EVENTSYSTEM_H