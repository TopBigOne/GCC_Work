#include <iostream>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;

void handler(int signum, siginfo_t *siginfo, void *ucontext);

void init_sigaction(struct sigaction *act);

void handler(int signum, siginfo_t *siginfo, void *ucontext) {
    cout << __FUNCTION__ << endl;
    printf("handler called\n");
    printf("signum(SIGQUIT) is: %d\n", signum);
    printf("information in siginfo is:\n");
    printf("\tsi_signo:  %d\n", siginfo->si_signo);
    printf("\tsi_errno:  %d\n", siginfo->si_errno);
    printf("\tsi_code:   %d\n", siginfo->si_code);
    /* printf("\tsi_trapno: %d\n", siginfo->si_trapno); */
    printf("\tsi_pid:    %d\n", siginfo->si_pid);
    printf("\tsi_uid:    %d\n", siginfo->si_uid);
    printf("\tsi_status: %d\n", siginfo->si_status);
   // printf("\tsi_utime:  %ld\n", siginfo->si_utime);//
   // printf("\tsi_stime:  %ld\n", siginfo->si_stime);//
    printf("\tsi_value:  %d\n", siginfo->si_value );
    printf("handler exiting...\n");


}

void init_sigaction(struct sigaction *act) {
    cout << __FUNCTION__ << endl;
    act->__sigaction_u.__sa_sigaction = handler;
    sigemptyset(&act->sa_mask);
    act->sa_flags = SA_SIGINFO;
}

void testOne();

void testOne() {
    cout << __FUNCTION__ << endl;
    struct sigaction act;
    init_sigaction(&act);
    cout << "pid is : " << getpid() << endl;
    if (sigaction(SIGQUIT, &act, nullptr) == -1) {
        cerr << "sigaction error" << endl;
        exit(1);
    }
    raise(SIGQUIT);
    cout << "exiting..." << endl;
}


void ouch(int sig)
{
    printf("\nOUCH! - I got signal %d\n", sig);

    // 恢复终端中断信号SIGINT的默认行为
    (void) signal(SIGINT, SIG_DFL);
}

void testTwo();
void testTwo(){
    struct sigaction act;
    act.sa_handler = ouch;

    // 创建空的信号屏蔽字，即不屏蔽任何信息
    sigemptyset(&act.sa_mask);

    // 使sigaction函数重置为默认行为
    act.sa_flags = SA_RESETHAND;

    sigaction(SIGINT, &act, 0);

    while(1)
    {
        printf("Hello World!\n");
        sleep(1);
    }

}


int main() {
    // testOne();
    testTwo();
    return 0;
}
