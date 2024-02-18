#include <iostream>

void test();

void cleanup_function(void *arg) {
    printf("Cleaning up thread-specific data...\n");


}

pthread_key_t key;

int *realValue = nullptr;

void test();

void *thread_function(void *arg) {
    pthread_setspecific(key, arg);
    pthread_cleanup_push(cleanup_function, nullptr)

        realValue = reinterpret_cast<int *>(pthread_getspecific(key));
        printf("case 1 : Thread-special data : %d\n", *realValue);

        // 这里可以执行线程的其他操作
        // 线程退出，调用 cleanup_function
    pthread_cleanup_pop(1);
    realValue = reinterpret_cast<int *>(pthread_getspecific(key));
    printf("case 2 : Thread-special data : %d\n", *realValue);
    pthread_key_delete(key);
    realValue = reinterpret_cast<int *>(pthread_getspecific(key));
    if(realValue== nullptr){
        printf("case 3 : Thread-special data  has been deleted.\n");

    }else{
        printf("case 3 : Thread-special data : %d\n", *realValue);
    }


    return NULL;
}


void test() {
    // 创建线程特定数据键
    if (pthread_key_create(&key, NULL) != 0) {
        fprintf(stderr, "ERROR");
        return;
    }
    pthread_t thread;
    int       data   = 42;
    int       result = pthread_create(&thread, NULL, thread_function, &data);
    if (result != 0) {
        fprintf(stderr, "Create pthread ERROR.");
        return;
    }

    pthread_join(thread, nullptr);
    pthread_key_delete(key);
}


int main() {
    test();
    return 0;
}
