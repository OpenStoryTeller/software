
#include "ost_common.h"
#include "debug.h"
#include "filesystem.h"
#include "picture.h"
#include "ost_tasker.h"

#define RUN_TESTS 1

#ifndef RUN_TESTS
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

    // Tasker
    ost_tasker_init();

    for (;;)
    {
    }

    return 0;
}
#else
int main()
{
    system_initialize();

    debug_printf("\r\n>>>>> Starting OpenStoryTeller tests: V%d.%d <<<<<\n", 1, 0);

    for (;;)
    {
        system_led_write(1);
        system_delay_ms(1000);
        system_led_write(0);
        system_delay_ms(1000);
    }
    return 0;
}
#endif
