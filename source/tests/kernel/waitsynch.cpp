// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>
#include <limits>
#include <string>
#include <cstring>
#include <stdio.h>
#include <3ds.h>

#include "output.h"
#include "common/string_funcs.h"
#include "common/scope_exit.h"
#include "tests/test.h"

namespace Kernel {
namespace WaitSynch {

const Result ERR_TIMEOUT = 0x09401BFE;
const Result ERR_INVALID_HANDLE = 0xD8E007F7;
const Result ERR_ARG_INVALID_RANGE = 0xE0E01BFD;
const Result ERR_ARG_INVALID_POINTER = 0xD8E007F6;

static Handle events[4];
static Handle threads[13];
static Handle mutexes[4];
static Handle semaphore;

static u32 thread0_stack[0x400];
static u32 thread1_stack[0x400];
static u32 thread2_stack[0x400];
static u32 thread3_stack[0x400];
static u32 thread4_stack[0x400];
static u32 thread5_stack[0x400];
static u32 thread6_stack[0x400];
static u32 thread7_stack[0x400];
static u32 thread8_stack[0x400];
static u32 thread9_stack[0x400];
static u32 thread10_stack[0x400];
static u32 thread11_stack[0x400];
static u32 thread12_stack[0x400];

void thread0_handler(void*) {
    svcSignalEvent(events[0]);
    svcExitThread();
}

void thread1_handler(void*) {
    svcSignalEvent(events[1]);
    svcExitThread();
}

void thread2_handler(void*) {
    svcSignalEvent(events[2]);
    svcExitThread();
}

void thread3_handler(void*) {
    svcSignalEvent(events[3]);
    svcExitThread();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#define ONE_SECOND (1000 * 1000 * 1000)

// Test WaitSynchN with wait_all=0, wait=0

bool Test_WaitSynchN_00() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    svcSignalEvent(events[2]);
    res = svcWaitSynchronizationN(&output, events, 4, 0, U64_MAX);
    SoftAssert(output == 2);
    SoftAssert(res == 0);

    svcSignalEvent(events[0]);
    res = svcWaitSynchronizationN(&output, events, 4, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with wait_all=0, wait=1

bool Test_WaitSynchN_01() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    svcCreateThread(&threads[3], thread3_handler, 0x0, (u32*)(&thread3_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[3]); });

    res = svcWaitSynchronizationN(&output, events, 4, 0, U64_MAX);
    SoftAssert(output == 3);
    SoftAssert(res == 0);

    svcClearEvent(events[3]);

    svcCreateThread(&threads[0], thread0_handler, 0x0, (u32*)(&thread0_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[0]); });

