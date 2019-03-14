#include "legato.h"
#include "interfaces.h"


//-------------------------------------------------------------------------
COMPONENT_INIT
{
    piOled_Display("Hello", 0);
    piOled_Display("this is a test", 1);
    piOled_Display("of", 2);
    piOled_Display("PiOled api component", 3);

    exit(EXIT_SUCCESS);
}
