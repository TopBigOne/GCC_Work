#include <iostream>
#include "VariantDemo.h"

void testVariantDemo();

void testVariantDemo() {
    VariantDemo variantDemo;
    variantDemo.baseUsage();
    variantDemo.typeChecking();
    variantDemo.withVisit();
    variantDemo.withLambda();
}


int main() {
  std::cout << "|-------------- test variant---------start✅" << std::endl;
    testVariantDemo();
  std::cout << "|-------------- test variant---------end  ✅" << std::endl;
    return 0;
}
