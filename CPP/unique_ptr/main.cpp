#include <iostream>

using namespace std;

extern void create_prt();

extern void get_shared_ptr();

extern void test_unique_ptr();

extern void test_share_cycle_reference();

extern void test_share_cycle_reference_2();
extern void test_weak_ptr_lock();

int main() {
    // share_prtr;
    cout << "-------------------start---------------------------------" << endl;
    create_prt();
    get_shared_ptr();
    cout << "-------------------end---------------------------------" << endl;

    cout << "-------------------start---------------------------------" << endl;
    test_unique_ptr();
    cout << "-------------------end---------------------------------" << endl;

    cout << "-------------------start---------------------------------" << endl;
    test_share_cycle_reference();
    test_share_cycle_reference_2();
    test_weak_ptr_lock();
    cout << "-------------------end---------------------------------" << endl;



    return 0;
}