    res = svcWaitSynchronizationN(&output, events, 4, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[3], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[0], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with wait_all=1, wait=0

bool Test_WaitSynchN_02() {
    s32 output = -1;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    svcSignalEvent(events[0]);
    svcSignalEvent(events[1]);
    svcSignalEvent(events[2]);
    svcSignalEvent(events[3]);

    // TODO: Figure out how output is set
    res = svcWaitSynchronizationN(&output, events, 4, 1, U64_MAX);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with wait_all=1, wait=1

bool Test_WaitSynchN_03() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    svcCreateThread(&threads[0], thread0_handler, 0x0, (u32*)(&thread0_stack[0x400]), 0x2F, 0xfffffffe);
    svcCreateThread(&threads[1], thread1_handler, 0x0, (u32*)(&thread1_stack[0x400]), 0x30, 0xfffffffe);
    svcCreateThread(&threads[2], thread2_handler, 0x0, (u32*)(&thread2_stack[0x400]), 0x31, 0xfffffffe);
    svcCreateThread(&threads[3], thread3_handler, 0x0, (u32*)(&thread3_stack[0x400]), 0x32, 0xfffffffe);

    SCOPE_EXIT({ svcCloseHandle(threads[0]); });
    SCOPE_EXIT({ svcCloseHandle(threads[1]); });
    SCOPE_EXIT({ svcCloseHandle(threads[2]); });
    SCOPE_EXIT({ svcCloseHandle(threads[3]); });

    res = svcWaitSynchronizationN(&output, threads, 4, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with wait_all=0, wait=1, timeout=1/2 sec

bool Test_WaitSynchN_04() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    res = svcWaitSynchronizationN(&output, events, 4, 0, ONE_SECOND / 2); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    return true;
}

// Test WaitSynchN with wait_all=1, wait=1, timeout=1/2 sec

bool Test_WaitSynchN_05() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    res = svcWaitSynchronizationN(&output, events, 4, 1, ONE_SECOND / 2); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    return true;
}

// Test WaitSynchN with invalid handle

bool Test_WaitSynchN_06() {
    s32 output = 0;
    s32 res = 0;

    res = svcWaitSynchronizationN(&output, &threads[4], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == ERR_INVALID_HANDLE);

    return true;
}

// Test WaitSynchN with duplicate handles (two events signaled, no wait)

bool Test_WaitSynchN_07() {
    s32 output = 0;
    s32 res = 0;

    Handle handles[] = {events[0], events[1], events[1], events[0], events[1], events[2]};

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);
    svcClearEvent(events[3]);

    svcSignalEvent(events[1]);
    svcSignalEvent(events[2]);

    res = svcWaitSynchronizationN(&output, handles, 6, 0, U64_MAX);
    SoftAssert(output == 1);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with duplicate handles (one event signaled, no wait)

bool Test_WaitSynchN_08() {
    s32 output = 0;
    s32 res = 0;
    Handle handles[] = {events[0], events[1], events[1], events[0], events[1]};

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, handles, 5, 0, U64_MAX);
    SoftAssert(output == 1);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with duplicate handles (one event signaled, with a wait)

bool Test_WaitSynchN_09() {
    s32 output = 0;
    s32 res = 0;
    Handle handles[] = {events[0], events[1], events[1], events[0], events[1]};

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[1], thread1_handler, 0x0, (u32*)(&thread1_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[1]); });

    res = svcWaitSynchronizationN(&output, handles, 5, 0, U64_MAX);
    SoftAssert(output == 4);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[1], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with svcDuplicateHandle (one event signaled, with wait)

bool Test_WaitSynchN_10() {
    s32 output = 0;
    s32 res = 0;
    Handle duplicate0, duplicate1;

    svcDuplicateHandle(&duplicate0, events[1]);
    svcDuplicateHandle(&duplicate1, events[1]);

    SCOPE_EXIT({ svcCloseHandle(duplicate0); });
    SCOPE_EXIT({ svcCloseHandle(duplicate1); });

    Handle handles[] = {events[0], duplicate0, events[1], events[0], duplicate1};

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[1], thread1_handler, 0x0, (u32*)(&thread1_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[1]); });

    res = svcWaitSynchronizationN(&output, handles, 5, 0, U64_MAX);
    SoftAssert(output == 4);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[1], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with svcDuplicateHandle (one event signaled, with wait)

bool Test_WaitSynchN_11() {
    s32 output = 0;
    s32 res = 0;
    Handle duplicate0, duplicate1;

    svcDuplicateHandle(&duplicate0, events[1]);
    svcDuplicateHandle(&duplicate1, events[1]);

    SCOPE_EXIT({ svcCloseHandle(duplicate0); });
    SCOPE_EXIT({ svcCloseHandle(duplicate1); });

    Handle handles[] = {events[0], duplicate0, events[1], events[0], duplicate1};

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, handles, 5, 0, U64_MAX);
    SoftAssert(output == 1);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with zero handles, wait_all=false

bool Test_WaitSynchN_12() {
    s32 output = 0;
    s32 res = 0;

    // Should hang, hence the timeout
    res = svcWaitSynchronizationN(&output, events, 0, 0, ONE_SECOND / 2);
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    return true;
}

// Test WaitSynchN with zero handles, wait_all=true

bool Test_WaitSynchN_13() {
    s32 output = 0;
    s32 res = 0;

    // Should pass right through
    res = svcWaitSynchronizationN(&output, events, 0, 1, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with invalid handle count

bool Test_WaitSynchN_14() {
    s32 output = 0;
    s32 res = 0;

    res = svcWaitSynchronizationN(&output, events, -1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == ERR_ARG_INVALID_RANGE);

    return true;
}

// Test WaitSynchN with wait_all=true; 2 events; signal one of them in another thread, clear that
// same event in that same thread, then signal the other one

void thread4_handler(void*)
{
    svcSignalEvent(events[0]);
    svcClearEvent(events[0]);
    svcSignalEvent(events[1]);
    svcExitThread();
}

bool Test_WaitSynchN_15() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[4], thread4_handler, 0x0, (u32*)(&thread4_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[4]); });

