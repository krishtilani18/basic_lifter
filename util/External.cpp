#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

#include <remill/Arch/X86/Runtime/State.h>
#include <util/Memory.hpp>

extern "C" {
LifterMemory *EXTERNAL_free(State *state, addr_t program_counter,
                            LifterMemory *memory) {
    uint64_t addr = state->gpr.rdi.qword;
    size_t size = memory->mallocs[addr];

    memory->mallocs.erase(addr);

    for (int i = 0; i < size; i++) {
        memory->memory.erase(addr + i);
    }

    free((void *)addr);

    return memory;
}

LifterMemory *EXTERNAL_malloc(State *state, addr_t program_counter,
                              LifterMemory *memory) {
    size_t size = state->gpr.rdi.qword;

    // Simulate behaviour from actual C malloc - we just
    // want the address that malloc generates
    // WARNING: allocating memory is *only* done at `malloc`,
    // and freeing memory is *only* done at `free`. Thus, these
    // "linking" functions can also introduce memory leaks.
    void *ptr = malloc(size);

    // Store dynamically-allocated memory info into `memory->mallocs`
    uint64_t addr = (uint64_t)ptr;
    memory->mallocs[addr] = size;
    state->gpr.rax.qword = addr;

    return memory;
}

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
