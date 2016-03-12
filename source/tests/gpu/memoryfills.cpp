#include <limits>
#include <string.h>
#include "output.h"
#include "common/scope_exit.h"
#include "common/string_funcs.h"
#include "tests/test.h"
#include "tests/gpu/memoryfills.h"

namespace GPU {
namespace MemoryFills {

static void FillAndWait(u8* buffer, u32 value, u32 size, u32 control, bool first_engine = true) {
    if (first_engine) {
        GX_MemoryFill((u32*)buffer, value, (u32*)(buffer + size), control, 0, 0, 0, 0);
        gspWaitForPSC0();
    } else {
        GX_MemoryFill(0, 0, 0, control, (u32*)buffer, value, (u32*)(buffer + size), control);
        gspWaitForPSC1();
    }
    GSPGPU_InvalidateDataCache(buffer, size);
}

static bool Fill32Bits(u8* buffer) {
    FillAndWait(buffer, 0x00FFFFFF, 48, 0x201, false);
    TestEquals(buffer[0], 0xFFu);
    TestEquals(buffer[1], 0xFFu);
    TestEquals(buffer[2], 0xFFu);
    TestEquals(buffer[3], 0x00u);
    TestEquals(buffer[4], 0xFFu);
    TestEquals(buffer[5], 0xFFu);
    TestEquals(buffer[6], 0xFFu);
    TestEquals(buffer[7], 0x00u);
    TestEquals(buffer[48], 0x00u);
    TestEquals(buffer[49], 0x00u);
    TestEquals(buffer[49], 0x00u);

    FillAndWait(buffer, 0x0000FFFF, 48, 0x201, false);
    TestEquals(buffer[0], 0xFFu);
    TestEquals(buffer[1], 0xFFu);
    TestEquals(buffer[2], 0x00u);
    TestEquals(buffer[3], 0x00u);
    TestEquals(buffer[4], 0xFFu);
    TestEquals(buffer[5], 0xFFu);
    TestEquals(buffer[6], 0x00u);
    TestEquals(buffer[7], 0x00u);

    return true;
}

static bool Fill24Bits(u8* buffer) {
    FillAndWait(buffer, 0x00FFFFFF, 48, 0x101);
    TestEquals(buffer[0], 0xFFu);
    TestEquals(buffer[1], 0xFFu);
    TestEquals(buffer[2], 0xFFu);
    TestEquals(buffer[3], 0xFFu);
    TestEquals(buffer[4], 0xFFu);
    TestEquals(buffer[5], 0xFFu);
    TestEquals(buffer[6], 0xFFu);
    TestEquals(buffer[7], 0xFFu);
    TestEquals(buffer[48], 0x00u);
    TestEquals(buffer[49], 0x00u);
    TestEquals(buffer[49], 0x00u);

    FillAndWait(buffer, 0xFFFFFFFF, 48, 0x101);
    TestEquals(buffer[0], 0xFFu);
    TestEquals(buffer[1], 0xFFu);
    TestEquals(buffer[2], 0xFFu);
    TestEquals(buffer[3], 0xFFu);

    FillAndWait(buffer, 0x00FFFF00, 48, 0x101);
    TestEquals(buffer[0], 0x00u);
    TestEquals(buffer[1], 0xFFu);
    TestEquals(buffer[2], 0xFFu);
    TestEquals(buffer[3], 0x00u);

    return true;
}

void TestAll() {
    const std::string tag = "MemoryFills";

    u8* buffer = (u8*)vramAlloc(0x400);

    Test(tag, "Fill24Bits", Fill24Bits(buffer), true);
    Test(tag, "Fill32Bits", Fill32Bits(buffer), true);

    vramFree(buffer);
}

}
}
