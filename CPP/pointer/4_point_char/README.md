### 我的疑问

```c
typedef struct RTMPPacket
{
    char *m_body;
} RTMPPacket;

auto *packet = new RTMPPacket;
packet->m_body[0] = 0xAF;
packet->m_body[1] = 0x01;



```

* m_body 明明是一个指针，为什么当成 数组来使用，用，0 ，1 来进行赋值操作？