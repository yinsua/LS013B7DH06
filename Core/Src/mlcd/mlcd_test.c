#include <stdlib.h>
#include <memory.h>
#include "fonts.h"
#include "mlcd_test.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;       //SPI interface for display

MLCD mlcd;

void mlcd_Init(void) {
  mlcd_Clear();
  HAL_Delay(1000);
  HAL_GPIO_WritePin(DISP_DISPLAY_GPIO_Port, DISP_DISPLAY_Pin, GPIO_PIN_SET);
  mlcd.CurrentX = 0;
  mlcd.CurrentY = 0;
  mlcd.Inverted = FALSE;
  mlcd.Initialized = TRUE;

  mlcd.BufferSize = LCD_WIDTH * LCD_HEIGHT * 3 / 8;
  mlcd.Buffer = (uint8_t *) malloc(mlcd.BufferSize);
}

void mlcd_Clear(void) {
  uint8_t clear_data[2] = {BIT_CLEAR, 0x00};

  HAL_GPIO_WritePin(CS_DISPLAY_GPIO_Port, CS_DISPLAY_Pin, GPIO_PIN_SET);
  HAL_SPI_Transmit(&hspi1, &clear_data[0], 1, HAL_MAX_DELAY);
  HAL_SPI_Transmit(&hspi1, &clear_data[1], 1, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(CS_DISPLAY_GPIO_Port, CS_DISPLAY_Pin, GPIO_PIN_RESET);

  mlcd.CurrentX = 0;
  mlcd.CurrentY = 0;

  memset(mlcd.Buffer, 0xFF, mlcd.BufferSize);
//  mlcd_Test();
}

void mlcd_DrawPixel(uint8_t x, uint8_t y, LCD_COLOR color) {
  if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
    return;
  }
  // x : 0~7 is a group, 8~15 is a group, etc...
  // a group fill 3 bytes: [R0 G0 B0 R1 G1 B1 R2 G2][B2 R3 G3 B3 R4 G4 B4 R5][G5 B5 R6 G6 B6 R7 G7 B7]
  // so a line have 0~15 group (128*3/8 = 48 bytes a line, 48 / 3 = 16)
  uint16_t y_offset = (LCD_WIDTH * 3 / 8) * y;
  uint8_t group = x / 8, group_offset = x - 8 * group;

//  if (group_offset < 0 || group_offset > 7) return;

  switch (group_offset) {
    case 0:mlcd.Buffer[y_offset + group * 3] &= 0b11111000;
      mlcd.Buffer[y_offset + group * 3] |= (color << 0);
      break;
    case 1:mlcd.Buffer[y_offset + group * 3] &= 0b11000111;
      mlcd.Buffer[y_offset + group * 3] |= (color << 3);
      break;
    case 2:mlcd.Buffer[y_offset + group * 3] &= 0b00111111;
      mlcd.Buffer[y_offset + group * 3 + 1] &= 0b11111110;
      mlcd.Buffer[y_offset + group * 3] |= (color << 6);
      mlcd.Buffer[y_offset + group * 3 + 1] |= (color >> 2);
      break;
    case 3:mlcd.Buffer[y_offset + group * 3 + 1] &= 0b11110001;
      mlcd.Buffer[y_offset + group * 3 + 1] |= (color << 1);
      break;
    case 4:mlcd.Buffer[y_offset + group * 3 + 1] &= 0b10001111;
      mlcd.Buffer[y_offset + group * 3 + 1] |= (color << 4);
      break;
    case 5:mlcd.Buffer[y_offset + group * 3 + 1] &= 0b01111111;
      mlcd.Buffer[y_offset + group * 3 + 2] &= 0b11111100;
      mlcd.Buffer[y_offset + group * 3 + 1] |= (color << 7);
      mlcd.Buffer[y_offset + group * 3 + 2] |= (color >> 1);
      break;
    case 6:mlcd.Buffer[y_offset + group * 3 + 2] &= 0b11100011;
      mlcd.Buffer[y_offset + group * 3 + 2] |= (color << 2);
      break;
    case 7:mlcd.Buffer[y_offset + group * 3 + 2] &= 0b00011111;
      mlcd.Buffer[y_offset + group * 3 + 2] |= (color << 5);
      break;
    default:break;
  }
}

