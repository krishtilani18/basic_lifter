#include <cstdio>
#include <map>

#include <remill/Arch/X86/Runtime/State.h>
#include <util/Memory.hpp>

extern "C" {
int printfs = 0;

// Mocking functionality for `vuln.c`
LifterMemory *EXTERNAL___stack_chk_fail(State *state, addr_t program_counter,
                                        LifterMemory *memory) {
    return memory;
}

LifterMemory *EXTERNAL_printf(State *state, addr_t program_counter,
                              LifterMemory *memory) {
    // Mock print statements as if they had access to global state
    if (printfs < 4) {
        printf(">> ");
    } else if (printfs == 4) {
        // Second argument
        int value = state->gpr.rsi.dword;
        printf("shouldn't u be zero not %d?\n", value);
    }

    printfs++;
    return memory;
}

LifterMemory *EXTERNAL___isoc99_scanf(State *state, addr_t program_counter,
                                      LifterMemory *memory) {
    // Second argument
    int addr = state->gpr.rsi.qword;

    // Fetch data from actual scanf
    int64_t value;
    scanf(" %lld", &value);
    uint64_t unsignedValue = (uint64_t)value;

    // Scanning a long long int, write 8 bytes
    for (size_t offset = 0; offset < 8; offset++) {
        size_t shift = offset * 8;
        uint64_t shifted = unsignedValue >> shift;

        (*memory)[addr + offset] = (uint8_t)(shifted & 0xff);
    }

    return memory;
}
}
