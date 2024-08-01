#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

void test_one();

pthread_mutex_t mutex;
pthread_cond_t  cond;
int             ready = 0;

void *thread_function() {
    printf("%s\n", __FUNCTION__);
    pthread_mutex_lock(&mutex);
    while (!ready) {
        printf("    wait...\n");
        pthread_cond_wait(&cond, &mutex);
    }
    printf("    Thread is notified.\n");
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}


void test_one() {
    pthread_t thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_create(&thread, NULL, thread_function, NULL);

    printf("start work for sth.\n");
    // 模拟一些耗时工作
    sleep(2);
    pthread_mutex_lock(&mutex);
    ready = 1;
    // 通知所有等待线程
    printf("    start notify.\n");
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(thread, NULL);
    pthread_cond_destroy(&cond);

}

int main() {
    int allMoney = 20000 + 1500 + 750 + 1100 + 2200 + 250 + 2300 + 6000 + 1000 + 1000 + 1000;
    printf("allMoney : %d\n", allMoney);
    test_one();
    return 0;
}