    // Should timeout
    res = svcWaitSynchronizationN(&output, events, 2, 1, ONE_SECOND / 2);
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    res = svcWaitSynchronizationN(&output, &threads[4], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynchN with invalid handles pointer

bool Test_WaitSynchN_16() {
    s32 output = 0;
    s32 res = 0;

    res = svcWaitSynchronizationN(&output, nullptr, 0, 0, ONE_SECOND / 2);
    //SoftAssert(output == 0); // TODO: Figure out how output is set, same as Test_WaitSynchN_02
    SoftAssert(res == ERR_ARG_INVALID_POINTER);

    res = svcWaitSynchronizationN(&output, nullptr, 0, 1, ONE_SECOND / 2);
    SoftAssert(output == 0);
    SoftAssert(res == ERR_ARG_INVALID_POINTER);

    res = svcWaitSynchronizationN(&output, nullptr, 1, 0, ONE_SECOND / 2);
    SoftAssert(output == 0);
    SoftAssert(res == ERR_ARG_INVALID_POINTER);

    res = svcWaitSynchronizationN(&output, nullptr, 1, 1, ONE_SECOND / 2);
    SoftAssert(output == 0);
    SoftAssert(res == ERR_ARG_INVALID_POINTER);

    return true;
}

// WaitSynchN wait_all=false; unlocked mutex0, unlocked mutex1; Make sure only mutex0 is locked and
// it does not wait

void thread5_handler(void*) {
    s32 output = 0;
    svcWaitSynchronizationN(&output, mutexes, 2, 0, U64_MAX);
    svcSignalEvent(events[0]);
    svcWaitSynchronization(events[1], U64_MAX);
    svcExitThread();
}

bool Test_WaitSynchN_17() {
    s32 output = 0;
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);
    svcReleaseMutex(mutexes[1]);

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[5], thread5_handler, 0x0, (u32*)(&thread5_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[5]); });

    svcWaitSynchronization(events[0], U64_MAX);

    // Make sure we cant get mutex 0
    res = svcWaitSynchronizationN(&output, mutexes, 1, 0, ONE_SECOND / 2);
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    // Make sure we do get mutex 1
    res = svcWaitSynchronizationN(&output, mutexes, 2, 0, U64_MAX);
    SoftAssert(output == 1);
    SoftAssert(res == 0);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[5], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// WaitSynchN tests that the thread does not wait on a mutex if it already acquired it as well as
// the recursive nature of acquiring a mutex

void thread5b_handler(void*) {
    svcWaitSynchronization(mutexes[0], U64_MAX);
    svcExitThread();
}

bool Test_WaitSynchN_18() {
    s32 output = 0;
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);
    svcReleaseMutex(mutexes[1]);

    res = svcWaitSynchronizationN(&output, mutexes, 2, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, mutexes, 2, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, mutexes, 2, 1, U64_MAX);
    // TODO: Figure out how output is set
    SoftAssert(res == 0);

    svcReleaseMutex(mutexes[0]);
    svcReleaseMutex(mutexes[0]);

    svcReleaseMutex(mutexes[1]);
    svcReleaseMutex(mutexes[1]);

    res = svcReleaseMutex(mutexes[0]);
    SoftAssert(res == 0);

    svcCreateThread(&threads[5], thread5b_handler, 0x0, (u32*)(&thread5_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[5]); });

    res = svcWaitSynchronization(mutexes[0], U64_MAX);
    SoftAssert(res == 0);

    res = svcWaitSynchronization(mutexes[0], U64_MAX);
    SoftAssert(res == 0);

    res = svcReleaseMutex(mutexes[0]);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[5], 1, 0, ONE_SECOND/2); // Wait for all threads to complete
    SoftAssert(res == ERR_TIMEOUT);

    res = svcReleaseMutex(mutexes[0]);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[5], 1, 0, U64_MAX); // Wait for all threads to complete
    SoftAssert(res == 0);

    return true;
}

