#include <3ds.h>

#include "output.h"
#include "tests/test.h"
#include "tests/fs/fs.h"
#include "tests/cpu/cputests.h"
#include "tests/kernel/kernel.h"
#include "tests/gpu/gpu.h"

static unsigned int test_counter = 0;
static TestCaller tests[] = {
    FS::TestAll,
    CPU::Integer::TestAll,
    CPU::Memory::TestAll,
    Kernel::TestAll,
    GPU::TestAll
};

int main(int argc, char** argv)
{
    gfxInitDefault();
    InitOutput();

    consoleClear();
    Print("Press A to begin...\n");

    while (aptMainLoop()) {
        gfxFlushBuffers();
        gfxSwapBuffers();

        hidScanInput();
        if (hidKeysDown() & KEY_START) {
            break;
        } else if (hidKeysDown() & KEY_A) {
            consoleClear();

            if (test_counter < (sizeof(tests) / sizeof(tests[0]))) {
                tests[test_counter]();
                test_counter++;
            } else {
                break;
            }

            Log("\n");
            Print("Press A to continue...\n");
        }

        gspWaitForVBlank();
    }

    consoleClear();

    gfxExit();
    DeinitOutput();

    return 0;
}
