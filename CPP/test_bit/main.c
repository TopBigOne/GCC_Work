#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct packet {
    uint8_t *m_body;

} packet;


void testOne(void);

int getFullLength(const packet *pkt);

void testOne(void) {
    packet *pkt = malloc(sizeof(packet));
    pkt->m_body = malloc(sizeof(uint8_t) * 4);
    int i = 0;
    //  300 = 0x12C = 0001 0010 1100
    int spsLength = 300;

    uint8_t v_300_high = (spsLength >> 8) & 0xFF;
    uint8_t v_300_low = (spsLength) & 0xFF;
    printf("t--v_300_high %d\n", v_300_high);
    printf("t--v_300_low %d\n", v_300_low);

    // 把16位的300 拆成2个8位，放在m_body
    pkt->m_body[i++] = v_300_high;
    pkt->m_body[i++] = v_300_low;

    printf("t--0 %d\n", pkt->m_body[0]);
    printf("t--1 %d\n", pkt->m_body[1]);
    int fullLength = getFullLength(pkt);
    printf("fullLength %d\n", fullLength);

    free(pkt->m_body);
    free(pkt);
}

int getFullLength(const packet *pkt) {
    // 把pkt中的m_body的[0][1]组合起来
    return (pkt->m_body[0] << 8) | pkt->m_body[1];
}

int main(void) {

    testOne();

    return 0;
}
