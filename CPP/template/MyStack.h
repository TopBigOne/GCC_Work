//
// Created by dev on 2023/9/15.
//

#ifndef TEMPLATE_MYSTACK_H
#define TEMPLATE_MYSTACK_H


template<typename T>
class MyStack {

public:
    MyStack() {
        elements = new T[100];
        size     = 0;
    }

    void push(T item) {
        elements[size++] = item;
    }

    T pop() {
        return elements[--size];
    }


private:
    T   *elements;
    int size;


};


#endif //TEMPLATE_MYSTACK_H
