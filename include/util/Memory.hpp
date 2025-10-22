#include <cstdint>
#include <map>

struct LifterMemory {
    std::map<uint64_t, uint8_t> memory;
    std::map<uint64_t, size_t> mallocs;
};
