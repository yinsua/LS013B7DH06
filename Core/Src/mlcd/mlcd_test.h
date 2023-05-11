
#ifndef MLCD_H_
#define MLCD_H_

#include <stdint.h>
#include "fonts.h"
#include "main.h"

/// MUST USE SPI LSB

#define DISP_DISPLAY_GPIO_Port MLCD_DISP_GPIO_Port
#define DISP_DISPLAY_Pin MLCD_DISP_Pin

#define CS_DISPLAY_GPIO_Port MLCD_CS_GPIO_Port
#define CS_DISPLAY_Pin MLCD_CS_Pin

#define TRUE 1
#define FALSE 0

#define LCD_HEIGHT  128
#define LCD_WIDTH   128

#define BIT_WRITECMD (0x01)
#define BIT_VCOM      (0x02)
#define BIT_CLEAR    (0x04)

typedef enum {
  Black = 0b000,
  Red = 0b001,
  Green = 0b010,
  Blue = 0b100,
  White = 0b111
} LCD_COLOR;

typedef struct {
  uint16_t CurrentX;
  uint16_t CurrentY;
  uint8_t Inverted;
  uint8_t Initialized;
  uint32_t BufferSize;
  uint8_t *Buffer;
} MLCD;

void mlcd_Init(void);
void mlcd_Clear(void);
void mlcd_Refresh(void);
void mlcd_DrawPixel(uint8_t x, uint8_t y, LCD_COLOR color);
char mlcd_WriteChar(char ch, FontType Font, LCD_COLOR color);
void mlcd_Test();
#endif