#include <iostream>


#include "TestArray.h"

int main() {
    /**
 * case 1:创建
 */
    testBase();

    /**
     * case 2:使用迭代器遍历
     */
    testIteration();

    /**
     * case 3: 使用stl 排序
     */
    testSTLSort();


    /**
     * case 4: 边界检查，
     */
    testAt();


    /**
     * case 5: 整体赋值和copy，
     */
    testCopy();


    /**
     * case 6: 用作 STL 容器的 key
     */
    testKey();


    /**
     * case 7: 填充数组：fill 方法
     */
    testFill();


    return 0;
}
