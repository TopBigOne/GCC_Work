### [AVString字符串函数库详解](https://ffmpeg.xianwaizhiyin.net/api-ffmpeg/avstring.html)
* int **av_strstart**(const char *str, const char *pfx, const char **ptr);
  * 检查 str 字符串是不是以 pfx 开头的，如果是，返回 非 0，同时把 ptr 指向 pfx 后面的字符