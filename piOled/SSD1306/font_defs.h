#include "legato.h"


#ifndef __FONT_DEFS_H__
#define __FONT_DEFS_H__

/* The Dot Factory */

// ==========================================================================
// structure definition
// ==========================================================================

// This structure describes a single character's display information
typedef struct
{
  uint8_t widthBits;         // width, in bits (or pixels), of the character
  uint16_t offset;         // offset of the character's bitmap, in bytes, into the the FONT_INFO's data array

} FONT_CHAR_INFO;

// Describes a single font
typedef struct
{
  uint8_t      heightPages;  // height, in pages (8 pixels), of the font's characters
  uint8_t      startChar;    // the first character in the font (e.g. in charInfo and data)
  uint8_t      endChar;       // last character in font
//  uint8_t      charCount;    // number of characters in fontset
  FONT_CHAR_INFO* charInfo; // pointer to array of char information
  uint8_t*     data;     // pointer to generated array of character visual representation

} FONT_INFO;

#endif  /* __FONT_DEFS_H__ */

