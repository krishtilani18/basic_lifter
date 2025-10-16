#include <cstdio>
#include <map>

#include <remill/Arch/X86/Runtime/State.h>
#include <util/Memory.hpp>

extern "C" {
LifterMemory *EXTERNAL_printf(State *state, addr_t program_counter,
                              LifterMemory *memory) {
    // 2nd argument stored in %esi
    int value = state->gpr.rsi.dword;

    // Currently, `.rodata` is not integrated, so use fixed value
    // to demonstrate functionality
    printf("%d\n", value);

    return memory;
}
}
