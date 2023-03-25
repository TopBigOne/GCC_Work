#include <stdio.h>

#include <libavutil/avutil.h>
#include <libavutil/avstring.h>

int main() {
    int        ret      = 0;
    char       *src_str = av_strdup("FFmpeg-Principle");
    char       *dst_str = av_malloc(1024);
    const char *ptr     = NULL;
    ret = av_strstart(src_str, "我", &ptr);
    printf("ret is :  %d ,ptr is : %s \n", ret, ptr);


    // case 2:
    ret = av_stristart(src_str, "w", &ptr);
    printf("ret is :  %d ,ptr is : %s \n", ret, ptr);

    // case 3: 跟C标准库函数 strstr() 一样，只是限制了搜索长度
    ptr = av_strnstr(src_str, "Princ", 20);
    printf("ptr is %s \n", ptr);

    // case 4: 根据长度拷贝字符，跟 BSD 的 strlcpy() 函数一样。
    ret = av_strlcpy(dst_str, src_str, 9);
    printf("ret is %d, dst_str is %s \n", ret, dst_str);

    // case 5: 拼接字符串函数，跟 BSD的 strlcat() 类似，但有些许区别。
    ret = av_strlcat(dst_str, src_str, 100);
    printf("ret is %d , dst_str is %s \n", ret, dst_str);

    // case 6:动态参数拼接字符串，感觉这个函数超有用
    ret = av_strlcatf(dst_str, 512, " is %d ", 666);
    printf("ret is %d , dst_str is %s \n", ret, dst_str);

    // case 7:获取字符串的长度，但是加了长度限制，估计是为了防止传错指针，字符串没有以 0 结尾，导致死循环。加了长度限制就不会死循环。
    ret = av_strnlen(dst_str, 1024);
    printf("ret is %d, dst_str is %s \n", ret, dst_str);

    // case 8: 类似 GNU 的 asprintf() ，返回值是堆指针，需要自己释放内存
    const char *dst_str_2 = av_asprintf("FFmpeg-Pinciple %s", "a new book");
    printf("dst_str_2 is %s \n", dst_str_2);

    // case 9: 不知道干嘛的，后面补充。
    const char *p2 = av_get_token(&dst_str_2, "-");
    printf("p2 is %s \n", p2);

    // case 10 : av_strtok : 字符串分割函数，可以把 hello-jason-please 字符串按照 - 符号分割成 hello jason please。
    char *token_str;
    char *saveptr = NULL;
    while ((token_str = av_strtok(src_str, "-", &saveptr))) {
        // 第二次调用的时候，src 必须为NULL；
        src_str = NULL;
        printf("ret is %d ,token_str is %s \n", ret, token_str);
    }
    // case 11: av_isdigit: 判断单个字符是不是数字字符，通过把 char 转成 int 来比较的。
    char digit_1 = '2';
    ret = av_isdigit(digit_1);
    printf("ret is %d \n", ret);
    char digit_2 = 'r';
    ret = av_isdigit(digit_2);
    printf("ret is %d \n", ret);

    // case 12 清空一下dst_str
    memset(dst_str, 0, 1024);

    av_freep(&src_str);
    av_freep(&dst_str);
    av_freep(&dst_str_2);
    return 0;
}
