#include <limits>
#include <string.h>
#include "output.h"
#include "common/scope_exit.h"
#include "common/string_funcs.h"
#include "tests/test.h"
#include "tests/gpu/displaytransfer.h"

namespace GPU {
namespace DisplayTransfer {

enum PixelFormat {
    IN_RGBA8  = 0 << 8,
    IN_RGB8   = 1 << 8,
    IN_RGB565 = 2 << 8,
    IN_RGB5A1 = 3 << 8,
    IN_RGBA4  = 4 << 8,
    OUT_RGBA8  = 0 << 12,
    OUT_RGB8   = 1 << 12,
    OUT_RGB565 = 2 << 12,
    OUT_RGB5A1 = 3 << 12,
    OUT_RGBA4  = 4 << 12,
};

enum Flags {
    LINEAR_TO_TILED = 1 << 1,
    RAW_COPY = 1 << 3,
    NO_SWIZZLE = 1 << 5,
    UNKNOWN1 = 1 << 16,
    HORIZONTAL_DOWNSCALE = 1 << 24,
    DOUBLE_DOWNSCALE = 1 << 25,
};

union Dimensions {
    struct {
        u16 height;
        u16 width;
    } dims;
    u32 raw;

    Dimensions(u16 height, u16 width) {
        dims.height = height;
        dims.width = width;
    }
};

static void DisplayTransferAndWait(u32* input, u32* output, Dimensions input_dimensions, Dimensions output_dimensions, u32 flags) {
    GSPGPU_FlushDataCache((u8*)input, sizeof(u32));
    GSPGPU_InvalidateDataCache((u8*)output, sizeof(u32));
    Result res = GX_DisplayTransfer((u32*)input, input_dimensions.raw, (u32*)output, output_dimensions.raw, flags);
    if ((u32)res != 0) {
        Log(Common::FormatString("Something went wrong: %u\n", (u32)res));
        return;
    }
    gfxFlushBuffers();
    gspWaitForPPF();
}

static bool RGBA4_To_RGB5A1(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xF000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA4 | OUT_RGB5A1);
    TestEquals(*output, (u32)0xF800);

    // Test Green Input
    *input = 0x0F00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA4 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x07C0);

    // Test Blue Input
    *input = 0x00F0; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA4 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x003E);

    // Test 15 Alpha Input
    *input = 0x000F; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA4 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);

    // Test 8 Alpha Input
    *input = 0x0008; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA4 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);

    // Test 7 Alpha Input
    *input = 0x0007; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA4 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0000);
    return true;
}

static bool RGBA8_To_RGBA8(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    //Test Red Input
    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8);
    TestEquals(*output, (u32)0xFF000000);

    //Test Green Input
    *input = 0x00FF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8);
    TestEquals(*output, (u32)0x00FF0000);

    //Test Blue Input
    *input = 0x0000FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8);
    TestEquals(*output, (u32)0x0000FF00);

    //Test Alpha Input
    *input = 0x000000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8);
    TestEquals(*output, (u32)0x000000FF);
    return true;
}

static bool RGBA8_To_RGB8(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    //Test Red Input
    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x00FF0000u);

    //Test Green Input
    *input = 0x00FF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x0000FF00u);

    //Test Blue Input
    *input = 0x0000FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x000000FFu);

    //Test Alpha Input
    *input = 0x000000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x00000000u);
    return true;
}

static bool RGBA8_To_RGB565(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    //Test Red Input
    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB565);
    TestEquals(*output, (u32)0xF800);

    //Test Green Input
    *input = 0x00FF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB565);
    TestEquals(*output, (u32)0x07E0);

    //Test Blue Input
    *input = 0x0000FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB565);
    TestEquals(*output, (u32)0x001F);

    //Test Alpha Input
    *input = 0x000000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB565);
    TestEquals(*output, (u32)0x0000);
    return true;
}

static bool RGBA8_To_RGB5A1(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0xF800);

    // Test Green Input
    *input = 0x00FF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x07C0);

    // Test Blue Input
    *input = 0x0000FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x003E);

    // Test 255 Alpha Input
    *input = 0x000000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);

    // Test 100 Alpha Input
    *input = 0x00000064; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0000);

    // Test 127 Alpha Input
    *input = 0x0000007F; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0000);

    // Test 128 Alpha Input
    *input = 0x00000080; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);

    // Test 254 Alpha Input
    *input = 0x000000FE; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);
    return true;
}

