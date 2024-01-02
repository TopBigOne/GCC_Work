#include <iostream>

#include <pthread.h>
#include <unistd.h>

void test_get_pointer_value();

void test_pthread_cancel();

using namespace std;

void *mySleep(void *arg) {
    int seconds = *(static_cast<int * >(arg));
    std::cout << "  Thread started. Sleeping for " << seconds << " seconds." << std::endl;
    sleep(seconds);

    std::cout << "  Thread finished." << std::endl;
    return nullptr;
}


void test_pthread_cancel() {
    puts("test_pthread_cancel ：");
    pthread_t thread;
    int       sleeps = 5;
    pthread_create(&thread, nullptr, mySleep, &sleeps);

    sleep(3);
    int result = pthread_cancel(thread);
    if (result == 0) {
        std::cout << "  Thread canceled." << std::endl;
    } else {
        std::cerr << "  Failed to cancel thread. Error code: " << result << std::endl;
    }
    pthread_join(thread, nullptr);

}


void test_get_pointer_value() {
    puts("test_get_pointer_value ：");
    int *aa = new int;
    int *bb = new int;
    *aa = 30;
    *bb = 40;

    cout << "   aa value   : " << *aa << endl;
    aa = bb;
    cout << "   aa value   : " << *aa << endl;
    int **cc = &aa;
    cout << "   *cc value  : " << *cc << endl;
    cout << "   **cc value : " << **cc << endl;
}


int main() {


    test_get_pointer_value();

    test_pthread_cancel();


    std::cout << "  Main thread finished." << std::endl;

    return 0;
}
