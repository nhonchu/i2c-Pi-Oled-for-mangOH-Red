#include "legato.h"
#include "interfaces.h"


#include "ssd1306.h"

#include <errno.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

ssd1306_t *_LCD;

// private function to send the command to the LCD
static le_result_t _ssd1306_WriteCommand( ssd1306_t *pLcd, uint8_t pCmd )
{
    uint8_t buf[2] = { 0 };

    buf[0] = 0x00;
    buf[1] = pCmd;

    if ( write(pLcd->i2cFd,buf,2) != 2)
    {
        perror("error writing i2c");
        return LE_FAULT;
    }
    return LE_OK;
}

le_result_t ssd1306_Open(char* i2cDev, uint8_t pAddr, ssd1306_Display_t pLcdType)
{
    le_result_t result = LE_FAULT;

    uint8_t vccstate = 0;

    if ( _LCD != NULL )
    {
        LE_WARN("ssd1306 already open, close and reopen");
        ssd1306_Close();
    }
    _LCD = calloc(1, sizeof(ssd1306_t));

    _LCD->i2cFd = open(i2cDev, O_RDWR);
    if (_LCD->i2cFd < 0)
    {
        perror("cannot i2c device file");
        return LE_FAULT;
    }

    if (ioctl(_LCD->i2cFd, I2C_SLAVE_FORCE, pAddr) <0)
    {
        perror("cannot force i2c slave");
        return LE_FAULT;
    }

    _LCD->Addr = pAddr;
    _LCD->Type = pLcdType;
    switch ( pLcdType )
    {
        case SSD1306_LCD_96X16:
            _LCD->Width  = 96;
            _LCD->Height = 16;
            break;
        case SSD1306_LCD_128X32:
        case SSD1306_LCD_PIOLED:
            _LCD->Width  = 128;
            _LCD->Height = 32;
            break;
        case SSD1306_LCD_128x64:
            _LCD->Width  = 128;
            _LCD->Height = 64;
            break;
        default:
            LE_WARN("ssd1306 invalid LCD type");
            return LE_FAULT;
            break;
    }

    // set default font
    ssd1306_SetFont( SSD1306_FONT_FIXED4X6 );

    // allocate framebuffer and init to 0
    _LCD->Frame = calloc( (_LCD->Width * _LCD->Height )/8, sizeof(uint8_t) );

    // Init sequence
    result = _ssd1306_WriteCommand(_LCD, SSD1306_DISPLAYOFF);                    // 0xAE
    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    result = _ssd1306_WriteCommand(_LCD, 0x80);                                  // the suggested ratio 0x80

    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETMULTIPLEX);                  // 0xA8
    result = _ssd1306_WriteCommand(_LCD, _LCD->Height - 1);

    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETDISPLAYOFFSET);              // 0xD3
    result = _ssd1306_WriteCommand(_LCD, 0x0);                                   // no offset
    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETSTARTLINE | 0x0);            // line #0
    result = _ssd1306_WriteCommand(_LCD, SSD1306_CHARGEPUMP);                    // 0x8D
    if (vccstate == SSD1306_EXTERNALVCC)
    {
        result = _ssd1306_WriteCommand(_LCD, 0x10);
    }
    else
    {
        result = _ssd1306_WriteCommand(_LCD, 0x14);
    }
    result = _ssd1306_WriteCommand(_LCD, SSD1306_MEMORYMODE);                    // 0x20
    result = _ssd1306_WriteCommand(_LCD, 0x00);                                  // 0x0 act like ks0108
    result = _ssd1306_WriteCommand(_LCD, SSD1306_SEGREMAP | 0x1);
    result = _ssd1306_WriteCommand(_LCD, SSD1306_COMSCANDEC);

    switch ( _LCD->Type )
    {
        case SSD1306_LCD_96X16:
            result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCOMPINS);                    // 0xDA
            result = _ssd1306_WriteCommand(_LCD, 0x2);   //ada x12
            result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCONTRAST);                   // 0x81
            if (vccstate == SSD1306_EXTERNALVCC)
            {
                result = _ssd1306_WriteCommand(_LCD, 0x10);
            }
            else
            {
                result = _ssd1306_WriteCommand(_LCD, 0xAF);
            }
            break;
        case SSD1306_LCD_128X32:
        case SSD1306_LCD_PIOLED:
            result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCOMPINS);                    // 0xDA
            result = _ssd1306_WriteCommand(_LCD, 0x02);
            result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCONTRAST);                   // 0x81
            result = _ssd1306_WriteCommand(_LCD, 0x8F);
            break;
        case SSD1306_LCD_128x64:
            result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCOMPINS);                    // 0xDA
            result = _ssd1306_WriteCommand(_LCD, 0x12);
            result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCONTRAST);                   // 0x81
            if (vccstate == SSD1306_EXTERNALVCC)
            {
                result = _ssd1306_WriteCommand(_LCD, 0x9F);
            }
            else
            {
                result = _ssd1306_WriteCommand(_LCD, 0xCF);
            }
            break;
        default:
            break;
    }
    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETPRECHARGE);                  // 0xd9
    if (vccstate == SSD1306_EXTERNALVCC)
    {
        result = _ssd1306_WriteCommand(_LCD, 0x22);
    }
    else
    {
        result = _ssd1306_WriteCommand(_LCD, 0xF1);
    }
    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETVCOMDETECT);                 // 0xDB
    result = _ssd1306_WriteCommand(_LCD, 0x40);
    result = _ssd1306_WriteCommand(_LCD, SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    result = _ssd1306_WriteCommand(_LCD, SSD1306_NORMALDISPLAY);                 // 0xA6

    result = _ssd1306_WriteCommand(_LCD, SSD1306_DEACTIVATE_SCROLL);

    memset(_LCD->Frame, 0, (_LCD->Width * _LCD->Height / 8) );

    ssd1306_Show();

    result = _ssd1306_WriteCommand(_LCD, SSD1306_DISPLAYON);                   //--turn on oled panel

    return result;
}