static bool RGBA8_To_RGBA4(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0xF000);

    // Test Green Input
    *input = 0x00FF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x0F00);

    // Test Blue Input
    *input = 0x0000FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x00F0);

    // Test 255 Alpha Input
    *input = 0x000000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x000F);

    // Test 100 Alpha Input
    *input = 0x00000064; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x0006);

    // Test 127 Alpha Input
    *input = 0x0000007F; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x0007);

    // Test 128 Alpha Input
    *input = 0x00000080; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x0008);

    // Test 254 Alpha Input
    *input = 0x000000FE; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x000F);
    return true;
}

static bool RGB8_To_RGB8(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    //Test Red Input
    *input = 0xFF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB8);
    TestEquals(*output, (u32)0xFF0000);

    //Test Green Input
    *input = 0x00FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB8);
    TestEquals(*output, (u32)0x00FF00);

    //Test Blue Input
    *input = 0x0000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB8);
    TestEquals(*output, (u32)0x0000FF);
    return true;
}

static bool RGB8_To_RGB565(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    //Test Red Input
    *input = 0xFF0000; //Input
    *output = 0; //Output
    Log("Preparing first\n");
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB565);
    TestEquals(*output, (u32)0xF800);

    //Test Green Input
    *input = 0x00FF00; //Input
    *output = 0; //Output
    Log("Preparing second\n");
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB565);
    TestEquals(*output, (u32)0x07E0);

    //Test Blue Input
    *input = 0x0000FF; //Input
    *output = 0; //Output
    Log("Preparing third\n");
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB565);
    TestEquals(*output, (u32)0x001F);
    return true;
}

static bool RGB8_To_RGB5A1(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xFF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0xF801);

    // Test Green Input
    *input = 0x00FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x07C1);

    // Test Blue Input
    *input = 0x0000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x003F);

    *input = 0x000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);
    return true;
}

static bool RGB8_To_RGBA4(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xFF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGBA4);
    TestEquals(*output, (u32)0xF00F);

    // Test Green Input
    *input = 0x00FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x0F0F);

    // Test Blue Input
    *input = 0x0000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x00FF);

    *input = 0x000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB8 | OUT_RGBA4);
    TestEquals(*output, (u32)0x000F);
    return true;
}

static bool RGB5A1_To_RGB5A1(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xF800; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB5A1);
    TestEquals(*output, (u32)0xF800);

    // Test Green Input
    *input = 0x07C0; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x07C0);

    // Test Blue Input
    *input = 0x003E; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x003E);

    // Test Alpha Input
    *input = 0x0001; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB5A1);
    TestEquals(*output, (u32)0x0001);
    return true;
}

static bool RGB5A1_To_RGB565(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xF800; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB565);
    TestEquals(*output, (u32)0xF800);

    // Test Green Input
    *input = 0x07C0; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB565);
    TestEquals(*output, (u32)0x07E0);

    // Test Blue Input
    *input = 0x003E; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB565);
    TestEquals(*output, (u32)0x001F);

    // Test Alpha Input
    *input = 0x0001; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGB565);
    TestEquals(*output, (u32)0x0000);
    return true;
}

static bool RGB5A1_To_RGBA4(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    // Test Red Input
    *input = 0xF800; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGBA4);
    TestEquals(*output, (u32)0xF000);

    // Test Green Input
    *input = 0x07C0; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGBA4);
    TestEquals(*output, (u32)0x0F00);

    // Test Blue Input
    *input = 0x003E; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGBA4);
    TestEquals(*output, (u32)0x00F0);

    // Test Alpha Input
    *input = 0x0001; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGB5A1 | OUT_RGBA4);
    TestEquals(*output, (u32)0x000F);
    return true;
}

