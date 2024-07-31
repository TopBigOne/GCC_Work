
# sigaction函数
```C
#include <signal.h>

int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact);
```
* 


# raise 函数
```c
#include <signal.h>
int raise(int sig);
```
* 在 Linux 中，raise() 函数用于向当前进程发送一个信号
* raise() 函数通过向当前进程发送一个指定的信号来触发信号处理函数的执行。
  * sig 参数指定了要发送的信号的编号，可以是任意有效的信号编号，如 SIGTERM、SIGINT 等。
* 当调用 raise() 函数时，系统会向当前进程发送指定的信号，
  * 如果当前进程已经注册了对应信号的处理函数，那么该处理函数将会被执行。
  * 如果当前进程没有注册对应信号的处理函数，系统会根据默认的信号处理方式来处理这个信号，
  * 例如终止进程或者忽略信号等。

# 参考
* [Linux 系统调用 sigaction](https://feng-qi.github.io/2017/05/09/linux-system-call-sigaction/)

