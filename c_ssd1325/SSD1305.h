/* 
 * File:   SSD1305.h
 * Author: mondul_air
 *
 * Created on 7 de noviembre de 2014, 15:31
 */

#ifndef SSD1305_H
#define SSD1305_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "hardware.h" // <--!! comentada ya que no existe
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "inttypes.h"
#include "stdbool.h"
#include "font5x7.h"
//PARA CONTROLAR DISPLAY
#define SSD1325_CS	14//chip select display
#define SSD1325_DCX	67//data comand pin
#define SSD1325_RST	68//rst pin 

// From SSD1305.h by "Andrea" with a few additions OLD
#define SSD1305_SETLOWCOLUMN        0x00 // 0x00 - 0F Set the lower nibble of the column start address register for Page Addressing Mode 
#define SSD1305_SETHIGHCOLUMN       0x10 // 0x10 - 0x1F Set the higher nibble of the column start address register for Page Addressing Mode.
#define SSD1305_MEMORYMODE          0x20 // 00b Hor Add, 01b Vert Add, 10b Page Add, 11b NA
#define SSD1305_SETSTARTCOLUMN      0x21 // Column Start Address 0 - 0x83, Column End Address 0 - 0x83
#define SSD1305_SETSTARTPAGE        0x22 // Page Start Address 0 - 0x07, Page End Address 0 - 0x07
#define SSD1305_SETSTARTLINE        0x40 // Set display start line register from 0 - 0x3F.
#define SSD1305_SETCONTRAST         0x81 // 1 out of 256 contrast steps. Contrast increases as the value increases.
#define SSD1305_SETBRIGHTNESS       0x82 // 1 out of 256 contrast steps. Brightness increases as the value increases.
#define SSD1305_LOOK_UP_TABLE       0x91 // Set look-up table
#define SSD1305_SEGREMAP0           0xA0 // Column address 0 is mapped to SEG0
#define SSD1305_SEGREMAP1           0xA1 // Column address 0x83 is mapped to SEG0
#define SSD1305_DISPLAYALLON_RESUME 0xA4 //  Resume RAM content display. Output follows RAM content
#define SSD1305_DISPLAYALLON        0xA5 // Entire display ON. Output ignores RAM content.
#define SSD1305_NORMALDISPLAY       0xA6 // Normal display
#define SSD1305_INVERTDISPLAY       0xA7 // Inverse display
#define SSD1305_SETMULTIPLEX        0xA8 //= Set MUX ratio to N+1 MUX, from 16MUX to 64MUX (0 to 14 are invalid)
#define SSD1305_DISPLAY_ON_DIM      0xAC // Display ON in dim mode
#define SSD1305_DCVOLTCONVERT       0xAD // Selects external VCC supply
#define SSD1305_DISPLAYOFF          0xAE // Display OFF (sleep mode)
#define SSD1305_DISPLAYON           0xAF // Display ON in normal mode
#define SSD1305_SET_PAGE            0xB0 // Set GDRAM Page Start Address for Page Addressing Mode - PAGE0~PAGE7
#define SSD1305_COMSCANDEC          0xC8 // Set COM Output Scan Direction  Remapped mode. Scan from COM[N?1] to COM0
#define SSD1305_SETDISPLAYOFFSET    0xD3 // Set vertical shift by COM from 0~63.
#define SSD1305_SETDISPLAYCLOCKDIV  0xB3 // * Set Display Clock Divide Ratio/Oscillator frequency
#define SSD1305_AREACOLOR_POWERMODE 0xD8 // Set Area Color Mode ON/OFF & Low Power Display Mode
#define SSD1305_SETPRECHARGE        0xD9 // Set Pre?charge Period up to 15DCLK. 0 is not valid.
#define SSD1305_SETCOMPINS          0xDA // Sequential COM pin configuration
#define SSD1305_SETVCOMDETECT       0xDB // Set VCOMH Deselect Level 0.43*VCC, 0.77*VCC, 0.83*VCC
#define SSD1305_SET_MODIFY          0xE0 // Enter the Read/Modify/Write mode
#define SSD1305_CLR_MODIFY          0xEE // Exit the Read/Modify/Write mode.
#define SSD1305_NOP                 0xE3 // Command for No Operation
#define SSD1305_SETHORISCROLL       0x26 // Horizontal scroll setup
#define SSD1305_SETVERTSCROLL       0xA3 // Set vertical scroll area
#define SSD1305_SETCONTSCROLL       0x29 // Continuous vertical & horizontal scroll setup
#define SSD1305_CLRSCROLL           0x2E // Deactivate scroll
#define SSD1305_SETSCROLL           0x2F // Activate scroll

