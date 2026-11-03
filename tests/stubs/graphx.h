#ifndef GRAPHX_H
#define GRAPHX_H
#include <stdint.h>
typedef enum { gfx_text_clip = 1, gfx_text_noclip } gfx_text_options_t;
typedef enum { gfx_8bpp = 0x27 } gfx_mode_t;
void gfx_Begin(void); void gfx_End(void); void gfx_SetDrawScreen(void);
void gfx_SetPalette(const void *, uint32_t, uint8_t); void gfx_SetTextConfig(uint8_t);
void gfx_SetTextTransparentColor(uint8_t); void gfx_SetTextScale(uint8_t,uint8_t);
void gfx_SetTextFGColor(uint8_t); void gfx_PrintStringXY(const char *,int,int);
void gfx_SetTextXY(int,int); void gfx_PrintUInt(unsigned int,uint8_t);
void gfx_SetColor(uint8_t); void gfx_FillScreen(uint8_t); void gfx_FillRectangle(int,int,int,int);
void gfx_Rectangle(int,int,int,int); void gfx_HorizLine(int,int,int); void gfx_VertLine(int,int,int);
void gfx_Line(int,int,int,int); void gfx_FillCircle(int,int,uint32_t);
#endif