// WaitSynchN wait_all=true; unlocked mutex0, unlocked mutex1; Make sure both mutexes are locked and
// it does not wait

void thread6_handler(void*) {
    s32 output = 0;

    svcWaitSynchronizationN(&output, mutexes, 2, 1, U64_MAX);

    svcSignalEvent(events[0]);

    svcWaitSynchronization(events[1], U64_MAX);

    svcExitThread();
}

bool Test_WaitSynchN_19() {
    s32 output = 0;
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);
    svcReleaseMutex(mutexes[1]);

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[6], thread6_handler, 0x0, (u32*)(&thread6_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[6]); });

    svcWaitSynchronization(events[0], U64_MAX);

    // Make sure we cant get either mutex
    res = svcWaitSynchronizationN(&output, mutexes, 2, 1, ONE_SECOND / 2);
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[6], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// WaitSynchN wait_all=false; signaled event0, unlocked mutex0; Make sure mutex0 does not lock and
// it does not wait

void thread7_handler(void*) {
    s32 output = 0;
    Handle handles[] = { events[2], mutexes[0] };

    svcWaitSynchronizationN(&output, handles, 2, 0, U64_MAX);
    svcSignalEvent(events[0]);
    svcWaitSynchronization(events[1], U64_MAX);

    svcExitThread();
}

bool Test_WaitSynchN_20() {
    s32 output = 0;
    s32 res = 0;
    Handle handles[] = { events[2], mutexes[0] };

    svcReleaseMutex(mutexes[0]);
    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcSignalEvent(events[2]);

    svcCreateThread(&threads[7], thread7_handler, 0x0, (u32*)(&thread7_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[7]); });

    svcWaitSynchronization(events[0], U64_MAX);
    svcClearEvent(events[2]);

    // Make sure we can still get the mutex
    res = svcWaitSynchronizationN(&output, handles, 2, 0, U64_MAX);
    SoftAssert(output == 1);
    SoftAssert(res == 0);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[7], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

void thread8_handler(void*) {
    s32 output = 0;
    Handle handles[] = { events[2], mutexes[0] };

    svcWaitSynchronizationN(&output, handles, 2, 1, U64_MAX);
    svcSignalEvent(events[0]);
    svcWaitSynchronization(events[1], U64_MAX);

    svcExitThread();
}

// WaitSynchN wait_all=true; signaled event0, unlocked mutex0; Make sure mutex0 locks and it does
// not wait

bool Test_WaitSynchN_21() {
    s32 output = 0;
    s32 res = 0;
    Handle handles[] = { events[2], mutexes[0] };

    svcReleaseMutex(mutexes[0]);
    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcSignalEvent(events[2]);

    svcCreateThread(&threads[8], thread8_handler, 0x0, (u32*)(&thread8_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[8]); });

    svcWaitSynchronization(events[0], U64_MAX);
    svcClearEvent(events[2]);

    // Make sure we can't get the mutex
    res = svcWaitSynchronizationN(&output, handles, 2, 0, ONE_SECOND/2);
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[8], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// WaitSynchN with wait_all=true, 2 objects, 1 ready, 1 not, set the first one not and the second
// one ready in a thread

void thread9_handler(void*) {
    svcSignalEvent(events[0]);
    svcClearEvent(events[1]);
    svcExitThread();
}

