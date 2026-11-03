#ifndef KEYPADC_H
#define KEYPADC_H
#include <stdint.h>
#define kb_Alpha (1u<<7)
#define kb_2nd (1u<<5)
#define kb_Left (1u<<1)
#define kb_Right (1u<<2)
#define kb_Down (1u<<0)
#define kb_Up (1u<<3)
#define kb_Enter (1u<<0)
#define kb_Clear (1u<<6)
extern volatile uint8_t kb_Data[8];
void kb_Scan(void); void kb_DisableOnLatch(void); void kb_Reset(void);
#define kb_On 0
#endif
