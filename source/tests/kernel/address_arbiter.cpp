#include <memory>
#include <cstring>
#include <3ds.h>

#include "common/scope_exit.h"
#include "tests/test.h"

namespace Kernel {
namespace AddressArbiter {

static bool Test_ArbitrateAddress() {
    const Result ERR_TIMEOUT = 0x09401BFE;

    Handle arbiter;
    TestEquals(svcCreateAddressArbiter(&arbiter), 0);
    SCOPE_EXIT({ svcCloseHandle(arbiter); });

    s32 arbitration_value = 0;

    // Test ARBITRATION_WAIT_IF_LESS_THAN, should not wait
    TestEquals(svcArbitrateAddress(arbiter, (u32)&arbitration_value, ARBITRATION_WAIT_IF_LESS_THAN, 0, 0), 0);
    TestEquals(arbitration_value, 0);

    // Test ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, this one should _not_ wait, however, it still returns a Timeout error
    TestEquals(svcArbitrateAddress(arbiter, (u32)&arbitration_value, ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, 0, 3000000000), ERR_TIMEOUT);
    TestEquals(arbitration_value, 0);

    // Test ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, this should wait and return a Timeout error
    TestEquals(svcArbitrateAddress(arbiter, (u32)&arbitration_value, ARBITRATION_WAIT_IF_LESS_THAN_TIMEOUT, 1, 3000000000), ERR_TIMEOUT);
    TestEquals(arbitration_value, 0);

    // Test ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN_TIMEOUT, this one should wait and return a Timeout error
    TestEquals(svcArbitrateAddress(arbiter, (u32)&arbitration_value, ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN_TIMEOUT, 1, 3000000000), ERR_TIMEOUT);
    // The value must be modified if the thread was put to sleep
    TestEquals(arbitration_value, -1);

    // Reset the value
    arbitration_value = 0;

    // Test ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN, this one should not wait and return RESULT_SUCCESS
    TestEquals(svcArbitrateAddress(arbiter, (u32)&arbitration_value, ARBITRATION_DECREMENT_AND_WAIT_IF_LESS_THAN, 0, 0), 0);
    // The value must not be modified if the thread was not put to sleep
    TestEquals(arbitration_value, 0);

    return true;
}

void TestAll() {
    Test("Kernel::AddressArbiter", "ArbitrateAddress", Test_ArbitrateAddress(), true);
}

} // namespace
} // namespace
