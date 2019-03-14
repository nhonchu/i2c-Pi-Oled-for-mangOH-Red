#include "legato.h"
#include "interfaces.h"

#include "ssd1306.h"

// protect LCD screen write access
static le_mutex_Ref_t mutex;

#define MAX_LINE 4

//-------------------------------------------------------------------------
static void sigHandlerSigTerm( int pSigNum )
{
    LE_INFO("shutting down ssd1306 OLED service");
    ssd1306_Close();

}

//-------------------------------------------------------------------------
le_result_t piOled_Display
(
    const char*   message,
    uint8_t       lineNumber
)
{
    if (lineNumber > MAX_LINE)
    {
        LE_ERROR("piOled - invalid line lineNumber");
        return LE_FAULT;
    }

    le_mutex_Lock(mutex);

    char v[22] = {0};
    snprintf(v,22,"%-22s",message);

    int offset = lineNumber * 8;

    ssd1306_PutString(v, 0, offset, SSD1306_WHITE );

    ssd1306_Show();

    le_mutex_Unlock(mutex);

    return LE_OK;
}

//-------------------------------------------------------------------------
COMPONENT_INIT
{
    le_result_t res = LE_FAULT;


    LE_INFO( "Starting ssd1306 OLED" );

    // setup to catch application termination and shutdown cleanly
    le_sig_Block( SIGTERM );
    le_sig_SetEventHandler( SIGTERM, sigHandlerSigTerm );

    LE_INFO( "ssd1306 OLED service started OK" );

    mutex = le_mutex_CreateRecursive("lcd");

    res = ssd1306_Open("/dev/i2c-8", 0x3c, SSD1306_LCD_PIOLED);
    if (res != LE_OK) 
    {
        LE_ERROR("error opening the lcd");
        exit(EXIT_SUCCESS);
    }
    else
    {
        ssd1306_Clear();
        
        ssd1306_SetFont( SSD1306_FONT_FIXED5X8 );
        
        ssd1306_PutString("piOled Ready", 0, 0, SSD1306_WHITE );
        ssd1306_Show();
        
    }
}