/*
 * Close: Close the SSD1306 instance
 */

le_result_t ssd1306_Close( void )
{
    le_result_t result = LE_FAULT;

    close(_LCD->i2cFd);

    if ( _LCD != NULL )
    {
        if ( _LCD->Frame != NULL )
        {
            free(_LCD->Frame);
        }
        free(_LCD);
        _LCD = NULL;
        result = LE_OK;
    }

    return result;
}

// push the buffer to the screen using I2C
le_result_t ssd1306_Show( void )
{
    le_result_t result = LE_FAULT;

    result = _ssd1306_WriteCommand(_LCD, SSD1306_COLUMNADDR);
    result = _ssd1306_WriteCommand(_LCD, 0);                           // Column start address (0 = reset)
    result = _ssd1306_WriteCommand(_LCD, _LCD->Width - 1);                // Column end address (127 = reset)

    result = _ssd1306_WriteCommand(_LCD, SSD1306_PAGEADDR);
    result = _ssd1306_WriteCommand(_LCD, 0);                           // Page start address (0 = reset)
    result = _ssd1306_WriteCommand(_LCD, (_LCD->Height/8)-1);              // Page end address - 1 page is 8 pixels high

    // I2C
    // send 128 bytes at a time
    int16_t rowLen = 128;
    for (int16_t Row=0; Row < (_LCD->Width * _LCD->Height / 8)/rowLen; Row++ )
    {
        uint8_t buffer[130] = { 0 };
        buffer[0] = 0x40;
        memcpy(&buffer[1], _LCD->Frame+(Row*rowLen), rowLen);


        if ( write(_LCD->i2cFd,buffer, rowLen+1) != (rowLen +1) )
        {
            perror("error writing i2c");
            return LE_FAULT;
        }
    }

    return result;
}

le_result_t ssd1306_Dim( void )
{
    le_result_t result = LE_FAULT;

    result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCONTRAST);
    result = _ssd1306_WriteCommand(_LCD, 0x00);

    return result;
}

le_result_t ssd1306_Bright( void )
{
    le_result_t result = LE_FAULT;

    uint8_t vccstate= 0;
    uint8_t contrast = 0x00;

    switch ( _LCD->Type )
    {
        case SSD1306_LCD_96X16:
            if (vccstate == SSD1306_EXTERNALVCC)
            {
                contrast = 0x10;
            }
            else
            {
                contrast = 0xAF;
            }
            break;
        case SSD1306_LCD_128X32:
        case SSD1306_LCD_PIOLED:
            contrast = 0x8F;
            break;
        case SSD1306_LCD_128x64:
            if (vccstate == SSD1306_EXTERNALVCC)
            {
                contrast = 0x9F;
            }
            else
            {
                contrast = 0xCF;
            }
            break;
        default:
            break;
    }

    if ( contrast != 0x00 )
    {
        result = _ssd1306_WriteCommand(_LCD, SSD1306_SETCONTRAST);                   // 0x81
        result = _ssd1306_WriteCommand(_LCD, contrast);
    }

    return result;
}


le_result_t ssd1306_Clear( void )
{
    memset(_LCD->Frame, 0x00, ( _LCD->Width * _LCD->Height / 8 ));
    return LE_OK;
}

int16_t ssd1306_GetWidth( void )
{
    if ( _LCD ) return _LCD->Width;
    else        return -1;
}

int16_t ssd1306_GetHeight( void )
{
    if ( _LCD ) return _LCD->Height;
    else        return -1;
}