char mlcd_WriteChar(char ch, FontType Font, LCD_COLOR color) {
  uint32_t i, b, j;
  if (ch < 32 || ch > 126)
    return 0;
//  if (LCD_WIDTH < (mlcd.CurrentX + Font.FontWidth)) {
//    if (LCD_HEIGHT > (mlcd.CurrentY + 2 * Font.FontHeight - 2)) {        // -2 => Margin of the character
//      mlcd.CurrentX = 0;
//      mlcd.CurrentY = mlcd.CurrentY + Font.FontHeight - 1;        //-1 => Margin of the character fort
//    } else {
//      return 0;
//    }
//  }

  for (i = 0; i < Font.FontHeight; i++) {
    b = Font.data[(ch - 32) * Font.FontHeight + i];
    for (j = 0; j < Font.FontWidth; j++) {
      if ((b << j) & 0x8000) {
        mlcd_DrawPixel(mlcd.CurrentX + j, (mlcd.CurrentY + i), color);
      }
//      else {
//        mlcd_DrawPixel(mlcd.CurrentX + j, (mlcd.CurrentY + i), White);
//      }
    }
  }
  mlcd.CurrentX += Font.FontWidth;
  return ch;
}

void mlcd_Refresh(void) {
  uint8_t cmd_data = BIT_WRITECMD;
  uint8_t adr_data = 0x01;
  uint8_t dmy_data = 0x00;

  HAL_GPIO_WritePin(CS_DISPLAY_GPIO_Port, CS_DISPLAY_Pin, GPIO_PIN_SET);

  HAL_SPI_Transmit(&hspi1, &cmd_data, 1, HAL_MAX_DELAY);

  for (uint16_t s_add = 1; s_add <= LCD_HEIGHT; s_add++) {
    adr_data = s_add;
    HAL_SPI_Transmit(&hspi1, &adr_data, 1, HAL_MAX_DELAY);
    uint16_t step = LCD_WIDTH * 3 / 8;
    for (uint16_t s_data = 0; s_data < LCD_WIDTH * 3 / 8; s_data++) {
      uint16_t index = s_data + (s_add - 1) * step;
      HAL_SPI_Transmit(&hspi1, &mlcd.Buffer[index], 1, HAL_MAX_DELAY);
    }
    HAL_SPI_Transmit(&hspi1, &dmy_data, 1, HAL_MAX_DELAY);
  }

  HAL_SPI_Transmit(&hspi1, &dmy_data, 1, HAL_MAX_DELAY);

  HAL_Delay(1);
  HAL_GPIO_WritePin(CS_DISPLAY_GPIO_Port, CS_DISPLAY_Pin, GPIO_PIN_RESET);
}

void mlcd_Test() {
#if 0
  uint8_t b1 = 0b10001000, b2 = 0b11000100, b3 = 0b01000000;
  for (uint16_t i = 0; i < LCD_HEIGHT; i++) {
    for (uint16_t j = 0; j < LCD_WIDTH * 3 / 8; j += 3) {
      uint16_t index = i * LCD_WIDTH * 3 / 8 + j;
      mlcd.Buffer[index] = b1;
      mlcd.Buffer[index + 1] = b2;
      mlcd.Buffer[index + 2] = b3;
    }
  }
#else
  uint8_t red[3] = {0b01001001, 0b10010010, 0b00100100};//R
  uint8_t green[3] = {0b10010010, 0b00100100, 0b01001001};//G
  uint8_t blue[3] = {0b00100100, 0b01001001, 0b10010010};//B
  for (int i = 0; i < mlcd.BufferSize; i += 3) {
    mlcd.Buffer[i] = green[0];
    mlcd.Buffer[i + 1] = green[1];
    mlcd.Buffer[i + 2] = green[2];
  }
#endif
}