bool Test_WaitSynchN_22() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);
    svcSignalEvent(events[1]);

    svcCreateThread(&threads[9], thread9_handler, 0x0, (u32*)(&thread9_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[9]); });

    // Make sure we don't timeout
    res = svcWaitSynchronizationN(&output, events, 2, 1, ONE_SECOND/2);
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    res = svcWaitSynchronizationN(&output, &threads[9], 1, 1, U64_MAX); // Wait for all threads to complete

    // NOTE: We won't pass this test correctly until we improve our scheduling. On real HW, the
    // thread will have already finished, and this will be 0. On Citra, the thread finishes on this
    // Wait, and the result is -1
    //SoftAssert(output == 0);

    SoftAssert(res == 0);

    return true;
}

// Call WaitSynchN with a service handle

bool Test_WaitSynchN_23() {
    s32 output = 0;
    s32 res = 0;

    Handle service = 0;
    SoftAssert(srvGetServiceHandle(&service, "APT:U") == 0);
    SoftAssert(service != 0);
    SCOPE_EXIT({ svcCloseHandle(service); });

    // Make sure we don't timeout
    res = svcWaitSynchronizationN(&output, &service, 1, 0, ONE_SECOND/2);
    SoftAssert(output == -1);
    SoftAssert(res == ERR_TIMEOUT);

    return true;
}

// WaitSynchN with an invalid handle passed in, nothing should wait

bool Test_WaitSynchN_24() {
    s32 output = 0;
    s32 res = 0;
    Handle handles[] = { events[0], 0xDEADBEEF };

    svcSignalEvent(events[0]);

    res = svcWaitSynchronizationN(&output, handles, 2, 0, U64_MAX);
    SoftAssert(res == ERR_INVALID_HANDLE);

    res = svcWaitSynchronizationN(&output, handles, 2, 1, ONE_SECOND / 2);
    SoftAssert(res == ERR_INVALID_HANDLE);

    return true;
}

// WaitSynchN with an invalid handle passed in, nothing should lock

void thread10_handler(void*) {
    s32 output = 0;

    Handle handles[] = { mutexes[0], 0xDEADBEEF };

    svcWaitSynchronizationN(&output, handles, 2, 0, U64_MAX);

    svcSignalEvent(events[0]);

    svcWaitSynchronization(events[1], U64_MAX);

    svcExitThread();
}

bool Test_WaitSynchN_25() {
    s32 output = 0;
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[10], thread10_handler, 0x0, (u32*)(&thread10_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[10]); });

    svcWaitSynchronization(events[0], U64_MAX);

    // Make sure we can still get mutex 0 due to the invalid handle
    res = svcWaitSynchronizationN(&output, mutexes, 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[10], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynch1 with a timeout and with wait=0

bool Test_WaitSynch1_00() {
    s32 res = 0;

    svcClearEvent(events[0]);

    res = svcWaitSynchronization(events[0], ONE_SECOND / 2);
    SoftAssert(res == ERR_TIMEOUT);

    svcSignalEvent(events[0]);
    res = svcWaitSynchronization(events[0], U64_MAX);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynch1 with wait=1

bool Test_WaitSynch1_01() {
    s32 output = 0;
    s32 res = 0;

    svcClearEvent(events[0]);

    svcCreateThread(&threads[0], thread0_handler, 0x0, (u32*)(&thread0_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[0]); });

    res = svcWaitSynchronization(events[0], U64_MAX);
    SoftAssert(output == 0);

    res = svcWaitSynchronizationN(&output, &threads[0], 1, 0, U64_MAX);
    SoftAssert(output == 0);
    SoftAssert(res == 0);

    return true;
}

// Test WaitSynch1 with an invalid handle

bool Test_WaitSynch1_02() {
    s32 res = svcWaitSynchronization(0xDEADBEEF, U64_MAX);
    SoftAssert(res == ERR_INVALID_HANDLE);
    return true;
}

// Test WaitSynch1 with a service handle

bool Test_WaitSynch1_03() {
    s32 res = 0;
    Handle service = 0;

    srvGetServiceHandle(&service, "soc:U");
    SCOPE_EXIT({ svcCloseHandle(service); });

    res = svcWaitSynchronization(service, ONE_SECOND/2);
    SoftAssert(res == ERR_TIMEOUT);

    return true;
}

// WaitSynch1; unlocked mutex0,  Make sure mutex0 locks

void thread11_handler(void*)
{
    svcWaitSynchronization(mutexes[0], U64_MAX);
    svcSignalEvent(events[0]);
    svcWaitSynchronization(events[1], U64_MAX);
    svcExitThread();
}

bool Test_WaitSynch1_04() {
    s32 output = 0;
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);

    svcCreateThread(&threads[11], thread11_handler, 0x0, (u32*)(&thread11_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[11]); });

    svcWaitSynchronization(events[0], U64_MAX);

    // Make sure we cant get mutex 0
    res = svcWaitSynchronization(mutexes[0], ONE_SECOND / 2);
    SoftAssert(res == ERR_TIMEOUT);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[11], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// WaitSynch1; unlocked mutex0,  Lock mutex0

