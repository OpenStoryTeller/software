 
#include "ost_common.h"
#include "debug.h"
#include "filesystem.h"
#include "picture.h"

int main(void)
{
    // Low level initialization, mainly platform stuff
    // After this call, debug_printf *MUST* be available
    system_initialize();
    debug_printf("\r\n>>>>> Starting OpenStoryTeller: V%d.%d <<<<<\n", 1, 0);

    // File system access
    filesystem_mount();

    // Display
    ost_display_initialize();
    decompress();

    // Audio

    for (;;)
    {
    }
    
    return 0;
}
