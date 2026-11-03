#ifndef FILEIOC_H
#define FILEIOC_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
uint8_t ti_Open(const char *, const char *);
size_t ti_Read(void *, size_t, size_t, uint8_t);
size_t ti_Write(const void *, size_t, size_t, uint8_t);
int ti_Close(uint8_t);
bool ti_ArchiveHasRoom(uint32_t);
int ti_SetArchiveStatus(bool, uint8_t);
void ti_SetGCBehavior(void (*)(void), void (*)(void));
#endif
