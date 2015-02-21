#include <3ds.h>

#include "output.h"
#include "tests/test.h"
#include "tests/fs/fs.h"
#include "tests/cpu/cputests.h"
#include "tests/cfg/cfg.h"
#include "tests/kernel/kernel.h"

static unsigned int test_counter = 0;
static TestCaller tests[] = {
    FS::TestAll,
    Cfg::TestAll,
    CPU::Memory::TestAll,
    Kernel::TestAll,
};

int main(int argc, char** argv)
{
    gfxInitDefault();
    initCfgu();
    InitOutput();

    ClearScreens();
    Print(GFX_TOP, "Press A to begin...\n");

    while (aptMainLoop()) {
        DrawBuffers();

        hidScanInput();
        if (hidKeysDown() & KEY_START) {
            break;
        } else if (hidKeysDown() & KEY_A) {
            ClearScreens();

            if (test_counter < (sizeof(tests) / sizeof(tests[0]))) {
                tests[test_counter]();
                test_counter++;
            } else {
                break;
            }

            Log(GFX_TOP, "\n");
            Print(GFX_TOP, "Press A to continue...\n");
        }

        gspWaitForEvent(GSPEVENT_VBlank0, false);
    }

    clearScreens();
    exitCfgu();
    gfxExit();
    DeinitOutput();
	
    return 0;
}
