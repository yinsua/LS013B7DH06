#include <stdint.h>

#ifndef FONTS_H_
#define FONTS_H_

typedef struct {
  const uint8_t FontWidth;
  const uint8_t FontHeight;
  const uint16_t *data;
} FontType;

extern FontType Font_6x8;
extern FontType Font_7x10;
extern FontType Font_11x18;
extern FontType Font_16x26;

#endif