static bool RGBA8_To_RGBA8_Scaled_Blending(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | HORIZONTAL_DOWNSCALE);
    TestEquals(*output, (u32)0x7F000000);

    // It doesn't average the pixels when the bit 24 isn't enabled
    *input = 0xFF000000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8);
    TestEquals(*output, (u32)0xFF000000);

    *input = 0xFFFF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | HORIZONTAL_DOWNSCALE);
    TestEquals(*output, (u32)0x7F7F0000);

    *input = 0xFFFF0000; //Input
    input[1] = 0xFF000000;
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | HORIZONTAL_DOWNSCALE);
    input[1] = 0;
    TestEquals(*output, (u32)0xFF7F0000);

    *input = 0xFFFF0000; //Input
    input[1] = 0xFF0000FF;
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | HORIZONTAL_DOWNSCALE);
    input[1] = 0;
    TestEquals(*output, (u32)0xFF7F007F);

    *input = 0xFFFF0000; //Input
    input[1] = 0x00FF0000; //Input
    input[2] = 0xFF000000;
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | HORIZONTAL_DOWNSCALE);
    input[1] = 0;
    input[2] = 0;
    TestEquals(*output, (u32)0x7FFF0000);
    TestEquals(output[0x40], (u32)0x7F000000);
    output[0x40] = 0;

    // Double downscale downscales the input in both directions (horizontal and vertical)
    *input = 0xFFFF0000; //Input
    input[1] = 0xFF0000FF;
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | DOUBLE_DOWNSCALE);
    input[1] = 0;
    TestEquals(*output, (u32)0x7F3F003F); // Blends 4 adjacent pixels together
    return true;
}

static bool RGBA8_To_RGB8_Different_Sizes(u32*, u32*) {
    u32* input = (u32*)linearAlloc(0x200 * 0x190 * 4);
    u32* output = (u32*)linearAlloc(0xF0 * 0x190 * 4);
    SCOPE_EXIT({
        linearFree(input);
        linearFree(output);
    });
    memset(input, 0, 0x200 * 0x190 * 4);
    memset(output, 0, 0xF0 * 0x190 * 4);

    //Test Red Input
    *input = 0xFF000000; //Input
    input[0x200 * 0x190 - 1] = 0x0000FF00;
    input[0xF0 * 0x190 - 1] = 0x00FF0000;
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x200, 0x190), Dimensions(0xF0, 0x190), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x00FF0000u);
    // Test that it doesn't do any sort of stretching.
    TestEquals(output[0xF0 * 0x190 - 1], (u32)0);
    TestEquals(output[0x1193F], (u32)0x00FF0000u); // Why here?

    //Test Green Input
    *input = 0x00FF0000; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x200, 0x190), Dimensions(0xF0, 0x190), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x0000FF00u);

    //Test Blue Input
    *input = 0x0000FF00; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x200, 0x190), Dimensions(0xF0, 0x190), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x000000FFu);

    //Test Alpha Input
    *input = 0x000000FF; //Input
    *output = 0; //Output
    DisplayTransferAndWait(input, output, Dimensions(0x200, 0x190), Dimensions(0xF0, 0x190), IN_RGBA8 | OUT_RGB8);
    TestEquals(*output, (u32)0x00000000u);
    return true;
}

static bool Test_ZCurve(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[1] = 0xABCDE;
    input[2] = 0xDEF00;
    input[13] = 0xAAAAAA;
    *output = 0;
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8);
    TestEquals(output[0 * 0x80 + 1], (u32)0xABCDEu);
    TestEquals(output[1 * 0x80 + 0], (u32)0xDEF00u);
    TestEquals(output[2 * 0x80 + 3], (u32)0xAAAAAA);

    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[0 * 0x80 + 1] = 0xABCDE;
    input[1 * 0x80 + 0] = 0xDEF00;
    input[2 * 0x80 + 3] = 0xAAAAAA;
    *output = 0;
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | LINEAR_TO_TILED);
    TestEquals(output[1], (u32)0xABCDEu);
    TestEquals(output[2], (u32)0xDEF00u);
    TestEquals(output[13], (u32)0xAAAAAA);
    return true;
}

static bool Test_Flags_Bit_16(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[1] = 0xABCDE;
    input[2] = 0xDEF00;
    input[13] = 0xAAAAAA;
    *output = 0;
    // Test bit 16, seems to do nothing?
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | UNKNOWN1);
    TestEquals(output[0 * 0x80 + 1], (u32)0xABCDEu);
    TestEquals(output[1 * 0x80 + 0], (u32)0xDEF00u);
    TestEquals(output[2 * 0x80 + 3], (u32)0xAAAAAA);
    return true;
}

