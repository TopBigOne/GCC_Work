#include <iostream>

using namespace std;

int *GetHeapPointer(int *p) {
    p      = new int;
    int k1 = 111;
    *p = k1;
    return p;
}
int *GetStackPointer(int *i){
    int k2 = 222;
    i= &k2;
    // 没有动态分配
    return i;
}

int main() {
    int *p =  NULL;
    p = GetHeapPointer(p);
    cout<<"堆指针*p="<<*p<<" 地址为："<<p<<endl;
    int *i = NULL;
    i = GetStackPointer(i);
    cout<<"栈指针*i="<<*i<<" 地址为："<<i<<endl;
    delete p;
    p = NULL;

    cout<<"内存销毁后堆指针 *p="<<*p<<" 地址为："<<p<<endl;

    return 0;
}
