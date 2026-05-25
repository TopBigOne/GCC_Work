#include <memory>
#include <vector>

#include "demo/demo.h"
#include "demo/demo_aggregator.h"
#include "demo/demo_basic.h"
#include "demo/demo_chain.h"
#include "demo/demo_thread.h"

int main() {
    std::vector<std::unique_ptr<Demo>> demos;
   // demos.emplace_back(std::make_unique<DemoBasic>());
  //  demos.emplace_back(std::make_unique<DemoThread>());
   // demos.emplace_back(std::make_unique<DemoAggregator>());
    demos.emplace_back(std::make_unique<DemoChain>());

    for (const auto & demo : demos) {
        demo->run();
    }

    return 0;
}
