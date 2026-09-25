/*
 *  LCDBigNumbers.hpp
 *
 *  Arduino library to write big numbers on a 1602 or 2004 LCD.
 *
 *  Copyright (C) 2022-2026  Armin Joachimsmeyer
 *
 *  This file is part of LCDBigNumbers https://github.com/ArminJo/LCDBigNumbers.
 *
 *  LCDBigNumbers is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

// Extracted two-row font tables; original upstream file and license in third_party.
#pragma once
#include <stdint.h>
#include <string.h>
#ifdef ARDUINO
#include <Arduino.h>
#else
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#endif
// http://www.picbasic.co.uk/forum/showthread.php?t=13376
// 8 custom characters for 1 column font
const uint8_t bigNumbers1x2CustomPatterns_1[][8] PROGMEM = {
 { 0b11110, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b11110 }, // 0 Closed rectangle - used for: 8, 9
 { 0b11110, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010 }, // 1 Rectangle - open at bottom - 0
 { 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b11110 }, // 2 Rectangle - open at top - 0, 4, 6, 8
 { 0b11110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b11110 }, // 3 Rectangle - open at left - 3
 { 0b11110, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11110 }, // 4 Rectangle - open at right - 2, 5, 6
 { 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010 }, // 5 Right bar - 1, 4, 7
 { 0b11110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010 }, // 6 Top right - 2, 7
 { 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b11110 }  // 7 Right bottom - 3,5,9
};
const uint8_t bigNumbers1x2_1[2][13] PROGMEM = {                   // 2-line numbers
//    "-"   "."   ":"    0     1     2     3     4     5     6     7     8     9
    { 0x5F, 0xFE, 0xA5, 0x01, 0x05, 0x06, 0x03, 0x02, 0x04, 0x04, 0x06, 0x00, 0x00 },
    { 0xFE, 0x2E, 0xA5, 0x02, 0x05, 0x04, 0x07, 0x05, 0x07, 0x02, 0x05, 0x02, 0x07 }
};

// https://www.alpenglowindustries.com/blog/the-big-numbers-go-marching-2x2#/
// https://github.com/AlpenglowIndustries/Alpenglow_BigNums2x2
// 8 custom characters for Trek font
// Requires 1 0xFF block for the special "0"
const uint8_t bigNumbers2x2CustomPatterns_1[][8] PROGMEM = {
 { 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000 }, // 0
 { 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000 }, // 1
 { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111 }, // 2
 { 0b11111, 0b11111, 0b00011, 0b00011, 0b00011, 0b00011, 0b11111, 0b11111 }, // 3
 { 0b11111, 0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111, 0b11111 }, // 4
 { 0b11111, 0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000 }, // 5
 { 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b11111, 0b11111 }, // 6
 { 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111, 0b11111 } // 7
};
const uint8_t bigNumbers2x2_1[2][23] PROGMEM = {                   // 2-line numbers
//    "-"   "."   ":"       0          1          2          3          4          5          6          7          8          9
    { 0xFE, 0xFE, 0xA5, 0x05,0xFF, 0x00,0x01, 0x00,0x03, 0x00,0x03, 0x01,0x01, 0x04,0x00, 0x05,0x00, 0x00,0x03, 0x04,0x03, 0x04,0x03},
    { 0x00, 0x2E, 0xA5, 0x07,0x06, 0x02,0x07, 0x04,0x02, 0x02,0x03, 0x00,0x05, 0x02,0x03, 0x04,0x03, 0xFE,0x01, 0x04,0x03, 0x02,0x06}
};


// 3x2 https://liudr.wordpress.com/2011/03/21/big-font/
// 3x2 http://www.netzmafia.de/skripten/hardware/Arduino/LCD/index.html
// Requires 0xFF blocks, but character 6 could be used for it
const uint8_t bigNumbers3x2CustomPatterns_1[6][8] PROGMEM = {
  { 0b11111, 0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 }, // 0 Upper bar
  { 0b00000, 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 }, // 1 Lower bar
  { 0b11111, 0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 }, // 2 Upper and lower bar
  { 0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 ,0b00000 ,0b00000 ,0b00000 }, // 3 Minus sign
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 }, // 4 Decimal point
  { 0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 ,0b00000 ,0b00000 ,0b00000 }  // 5 Colon
};
const uint8_t bigNumbers3x2_1[2][33] PROGMEM = {               // 2-line numbers
//    "-"   "."   ":"         0               1               2               3               4               5               6               7               8               9
    { 0x01, 0xFE, 0x05, 0xFF,0x00,0xFF, 0x00,0xFF,0xFE, 0x02,0x02,0xFF, 0x00,0x02,0xFF, 0xFF,0x01,0xFF, 0xFF,0x02,0x02, 0xFF,0x02,0x02, 0x00,0x00,0xFF, 0xFF,0x02,0xFF, 0xFF,0x02,0xFF},
    { 0xFE, 0x04, 0x05, 0xFF,0x01,0xFF, 0x01,0xFF,0x01, 0xFF,0x01,0x01, 0x01,0x01,0xFF, 0xFE,0xFE,0xFF, 0x01,0x01,0xFF, 0xFF,0x01,0xFF, 0xFE,0xFE,0xFF, 0xFF,0x01,0xFF, 0x01,0x01,0xFF}
};

// 3x2 https://forum.arduino.cc/t/display-3-character-wide-big-digits-on-16x2-lcd/905360 bottom of page
// Requires 0xFF blocks
const uint8_t bigNumbers3x2CustomPatterns_2[8][8] PROGMEM = {
  { 0b11111 ,0b11111 ,0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 }, // 0 Upper bar
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 ,0b11111 }, // 1 Lower bar
  { 0b11111 ,0b11111 ,0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 }, // 2 Upper and lower bar for 5,6
  { 0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 }, // 3 Left bar
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11100 ,0b11100 ,0b11100 }, // 4 Left lower bar for 2
  { 0b11100 ,0b11100 ,0b11100 ,0b00000 ,0b00000 ,0b00000 ,0b11100 ,0b11100 }, // 5 Left upper and lower bar for 5,6
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 }, // 6 Decimal point
  { 0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 ,0b00000 ,0b00000 ,0b00000 }  // 7 Colon
};
const uint8_t bigNumbers3x2_2[2][33] PROGMEM = {               // 2-line numbers
//    "-"   "."   ":"         0               1               2               3               4               5               6               7               8               9
    { 0x01, 0xFE, 0x07, 0xFF,0x00,0x03, 0x00,0x03,0xFE, 0x02,0x02,0x03, 0x02,0x02,0x03, 0xFF,0x01,0x03, 0xFF,0x02,0x05, 0xFF,0x02,0x05, 0x00,0x00,0x03, 0xFF,0x02,0x03, 0xFF,0x02,0x03},
    { 0xFE, 0x06, 0x07, 0xFF,0x01,0x03, 0xFE,0x03,0xFE, 0xFF,0x01,0x04, 0x01,0x01,0x03, 0xFE,0xFE,0x03, 0x01,0x01,0x03, 0xFF,0x01,0x03, 0xFE,0xFE,0x03, 0xFF,0x01,0x03, 0xFE,0xFE,0x03}
};

//3x2 https://exploreembedded.com/wiki/Distance_Meter_with_Big_Fonts
const uint8_t bigNumbers3x2CustomPatterns_3[8][8] PROGMEM = {
{ 0b11100, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11100}, // 0 left bar
{ 0b00111, 0b01111, 0b01111, 0b01111, 0b01111, 0b01111, 0b01111, 0b00111}, // 1 right bar
{ 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // 2 upper and lower bar
{ 0b11110, 0b11100, 0b00000, 0b00000, 0b00000, 0b00000, 0b11000, 0b11100}, // 3 left upper and lower rounded
{ 0b01111, 0b00111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00011, 0b00111}, // 4 right upper and lower rounded
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // 5 right lower
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00111, 0b01111}, // 6 right lower rounded
{ 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}  // 7 upper bar
};
const uint8_t bigNumbers3x2_3[2][33] PROGMEM = {                   // 2-line numbers
//    "-"   "."   ":"         0               1               2               3               4               5               6               7               8               9
    { 0xFE, 0xFE, 0xA5, 0x01,0x07,0x00, 0xFE,0x00,0xFE, 0x04,0x02,0x00, 0x04,0x02,0x00, 0x01,0x05,0x00, 0x01,0x02,0x03, 0x01,0x02,0x03, 0x01,0x07,0x00, 0x01,0x02,0x00, 0x01,0x02,0x00},
    { 0x07, 0x06, 0xA5, 0x01,0x05,0x00, 0xFE,0x00,0xFE, 0x01,0x05,0x05, 0x06,0x05,0x00, 0xFE,0xFE,0x00, 0x06,0x05,0x00, 0x01,0x05,0x00, 0xFE,0xFE,0x00, 0x01,0x05,0x00, 0x06,0x05,0x00}
};


struct ExtraFont {
  const uint8_t (*patterns)[8];
  const uint8_t* digits;
  uint8_t width, patternCount;
};
inline ExtraFont extraFont(int index) {
  switch(index) {
    case 0: return {bigNumbers1x2CustomPatterns_1, &bigNumbers1x2_1[0][0], 1, 8};
    case 1: return {bigNumbers2x2CustomPatterns_1, &bigNumbers2x2_1[0][0], 2, 8};
    case 2: return {bigNumbers3x2CustomPatterns_1, &bigNumbers3x2_1[0][0], 3, 6};
    case 3: return {bigNumbers3x2CustomPatterns_3, &bigNumbers3x2_3[0][0], 3, 8};
    default: return {bigNumbers3x2CustomPatterns_2, &bigNumbers3x2_2[0][0], 3, 8};
  }
}
inline bool colonVisible(int mode, int second) {
  return mode == 1 || (mode == 0 && second % 2 == 0);
}
inline void renderExtraFont(int index, int hour, int minute, bool colon, uint8_t rows[2][16]) {
  ExtraFont f=extraFont(index);
  memset(rows, ' ', 32);
  const int digits[4]={hour/10,hour%10,minute/10,minute%10};
  const int starts[4]={0,4,9,13};
  for(int row=0;row<2;row++) {
    for(int d=0;d<4;d++) {
      int x=starts[d]+(3-f.width)/2;
      for(int c=0;c<f.width;c++) {
        uint8_t code=pgm_read_byte(f.digits+row*(3+10*f.width)+3+digits[d]*f.width+c);
        rows[row][x+c]=code==0xfe?' ':code;
      }
    }
    rows[row][8]=colon?'.':' ';
  }
}
