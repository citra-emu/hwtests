#include <array>
#include <3ds.h>

#include "common/result.h"
#include "output.h"
#include "tests/test.h"
#include "tests/cfg/cfg.h"

namespace Cfg {

    template <u32 Id, u16 Size>
    struct CfgBlock {
        static const u32 id = Id;
        static const u16 size = Size;
    };

    // A valid block
    static const CfgBlock<0x000B0000, 0x4> BLK_COUNTRY_INFO;

    // Invalid due to wrong size
    static const CfgBlock<0x000B0000, 0x8> INVALID_COUNTRY_INFO_SIZE;

    // This block does not have a valid ID
    static const CfgBlock<0x01160500, 0x1> INVALID_ID;

    // Expected errors
    const ResultCode RC_BAD_SIZE =
        ResultCode(ErrorDescription::InvalidSize, ErrorModule::Config, ErrorSummary::WrongArgument, ErrorLevel::Permanent);

    const ResultCode RC_BAD_ID =
        ResultCode(ErrorDescription::NotFound, ErrorModule::Config, ErrorSummary::WrongArgument, ErrorLevel::Permanent);


    static bool TestCfgBlockNotFoundRC() {
        const u32 blkID = INVALID_ID.id;
        const u32 size = INVALID_ID.size;
        std::array<u8, size> outData;

        Result result = CFGU_GetConfigInfoBlk2(size, blkID, &outData[0]);

        if ((u32)result != RC_BAD_ID.raw)
            return false;

        return true;
    }

    static bool TestCfgBlockWrongSizeRC() {
        const u32 blkID = INVALID_COUNTRY_INFO_SIZE.id;
        const u32 size = INVALID_COUNTRY_INFO_SIZE.size;
        std::array<u8, size> outData;

        Result result = CFGU_GetConfigInfoBlk2(size, blkID, &outData[0]);

        if ((u32)result != RC_BAD_SIZE.raw)
            return false;

        return true;
    }

    void TestAll()
    {
        const std::string tag = "Cfg";
        Test(tag, "TestCfgBlockNotFoundRC", TestCfgBlockNotFoundRC(), true);
        Test(tag, "TestCfgBlockWrongSizeRC", TestCfgBlockWrongSizeRC(), true);
        // TODO: Add test for wrong flag
    }

} // namespace
