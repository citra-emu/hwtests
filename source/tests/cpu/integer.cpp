#include <limits>
#include <string>
#include "output.h"
#include "common/string_funcs.h"
#include "tests/test.h"

namespace CPU {
namespace Integer {

// ADD variants
static bool Add() {
    unsigned int output = 0;
    unsigned int rm = 0;
    unsigned int rn = 0;
    unsigned int badflags = 0;

    // Generic addition
    asm volatile ("LDR r0, =420\n"
                  "LDR r1, =69\n"
                  "ADD %[out], r0, r1" : [out] "=r"(output));
    if (output != 489)
        return false;

    // ADD overflow (unsigned)
    rm = std::numeric_limits<unsigned int>::max();
    rn = 1;
    asm volatile ("ADD %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    if (output != 0)
        return false;

    // ADDS
    badflags = 0;
    rm = std::numeric_limits<unsigned int>::max() - 1;
    rn = 1;
    asm volatile ("ADDS  %[Rm], %[Rm], %[Rn]\n"
                  "ORRCS %[out], %[out], #1\n"
                  "ORRPL %[out], %[out], #2\n"
                  "ORRVS %[out], %[out], #4\n"
                  "ORREQ %[out], %[out], #8\n"
                  "ADDS  %[Rm], %[Rm], %[Rn]\n"
                  "ORRCC %[out], %[out], #1\n"
                  "ORRMI %[out], %[out], #2\n"
                  "ORRVS %[out], %[out], #4\n"
                  "ORRNE %[out], %[out], #8" : [out] "+r"(badflags), [Rm] "+r"(rm) : [Rn] "r"(rn));
    if (badflags != 0)
        return false;

    // TODO: ADC, ADCS, ADDW variants.

    return true;
}

// SUB variants
static bool Sub() {
    unsigned int output;

    // Generic subtraction.
    asm volatile ("MOV r0, #488\n"
                  "SUB %[out], r0, #69" : [out] "=r"(output));
    if (output != 419)
        return false;

    // Subtract overflow (0 - 1).
    asm volatile ("MOV r0, #0\n"
                  "SUB %[out], r0, #1" : [out] "=r"(output));
    if (output != std::numeric_limits<unsigned int>::max())
        return false;

    // TODO: SUBS, etc.

    return true;
}

// MUL variants
static bool Mul() {
    unsigned int output;

    // Multiply by loaded registers.
    asm volatile ("MOV r0, #20\n"
                  "MOV r1, #21\n"
                  "MUL %[out], r0, r1" : [out] "=r"(output));
    if (output != 420)
        return false;

    // TODO: Other MUL variants.

    return true;
}

// QADD16
static bool Qadd16()
{
    int output = 0;
    int rm = std::numeric_limits<short>::min();
    int rn = std::numeric_limits<short>::min();

    // rm and rn == SHORT_MIN
    asm volatile ("QADD16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == -98304);

    // rm and rn == SHORT_MAX
    rm = std::numeric_limits<short>::max();
    rn = std::numeric_limits<short>::max();
    asm volatile ("QADD16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == std::numeric_limits<short>::max());

    // rm == SHORT_MAX | rn == SHORT_MIN
    rn = std::numeric_limits<short>::min();
    asm volatile ("QADD16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == -1);

    return true;
}

// QSUB16
static bool Qsub16()
{
    int output = 0;
    int rm = std::numeric_limits<short>::min();
    int rn = std::numeric_limits<short>::min();

    // rm and rn == SHORT_MIN
    asm volatile ("QSUB16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 0);

    // rm and rn == SHORT_MAX
    rm = std::numeric_limits<short>::max();
    rn = std::numeric_limits<short>::max();
    asm volatile ("QSUB16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 0);

    // rm == SHORT_MAX | rn == SHORT_MIN
    rn = std::numeric_limits<short>::min();
    asm volatile ("QSUB16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 98303);

    return true;
}

// SASX
static bool Sasx() {
    unsigned int output;
    unsigned int rm = std::numeric_limits<unsigned int>::max();
    unsigned int rn = rm - 4000;

    asm volatile ("SASX %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));

    return (output == 4032692224);
}

// SSAX
static bool Ssax() {
    unsigned int output;
    unsigned int rm = std::numeric_limits<unsigned int>::max();
    unsigned int rn = rm - 4000;

    asm volatile ("SSAX %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));

    return (output == 262209534);
}

// UQSUB8
static bool Uqsub8() {
    unsigned int output;
    unsigned int rm = 70;
    unsigned int rn = 50;

    // Regular subtraction
    asm volatile ("UQSUB8 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 20);

    // Floor subtraction (50 - 70) == 0 with UQSUB8 (or any of the other UQSUB variants).
    rm = 50;
    rn = 70;
    asm volatile ("UQSUB8 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 0);

    return true;
}

// USAD8
static bool Usad8() {
    unsigned int output;
    unsigned int rm = 50;
    unsigned int rn = 10;

    // Regular subtraction
    asm volatile ("USAD8 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 40);

    // Absolute value subtraction (0 - 1) == 1 with USAD8. UCHAR_MAX rollover does not occur.
    rm = 0;
    rn = 1;
    asm volatile ("USAD8 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 1);

    return true;
}

// USADA8
static bool Usada8() {
    unsigned int output;
    unsigned int rm = 50;
    unsigned int rn = 10;
    unsigned int ra = 1;

    // Regular subtraction with accumulator add: abs(50 - 10) + 1
    asm volatile ("USADA8 %[out], %[Rm], %[Rn], %[Ra]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn), [Ra] "r"(ra));
    SoftAssert(output == 41);

    // Absolute value subtraction with accumulator add: abs(0 - 1) + 9
    rm = 0;
    rn = 1;
    ra = 9;
    asm volatile ("USADA8 %[out], %[Rm], %[Rn], %[Ra]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn), [Ra] "r"(ra));
    SoftAssert(output == 10);

    return true;
}

// SXTH
static bool Sxth() {
    unsigned int output = 50;

    // No rotation
    output = 0x0000FFCE;
    asm volatile ("SXTH %[out], %[out]" : [out] "+r"(output));
    SoftAssert(output == (u32)-50);

    // ROR by 8
    output = 0x00FCE000;
    asm volatile ("SXTH %[out], %[out], ROR #8" : [out] "+r"(output));
    SoftAssert(output == (u32)-800);

    // ROR by 16
    output = 0x01410000;
    asm volatile ("SXTH %[out], %[out], ROR #16" : [out] "+r"(output));
    SoftAssert(output == (u32)321);

    // ROR by 24
    output = 0xCE0000FF;
    asm volatile ("SXTH %[out], %[out], ROR #24" : [out] "+r"(output));
    SoftAssert(output == (u32)-50);

    return true;
}

// UXTAB16
static bool Uxtab16() {
    unsigned int output;
    unsigned int rm = std::numeric_limits<unsigned int>::max();
    unsigned int rn = std::numeric_limits<int>::max();

    // No rotation
    asm volatile ("UXTAB16 %[out], %[Rm], %[Rn]" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 16646398);

    // ROR by 8
    asm volatile ("UXTAB16 %[out], %[Rm], %[Rn], ROR #8" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 8257790);

    // ROR by 16
    asm volatile ("UXTAB16 %[out], %[Rm], %[Rn], ROR #16" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 16646398);

    // ROR by 24
    asm volatile ("UXTAB16 %[out], %[Rm], %[Rn], ROR #24" : [out] "=r"(output) : [Rm] "r"(rm), [Rn] "r"(rn));
    SoftAssert(output == 16646270);

    return true;
}

// UXTB16
static bool Uxtb16() {
    unsigned int output = 50;

    // No rotation
    asm volatile ("UXTB16 %[out], %[out]" : [out] "+r"(output));
    SoftAssert(output == 50);

    // ROR by 8
    output = (1 << 16) - 1;
    asm volatile ("UXTB16 %[out], %[out], ROR #8" : [out] "+r"(output));
    SoftAssert(output == 0xFF);

    // ROR by 16
    output = (1 << 24) - 1;
    asm volatile ("UXTB16 %[out], %[out], ROR #16" : [out] "+r"(output));
    SoftAssert(output == 0xFF00FF);

    // ROR by 24
    output = (1 << 24) - 1;
    output /= 2;
    asm volatile ("UXTB16 %[out], %[out], ROR #24" : [out] "+r"(output));
    SoftAssert(output == 0xFF0000);

    return true;
}


void TestAll() {
    const std::string tag = "Integer";

    Test(tag, "ADD", Add(), true);
    Test(tag, "SUB", Sub(), true);
    Test(tag, "MUL", Mul(), true);
    Test(tag, "QADD16", Qadd16(), true);
    Test(tag, "QSUB16", Qsub16(), true);
    Test(tag, "SASX", Sasx(), true);
    Test(tag, "SSAX", Ssax(), true);
    Test(tag, "UQSUB8", Uqsub8(), true);
    Test(tag, "USAD8", Usad8(), true);
    Test(tag, "USADA8", Usada8(), true);
    Test(tag, "SXTH", Sxth(), true);
    Test(tag, "UXTAB16", Uxtab16(), true);
    Test(tag, "UXTB16", Uxtb16(), true);
}

}
}
