#include <iostream>

#include "OptionalDemo.h"

void optionalTest();

void optionalTest() {
    OptionalDemo optionalDemo;
    optionalDemo.baseAbility();
    optionalDemo.findElement();
    optionalDemo.delayInit();


    auto config = optionalDemo.parseConfig("brightness: 80 theme: dark timeout: 15.5");
    if (config.has_value()) {
        std::cout << "Parsed config:\n";
        if (config->brightness)
            std::cout << "Brightness: " << *config->brightness << "\n";
        if (config->timeout)
            std::cout << "Timeout: " << *config->timeout << "\n";
        if (config->theme)
            std::cout << "Theme: " << *config->theme << "\n";
    }

    optionalDemo.accessData();
    optionalDemo.withSTL();
}

int main() {
    optionalTest();
    return 0;
}
