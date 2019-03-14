/*
 * Code modified from Adafruit SSD_1306 driver
 */

#include "legato.h"
#include "interfaces.h"

#include "font_defs.h"

#include "font_fixed.h"


#ifndef __SSD1306_H__
#define __SSD1306_H__

#define SSD1306_SETCONTRAST          0x81
#define SSD1306_DISPLAYALLON_RESUME  0xA4
#define SSD1306_DISPLAYALLON         0xA5
#define SSD1306_NORMALDISPLAY        0xA6
#define SSD1306_INVERTDISPLAY        0xA7
#define SSD1306_DISPLAYOFF           0xAE
#define SSD1306_DISPLAYON            0xAF

#define SSD1306_SETDISPLAYOFFSET     0xD3
#define SSD1306_SETCOMPINS           0xDA

#define SSD1306_SETVCOMDETECT        0xDB

#define SSD1306_SETDISPLAYCLOCKDIV   0xD5
#define SSD1306_SETPRECHARGE         0xD9

#define SSD1306_SETMULTIPLEX         0xA8

#define SSD1306_SETLOWCOLUMN         0x00
#define SSD1306_SETHIGHCOLUMN        0x10

#define SSD1306_SETSTARTLINE         0x40

#define SSD1306_MEMORYMODE           0x20
#define SSD1306_COLUMNADDR           0x21
#define SSD1306_PAGEADDR             0x22

#define SSD1306_COMSCANINC           0xC0
#define SSD1306_COMSCANDEC           0xC8

#define SSD1306_SEGREMAP             0xA0

#define SSD1306_CHARGEPUMP           0x8D

#define SSD1306_EXTERNALVCC          0x01
#define SSD1306_SWITCHCAPVCC         0x02

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL                      0x2F
#define SSD1306_DEACTIVATE_SCROLL                    0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A

typedef enum
{
    SSD1306_LCD_NONE = 0,
    SSD1306_LCD_PIOLED = 1,
    SSD1306_LCD_96X16 = 2,
    SSD1306_LCD_128X32 = 3,
    SSD1306_LCD_128x64 = 4
}
ssd1306_Display_t;

typedef enum
{
    SSD1306_BLACK = 0,
    SSD1306_WHITE = 1,
    SSD1306_INVERSE = 2
}
ssd1306_Colour_t;

typedef enum
{
    SSD1306_FONT_FIXED4X6 = 0,
    SSD1306_FONT_FIXED5X8 = 1,
    SSD1306_FONT_LAST = 2
}
ssd1306_Font_t;

// LCD screen
typedef struct
{
    int                  i2cFd;
    uint8_t              Addr;     // I2C address
    ssd1306_Display_t Type;        // LCD type
    int16_t              Width;    // width in pixels
    int16_t              Height;   // height in pixels
    uint8_t             *Frame;    // pointer to framebuffer
    ssd1306_Font_t    FontId;      // current font ID
    FONT_INFO           *Font;     // current font
} ssd1306_t;



le_result_t ssd1306_Open(char* i2cDev, uint8_t pAddr, ssd1306_Display_t pLcdType );
le_result_t ssd1306_Show( void );
le_result_t ssd1306_Close();

le_result_t ssd1306_Clear( void );
le_result_t ssd1306_SetFont( ssd1306_Font_t pFont );
le_result_t ssd1306_PutString(const char *pStr, int16_t x, int16_t y, ssd1306_Colour_t pColour);

#endif
