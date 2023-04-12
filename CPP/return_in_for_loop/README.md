### 双层 for 循环中，break,continue,return 的区别

* continue

> 在双层FOR循环中，在内循环使用Continue,只会在内循环中跳过此处循环，小循环继续执行，不会跳过大循环。

* break

> 在双层FOR循环中，在内循环使用Break,只会在内循环中结束本次循环，小循环结束此次执行，不会跳过大循环。在双层FOR循环中，在内循环使用Return，结束本次循环，同时结束本次方法 。

* return

> 在双层FOR循环中，在内循环使用Return，结束本次循环，同时结束本次方法 。

### 我所疑惑的事 return
> 在 for 循环中，我貌似没主动用过 **return**

```c
void test_double_for() {
    puts("test_double_for : start.");
    for (int i = 0; i < 4; i++) {
        printf("i==> %d \n", i);

        for (int j = 0; j < 3; j++) {
            if (i == 2) {
                puts(" 在第二层for中，直接结束整个函数.");
                return;
            }
            printf(" j==> %d \n", j);
        }
        printf("\n");
    }

    // NOTE: 下面这一行代码，是不会走的.
    puts("test_double_for : end.");
}

```

### 参考：

* https://blog.csdn.net/a4171175/article/details/79980333