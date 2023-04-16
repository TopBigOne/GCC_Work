### read the ffplay.c 代码的时候，遇到了下面 诡异的代码
```c

static Frame *frame_queue_peek(FrameQueue *f) {
    // todo 这个模运算是什么意思？ 是TM怕 数组越界？
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}
```
* 我什么脾气，看不懂，，我就撸，，，码。。。
* 测试一下 用一个数组（array）和一个下标（index） ,在使用index的时候，先和数组大小size做一个% 运算，有什么效果
* 形如
```c
array[index%size]
```

### 我写了测试代码
### 结论
* index%size 以后，
  * 当index是一个正常没有越界的index，那就正常使用
  * 当index，是一个越界的值， index%size 的index 哈哈哈，，那就不好说了，反正不会 more than the array size,**所以，不会出现 下标越界异常**
  * 