le_result_t ssd1306_PutPixel(int16_t pX, int16_t pY, ssd1306_Colour_t pColour)
{
    if ((pX < 0) || (pX >= _LCD->Width) || (pY < 0) || (pY >= _LCD->Height))
        return LE_FAULT;

#ifdef __BUILDME__
    // check rotation, move pixel around if necessary
    switch (getRotation())
    {
        case 1:
            ssd1306_swap(pX, pY);
            pX = _XPixels - pX - 1;
            break;
        case 2:
            pX = _XPixels - pX - 1;
            pY = _YPixels - pY - 1;
            break;
        case 3:
            ssd1306_swap(pX, pY);
            pY = _YPixels - pY - 1;
            break;
        default: break;
    }
#endif

    // pX is which column
    switch (pColour)
    {
        case SSD1306_WHITE:   _LCD->Frame[pX+ (pY/8)*_LCD->Width] |=  (1 << (pY&7)); break;
        case SSD1306_BLACK:   _LCD->Frame[pX+ (pY/8)*_LCD->Width] &= ~(1 << (pY&7)); break;
        case SSD1306_INVERSE: _LCD->Frame[pX+ (pY/8)*_LCD->Width] ^=  (1 << (pY&7)); break;
        default:
            break;
    }

    return LE_OK;
}

le_result_t ssd1306_SetFont( ssd1306_Font_t pFont )
{
    le_result_t result = LE_OK;

    switch ( pFont )
    {
        case SSD1306_FONT_FIXED4X6:
            _LCD->FontId = pFont;
            _LCD->Font   = (FONT_INFO*)&fixed_4x6_FontInfo;
            break;
        case SSD1306_FONT_FIXED5X8:
            _LCD->FontId = pFont;
            _LCD->Font   = (FONT_INFO*)&fixed_5x8_FontInfo;
            break;
        default:
            result = LE_FAULT;
            break;
    }
    return result;
}

ssd1306_Font_t ssd1306_GetFont( void )
{
    return _LCD->FontId;
}

le_result_t ssd1306_GetFontInfo( ssd1306_Font_t pFont, char* pName, size_t pNameSize, int16_t* pHeightPtr, char* pFirstPtr, char* pLastPtr)
{
    // TODO?
    return LE_FAULT;
}

int16_t ssd1306_GetFontHeight( void )
{
    return _LCD->Font->heightPages;
}

int16_t ssd1306_GetStringWidth( const char *pStr )
{
    uint8_t cols = 0;
    uint8_t width = 0;
    uint8_t charIdx;

    FONT_INFO *font = _LCD->Font;          // current font

    while (*pStr != 0x00) // loop until null terminator
    {
        charIdx = *pStr - font->startChar;
        cols = font->charInfo[charIdx].widthBits;
        width += cols;
        width += 1; // inter char space!
        pStr++;
    }
    if ( width > 0 ) width -=1; // remove last inter-char space

    return width;
}

le_result_t ssd1306_PutCharacter( char pChar, int16_t pX, int16_t pY, ssd1306_Colour_t pColour )
{
    le_result_t result = LE_FAULT;
    uint8_t column;
    uint8_t cols;  // number of columns (in bits) for this character
    uint8_t rows;  // number of rows (in bits) for this character
    int16_t offset; // offset into data array
    uint8_t charIdx;

    FONT_INFO *font = _LCD->Font;          // current font

    charIdx = pChar - font->startChar;
    rows = font->heightPages;
    cols = font->charInfo[charIdx].widthBits;
    offset = font->charInfo[charIdx].offset;

    for (int16_t i=0; i<cols; i++)
    {
        offset = font->charInfo[charIdx].offset + i;

        for (int j=0; j<rows; j++) // rows(bits) (y) per character
        {
            if ( j%8 == 0 )
            {
                // do this in two steps to stop the compiler optimising it (or doing something weird)
                uint8_t *P = (font->data)+offset; // get column info
                column = *P; // get column info
                offset+=cols;
            }
            if (column & (0x01 << (j%8))) // test bits to set pixels
                result = ssd1306_PutPixel(pX+i, pY+j, pColour);  // WHITE
            else
                result = ssd1306_PutPixel(pX+i, pY+j, SSD1306_BLACK);  // if colour==INVERSE, INVERT pixel; else if WHITE draw BLACK etc
        }
    }

    return result;
}

le_result_t ssd1306_PutString(const char *pStr, int16_t x, int16_t y, ssd1306_Colour_t pColour)
{
    le_result_t result = LE_FAULT;
    uint8_t cols;  // number of columns for this character
    uint8_t rows;
    uint8_t charIdx;

    FONT_INFO *font = _LCD->Font;          // current font

    rows = font->heightPages;

    while (*pStr != 0x00) // loop until null terminator
    {
        charIdx = *pStr - font->startChar;
        cols = font->charInfo[charIdx].widthBits;

        result = ssd1306_PutCharacter(*pStr++, x, y, pColour);
        x+=cols;
        for (int i=y; i<y+rows; i++)
        {
            result = ssd1306_PutPixel(x, i, (pColour == SSD1306_INVERSE)? SSD1306_INVERSE :
                                         (pColour == SSD1306_WHITE) ? SSD1306_BLACK : SSD1306_WHITE );    // space is always 'INVERSE' of text colour
        }
        x++;
        if (x > (_LCD->Width - cols)) // Enables wrap around
        {
            x = 0;
            y += rows;
        }
    }

    return result;
}

