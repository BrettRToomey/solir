#include "solir.h"

char codeExample[] = {
    0x55, 0x48, 0x89, 0xe5, 0xb8, 0x05, 0x00, 0x00,
    0x00, 0x5d, 0xc3, 0x0f, 0x1f, 0x44, 0x00, 0x00,
    0x55, 0x48, 0x89, 0xe5, 0xb8, 0x0a, 0x00, 0x00,
    0x00, 0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00,
    0x5d, 0xc3
};

int main() {
    struct SolSymbol symbols[] = {
        {.name = "_test", .offset = 0,    .flags = SOL_SYMBOL_FLAG_DEFINED | SOL_SYMBOL_FLAG_EXTERNAL},
        {.name = "_main", .offset = 0x10, .flags = SOL_SYMBOL_FLAG_DEFINED | SOL_SYMBOL_FLAG_EXTERNAL},
    };

    int symbolCount = sizeof(symbols)/sizeof(symbols[0]);

    OutputMachO("test.o", &codeExample[0], sizeof(codeExample), &symbols[0], symbolCount);
}