bool Test_WaitSynch1_05() {
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);

    // Make sure we can get mutex 0
    res = svcWaitSynchronization(mutexes[0], U64_MAX);
    SoftAssert(res == 0);

    return true;
}

// WaitSynch1; wait then get mutex0

void thread12_handler(void*)
{
    svcWaitSynchronization(mutexes[0], U64_MAX);
    svcSignalEvent(events[0]);

    svcSleepThread(ONE_SECOND/2);
    svcReleaseMutex(mutexes[0]);

    svcWaitSynchronization(events[1], U64_MAX);
    svcExitThread();
}

bool Test_WaitSynch1_06() {
    s32 output = 0;
    s32 res = 0;

    svcReleaseMutex(mutexes[0]);

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);

    svcCreateThread(&threads[12], thread12_handler, 0x0, (u32*)(&thread12_stack[0x400]), 0x31, 0xfffffffe);
    SCOPE_EXIT({ svcCloseHandle(threads[12]); });

    svcWaitSynchronization(events[0], U64_MAX);

    // Make sure we cant get mutex 0
    res = svcWaitSynchronization(mutexes[0], U64_MAX);
    SoftAssert(res == 0);

    svcSignalEvent(events[1]);

    res = svcWaitSynchronizationN(&output, &threads[12], 1, 1, U64_MAX); // Wait for all threads to complete
    SoftAssert(output == -1);
    SoftAssert(res == 0);

    return true;
}

// WaitSynch1; semaphore test

bool Test_WaitSynch1_07() {
    s32 res = 0;

    svcClearEvent(events[0]);
    svcClearEvent(events[1]);
    svcClearEvent(events[2]);

    res = svcWaitSynchronization(semaphore, U64_MAX);
    SoftAssert(res == 0);

    res = svcWaitSynchronization(semaphore, U64_MAX);
    SoftAssert(res == 0);

    res = svcWaitSynchronization(semaphore, U64_MAX);
    SoftAssert(res == 0);

    res = svcWaitSynchronization(semaphore, ONE_SECOND/2);
    SoftAssert(res == ERR_TIMEOUT);

    return true;
}

// Test WaitSynch with all Kernel Object types

bool Test_WaitSynch_KernelType_SharedMem() {
    s32 output;
    s32 res;
    Handle service = 0;

    // Get an example of a session handle
    SoftAssert(srvGetServiceHandle(&service, "hid:USER") == 0);
    SoftAssert(service != 0);
    SCOPE_EXIT({ svcCloseHandle(service); });

    // Get an example of a shared_mem handle
    u32* cmdbuf=getThreadCommandBuffer();
    cmdbuf[0] = 0xa0000;
    svcSendSyncRequest(service);
    Handle shared_mem = cmdbuf[3];
    SoftAssert(shared_mem != 0);

    // Test WaitSynch with a shared mem handle
    res = svcWaitSynchronization(shared_mem, ONE_SECOND/2);
    SoftAssert(res == ERR_INVALID_HANDLE);

    res = svcWaitSynchronizationN(&output, &shared_mem, 1, 0, ONE_SECOND/2);
    SoftAssert(res == ERR_INVALID_HANDLE);

    res = svcWaitSynchronizationN(&output, &shared_mem, 1, 1, ONE_SECOND/2);
    SoftAssert(res == ERR_INVALID_HANDLE);

    return true;
}