static bool Test_Flags_Bit_5(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[1] = 0xABCDE;
    input[2] = 0xDEF00;
    input[13] = 0xAAAAAA;
    // Bit 5 leaves the output in the same swizzle order as the input
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | NO_SWIZZLE);
    
    TestEquals(output[1], (u32)0xABCDEu);
    TestEquals(output[2], (u32)0xDEF00u);
    TestEquals(output[13], (u32)0xAAAAAA);
    
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[1] = 0xABCDE;
    input[2] = 0xDEF00;
    input[13] = 0xAAAAAA;
    // Bit 5 leaves the output in the same swizzle order as the input
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGBA8 | NO_SWIZZLE | LINEAR_TO_TILED);
    
    TestEquals(output[1], (u32)0xABCDEu);
    TestEquals(output[2], (u32)0xDEF00u);
    TestEquals(output[13], (u32)0xAAAAAA);
    return true;
}

static bool Test_Raw_Copy(u32* input, u32* output) {
    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[1] = 0xABCDE;
    input[2] = 0xDEF00;
    input[13] = 0xAAAAAA;
    *output = 0;
    // Test that it doesn't do any kind of conversion
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB8 | RAW_COPY);
    TestEquals(output[1], (u32)0xABCDEu);
    TestEquals(output[2], (u32)0xDEF00u);
    TestEquals(output[13], (u32)0xAAAAAA);

    memset(output, 0, 0x4000 * 4);
    memset(input, 0, 0x4000 * 4);

    input[1] = 0xABCDE;
    input[2] = 0xDEF00;
    input[13] = 0xAAAAAA;
    *output = 0;
    // Test that it doesn't do linear->tiled transfer either
    DisplayTransferAndWait(input, output, Dimensions(0x80, 0x80), Dimensions(0x80, 0x80), IN_RGBA8 | OUT_RGB8 | LINEAR_TO_TILED | RAW_COPY);
    TestEquals(output[1], (u32)0xABCDEu);
    TestEquals(output[2], (u32)0xDEF00u);
    TestEquals(output[13], (u32)0xAAAAAA);
    return true;
}

void TestAll() {
    const std::string tag = "DisplayTransfer";

    u32* input = (u32*)linearAlloc(0x4000 * 4);
    u32* output = (u32*)linearAlloc(0x4000 * 4);

    // RGBA8 -> X tests
    Test(tag, "RGBA8_To_RGBA8", RGBA8_To_RGBA8(input, output), true);
    Test(tag, "RGBA8_To_RGB8", RGBA8_To_RGB8(input, output), true);
    Test(tag, "RGBA8_To_RGB565", RGBA8_To_RGB565(input, output), true);
    Test(tag, "RGBA8_To_RGB5A1", RGBA8_To_RGB5A1(input, output), true);
    Test(tag, "RGBA8_To_RGBA4", RGBA8_To_RGBA4(input, output), true);
    // RGB8 -> X tests
    Test(tag, "RGB8_To_RGB8", RGB8_To_RGB8(input, output), true);
    //Test(tag, "RGB8_To_RGB565", RGB8_To_RGB565(input, output), true); Freezes
    //Test(tag, "RGB8_To_RGB5A1", RGB8_To_RGB5A1(input, output), true); Freezes
    // Test(tag, "RGB8_To_RGBA4", RGB8_To_RGBA4(input, output), true);
    // RGB565 -> X tests
    Test(tag, "RGB5A1_To_RGB565", RGB5A1_To_RGB565(input, output), true);
    Test(tag, "RGB5A1_To_RGB5A1", RGB5A1_To_RGB5A1(input, output), true);
    Test(tag, "RGB5A1_To_RGBA4", RGB5A1_To_RGBA4(input, output), true);
    // RGBA4 -> X tests
    Test(tag, "RGBA4_To_RGB5A1", RGBA4_To_RGB5A1(input, output), true);
    // Misc tests
    Test(tag, "RGBA8_To_RGBA8_Scaled_Blending", RGBA8_To_RGBA8_Scaled_Blending(input, output), true);
    Test(tag, "RGBA8_To_RGB8_Different_Sizes", RGBA8_To_RGB8_Different_Sizes(input, output), true);
    Test(tag, "Test_ZCurve", Test_ZCurve(input, output), true);
    Test(tag, "Test_Raw_Copy", Test_Raw_Copy(input, output), true);
    Test(tag, "Test_Flags_Bit_16", Test_Flags_Bit_16(input, output), true);
    Test(tag, "Test_Flags_Bit_5", Test_Flags_Bit_5(input, output), true);

    linearFree(input);
    linearFree(output);
}

}
}
