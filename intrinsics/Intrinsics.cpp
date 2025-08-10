#include <cstdint>
#include <iostream>
#include <map>

typedef uint64_t addr_t;

struct State;
using Memory = std::map<uint64_t, uint8_t>;

extern "C" {
Memory *__lifter_init_memory() {
    Memory *memory = new Memory;
    return memory;
}

void __lifter_free_memory(Memory *memory) { delete memory; }

uint32_t __remill_read_memory_32(Memory *memory, addr_t addr) {
    uint32_t result = 0;

    for (size_t offset = 0; offset < 4; offset++) {
        auto byteIt = memory->find(addr + offset);

        if (byteIt == memory->end()) {
            return 0;
        }

        uint32_t byte = (uint32_t)(*byteIt).second;

        size_t shift = (3 - offset) * 8;
        uint32_t shifted = byte << shift;
        result |= shifted;
    }

    return result;
}

uint64_t __remill_read_memory_64(Memory *memory, addr_t addr) {
    uint64_t result = 0;

    for (size_t offset = 0; offset < 8; offset++) {
        auto byteIt = memory->find(addr + offset);

        if (byteIt == memory->end()) {
            return 0;
        }

        uint64_t byte = (uint64_t)(*byteIt).second;

        size_t shift = (7 - offset) * 8;
        uint64_t shifted = byte << shift;
        result |= shifted;
    }

    return result;
}

Memory *__remill_write_memory_32(Memory *memory, addr_t addr, uint32_t val) {
    for (size_t offset = 0; offset < 4; offset++) {
        size_t shift = (3 - offset) * 8;
        uint32_t shifted = val >> shift;

        (*memory)[addr + offset] = (uint8_t)(shifted & 0xff);
    }

    return memory;
}

Memory *__remill_write_memory_64(Memory *memory, addr_t addr, uint64_t val) {
    for (size_t offset = 0; offset < 8; offset++) {
        size_t shift = (7 - offset) * 8;
        uint64_t shifted = val >> shift;

        (*memory)[addr + offset] = (uint8_t)(shifted & 0xff);
    }

    return memory;
}

bool __remill_flag_computation_sign(bool result, ...) { return result; }

bool __remill_flag_computation_zero(bool result, ...) { return result; }

bool __remill_flag_computation_carry(bool result, ...) { return result; }

bool __remill_flag_computation_overflow(bool result, ...) { return result; }

bool __remill_compare_neq(bool result) { return result; }

bool __remill_compare_uge(bool result) { return result; }

bool __remill_compare_sle(bool result) { return result; }

bool __remill_compare_sgt(bool result) { return result; }

bool __remill_compare_eq(bool result) { return result; }

uint8_t __remill_undefined_8(void) { return 0; }

Memory *__remill_error(State &state, addr_t addr, Memory *memory) {
    return memory;
}

Memory *__remill_function_call(State &state, addr_t addr, Memory *memory) {
    return memory;
}

Memory *__remill_function_return(State &state, addr_t addr, Memory *memory) {
    return memory;
}
}