// Test WaitSynch with all Kernel Object types

bool Test_WaitSynch_KernelType_Session() {
    Handle service = 0;

    // Get an example of a session handle
    SoftAssert(srvGetServiceHandle(&service, "hid:USER") == 0);
    SoftAssert(service != 0);
    SCOPE_EXIT({ svcCloseHandle(service); });

    // TODO: These asserts aren't correct on a 3DS, figure out how WaitSynch works with a Session
    // kernel object

    // s32 output[3];
    // s32 res[3];

    // Test WaitSynch with a session handle
    // output[0] = 0;
    // res[0] = svcWaitSynchronization(service, ONE_SECOND/2);
    // SoftAssert(res[0] == ERR_TIMEOUT);

    // res[1] = svcWaitSynchronizationN(&output[1], &service, 1, 0, ONE_SECOND/2);
    // SoftAssert(output[1] == -1);
    // SoftAssert(res[1] == ERR_TIMEOUT);

    // res[2] = svcWaitSynchronizationN(&output[2], &service, 1, 1, ONE_SECOND/2);
    // SoftAssert(output[2] == -1);
    // SoftAssert(res[2] == 0); // Wrong in citra

    s32 output = 0;
    svcWaitSynchronizationN(&output, &service, 1, 1, ONE_SECOND / 2);

    return true;
}

// Test WaitSynch with an AddressArbiter

bool Test_WaitSynch_KernelType_AddressArbiter() {
    s32 output;
    s32 res;
    Handle arb = 0;

    // TODO(bunnei): Replace with svcCreateAddressArbiter once libctru implements it
    asm volatile ("svc 0x21\n"
                  "MOV %[out], r1" : [out] "=r"(arb));

    SoftAssert(arb != 0);
    SCOPE_EXIT({ svcCloseHandle(arb); });

    res = svcWaitSynchronization(arb, ONE_SECOND/2);
    SoftAssert(res == ERR_INVALID_HANDLE);

    // TODO(bunnei): Verify output
    res = svcWaitSynchronizationN(&output, &arb, 1, 0, ONE_SECOND/2);
    SoftAssert(res == ERR_INVALID_HANDLE);
    res = svcWaitSynchronizationN(&output, &arb, 1, 1, ONE_SECOND/2);
    SoftAssert(res == ERR_INVALID_HANDLE);

    return true;
}