//controlar SSD1325
#define SSD1325_SETCOLADDR 0x15
#define SSD1325_SETROWADDR 0x75
#define SSD1325_SETCONTRAST 0x81
#define SSD1325_SETCURRENT 0x84
#define SSD1325_SETREMAP 0xA0
#define SSD1325_SETSTARTLINE 0xA1
#define SSD1325_SETOFFSET 0xA2
#define SSD1325_NORMALDISPLAY 0xA4
#define SSD1325_DISPLAYALLON 0xA5
#define SSD1325_DISPLAYALLOFF 0xA6
#define SSD1325_INVERTDISPLAY 0xA7
#define SSD1325_SETMULTIPLEX 0xA8// =
#define SSD1325_MASTERCONFIG 0xAD
#define SSD1325_DISPLAYOFF 0xAE//   =
#define SSD1325_DISPLAYON 0xAF//    =
#define SSD1325_SETPRECHARGECOMPENABLE 0xB0
#define SSD1325_SETPHASELEN 0xB1
#define SSD1325_SETROWPERIOD 0xB2
#define SSD1325_SETCLOCK 0xB3 //   != D5
#define SSD1325_SETPRECHARGECOMP 0xB4
#define SSD1325_SETGRAYTABLE 0xB8
#define SSD1325_SETPRECHARGEVOLTAGE 0xBC
#define SSD1325_SETVCOMLEVEL 0xBE
#define SSD1325_SETVSL 0xBF
#define SSD1325_GFXACCEL 0x23
#define SSD1325_DRAWRECT 0x24
#define SSD1325_COPY 0x25


#define DISPLAY_WIDTH   128 // X
#define DISPLAY_HEIGHT  64  // Y fix to SSD1325
#define PAGEMAX         6 // DISPLAY_HEIGHT/8    // 48pixel/8pixel= 6 pages
#define GDRAM_WIDTH     132
#define GDRAM_HEIGHT    32 // Is really 64
#define CHAR7x5_WIDTH   5   // 7x5 char
#define CHAR7x5_HEIGHT  8   // 7 + blank space
#define BUFFER_LENGTH   528 // 1056 // (GDRAM_WIDTH * GDRAM_HEIGHT) / 8

#define DISPLAY_NORMAL  0  // Previously 0

#define DISPLAY_X_MIRROR    1
#define DISPLAY_Y_MIRROR    2
#define DISPLAY_XY_MIRROR   3
#define MAX_Y_CHAR          4   // DISPLAY_HEIGHT/CHAR7x5_HEIGHT
#define MAX_X_CHAR          21  // DISPLAY_WIDTH/(CHAR7x5_WIDTH+1)

    uint8_t display_buffer[BUFFER_LENGTH];
    bool buffer_update;
    int WriteSPI2(uint8_t data);
    int gpioexport(int gpioid);
    int gpiosetdir(int gpioid,char *mode);
    int gpiosetbits(int gpioid);
    int gpioclearbits(int gpioid);

    static void oled_write_command(uint8_t command);
    static void oled_write_data(uint8_t data);
    static void oled_init(void);

    void oled_buffer_putc(uint8_t character, uint8_t X, uint8_t Y, bool hL);
    void oled_buffer_puts(const uint8_t* string, uint8_t X, uint8_t Y, bool hL);
    void oled_buffer_printf(uint8_t* string, uint8_t X, uint8_t Y, bool hL);
    void oled_buffer_update();


#ifdef __cplusplus
}
#endif

#endif /* SSD1305_H */
