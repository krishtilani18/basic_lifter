#include <inttypes.h>
#include <iomanip>
#include <iostream>

#include "./elf.hpp"

int main(int argc, char* argv[]) {
  getTextSections(argv[1]);
}