void TestAll() {
    svcCreateEvent(&events[0], RESET_STICKY);
    svcCreateEvent(&events[1], RESET_STICKY);
    svcCreateEvent(&events[2], RESET_STICKY);
    svcCreateEvent(&events[3], RESET_STICKY);

    svcCreateMutex(&mutexes[0], false);
    svcCreateMutex(&mutexes[1], false);

    svcCreateSemaphore(&semaphore, 3, 10);

    Test("Test_WaitSynchN_00", "Test_WaitSynchN_00", Test_WaitSynchN_00(), true);
    Test("Test_WaitSynchN_01", "Test_WaitSynchN_01", Test_WaitSynchN_01(), true);
    Test("Test_WaitSynchN_02", "Test_WaitSynchN_02", Test_WaitSynchN_02(), true);
    Test("Test_WaitSynchN_03", "Test_WaitSynchN_03", Test_WaitSynchN_03(), true);
    Test("Test_WaitSynchN_04", "Test_WaitSynchN_04", Test_WaitSynchN_04(), true);
    Test("Test_WaitSynchN_05", "Test_WaitSynchN_05", Test_WaitSynchN_05(), true);
    Test("Test_WaitSynchN_06", "Test_WaitSynchN_06", Test_WaitSynchN_06(), true);
    Test("Test_WaitSynchN_07", "Test_WaitSynchN_07", Test_WaitSynchN_07(), true);
    Test("Test_WaitSynchN_08", "Test_WaitSynchN_08", Test_WaitSynchN_08(), true);
    Test("Test_WaitSynchN_09", "Test_WaitSynchN_09", Test_WaitSynchN_09(), true);
    Test("Test_WaitSynchN_10", "Test_WaitSynchN_10", Test_WaitSynchN_10(), true);
    Test("Test_WaitSynchN_11", "Test_WaitSynchN_11", Test_WaitSynchN_11(), true);
    Test("Test_WaitSynchN_12", "Test_WaitSynchN_12", Test_WaitSynchN_12(), true);
    Test("Test_WaitSynchN_13", "Test_WaitSynchN_13", Test_WaitSynchN_13(), true);
    Test("Test_WaitSynchN_14", "Test_WaitSynchN_14", Test_WaitSynchN_14(), true);
    Test("Test_WaitSynchN_15", "Test_WaitSynchN_15", Test_WaitSynchN_15(), true);
    Test("Test_WaitSynchN_16", "Test_WaitSynchN_16", Test_WaitSynchN_16(), true);
    Test("Test_WaitSynchN_17", "Test_WaitSynchN_17", Test_WaitSynchN_17(), true);
    Test("Test_WaitSynchN_18", "Test_WaitSynchN_18", Test_WaitSynchN_18(), true);
    Test("Test_WaitSynchN_19", "Test_WaitSynchN_19", Test_WaitSynchN_19(), true);
    Test("Test_WaitSynchN_20", "Test_WaitSynchN_20", Test_WaitSynchN_20(), true);
    Test("Test_WaitSynchN_21", "Test_WaitSynchN_21", Test_WaitSynchN_21(), true);
    Test("Test_WaitSynchN_22", "Test_WaitSynchN_22", Test_WaitSynchN_22(), true);
    Test("Test_WaitSynchN_23", "Test_WaitSynchN_23", Test_WaitSynchN_23(), true);
    Test("Test_WaitSynchN_24", "Test_WaitSynchN_24", Test_WaitSynchN_24(), true);
    Test("Test_WaitSynchN_25", "Test_WaitSynchN_25", Test_WaitSynchN_25(), true);
    Test("Test_WaitSynch1_00", "Test_WaitSynch1_00", Test_WaitSynch1_00(), true);
    Test("Test_WaitSynch1_01", "Test_WaitSynch1_01", Test_WaitSynch1_01(), true);
    Test("Test_WaitSynch1_02", "Test_WaitSynch1_02", Test_WaitSynch1_02(), true);
    Test("Test_WaitSynch1_03", "Test_WaitSynch1_03", Test_WaitSynch1_03(), true);
    Test("Test_WaitSynch1_04", "Test_WaitSynch1_04", Test_WaitSynch1_04(), true);
    Test("Test_WaitSynch1_05", "Test_WaitSynch1_05", Test_WaitSynch1_05(), true);
    Test("Test_WaitSynch1_06", "Test_WaitSynch1_06", Test_WaitSynch1_06(), true);
    Test("Test_WaitSynch1_07", "Test_WaitSynch1_07", Test_WaitSynch1_07(), true);
    Test("Test_WaitSynch_KernelType_Session", "Test_WaitSynch_KernelType_Session", Test_WaitSynch_KernelType_Session(), true);
    Test("Test_WaitSynch_KernelType_SharedMem", "Test_WaitSynch_KernelType_SharedMem", Test_WaitSynch_KernelType_SharedMem(), true);
    Test("Test_WaitSynch_KernelType_AddressArbiter", "Test_WaitSynch_KernelType_AddressArbiter", Test_WaitSynch_KernelType_AddressArbiter(), true);

    svcCloseHandle(events[0]);
    svcCloseHandle(events[1]);
    svcCloseHandle(events[2]);
    svcCloseHandle(events[3]);

    svcCloseHandle(mutexes[0]);
    svcCloseHandle(mutexes[1]);

    svcCloseHandle(semaphore);
}

} // namespace
} // namespace
