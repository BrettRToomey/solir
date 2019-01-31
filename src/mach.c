#define MachMagic 0xfeedfacf
#define MachCigam 0xcffaedfe

#define MachObject 0x1

#define Mach_CPU_ARCH_ABI64 0x01000000

#define Mach_CPU_TYPE_X86      (7)
#define Mach_CPU_TYPE_X86_64   (Mach_CPU_TYPE_X86 | Mach_CPU_ARCH_ABI64)

#define Mach_CPU_SUBTYPE_X86_ALL ((3) + ((0) << 4))

#define Mach_VM_PROT_ALL (0x7)

enum MachLoadCommandType {
    MachLoadCommandType_Segment_32 = 0x1,
    MachLoadCommandType_SymbolTable = 0x2,
    MachLoadCommandType_DynamicSymbolTable = 0xb,
    MachLoadCommandType_Segment = 0x19
};

struct MachHeader {
    Sol_U32 magic;
    Sol_I32 cputype;
    Sol_I32 cpusubtype;
    Sol_U32 filetype;
    Sol_U32 ncmds;
    Sol_U32 sizeofcmds;
    Sol_U32 flags;
    Sol_U32 reserved;
};

struct MachSegmentCommand {
    Sol_U32    cmd;
    Sol_U32    cmdsize;
    char       segname[16];
    Sol_U64    vmaddr;
    Sol_U64    vmsize;
    Sol_U64    fileoff;
    Sol_U64    filesize;
    Sol_U32    maxprot;
    Sol_U32    initprot;
    Sol_U32    nsects;
    Sol_U32    flags;
};

enum MachSectionType {
    MachSectionType_Regular = 0x0
};

enum MachSectionAttribute {
    MachSectionAttribute_PureInstructions = 0x80000000
};

struct MachSection {
    char       sectname[16];
    char       segname[16];
    Sol_U64    addr;
    Sol_U64    size;
    Sol_U32    offset;
    Sol_U32    align;
    Sol_U32    reloff;
    Sol_U32    nreloc;
    Sol_U32    flags;
    Sol_U32    reserved1;
    Sol_U32    reserved2;
    Sol_U32    reserved3;
};

struct MachSymtabCommand {
    Sol_U32    cmd;
    Sol_U32    cmdsize;
    Sol_U32    symoff;
    Sol_U32    nsyms;
    Sol_U32    stroff;
    Sol_U32    strsize;
};

struct MachDsymtabCommand {
    Sol_U32 cmd;
    Sol_U32 cmdsize;
    Sol_U32 ilocalsym;
    Sol_U32 nlocalsym;
    Sol_U32 iextdefsym;
    Sol_U32 nextdefsym;
    Sol_U32 iundefsym;
    Sol_U32 nundefsym;
    Sol_U32 tocoff;
    Sol_U32 ntoc;
    Sol_U32 modtaboff;
    Sol_U32 nmodtab;
    Sol_U32 extrefsymoff;
    Sol_U32 nextrefsyms;
    Sol_U32 indirectsymoff;
    Sol_U32 nindirectsyms;
    Sol_U32 extreloff;
    Sol_U32 nextrel;
    Sol_U32 locreloff;
    Sol_U32 nlocrel;
};

struct MachNList {
    union {
        Sol_U32 n_strx;
    } n_un;

    Sol_U8  n_type;
    Sol_U8  n_sect;
    Sol_I16 n_desc;
    Sol_U64 n_value;
};

struct MachRelocation {
    Sol_I32 r_address;
    Sol_U32 r_symbolnum: 24,
            r_pcrel:      4,
            r_length:     2,
            r_extern:     1,
            r_type:       4;
};

enum MachX86RelocType {
    Mach_X86_UNSIGNED,      // for absolute addresses
    Mach_X86_SIGNED,        // for signed 32-bit displacement
    Mach_X86_BRANCH,        // a CALL/JMP instruction with 32-bit displacement
    Mach_X86_GOT_LOAD,      // a MOVQ load of a GOT entry
    Mach_X86_GOT,           // other GOT references
    Mach_X86_SUBTRACTOR,    // must be followed by a X86_64_RELOC_UNSIGNED
    Mach_X86_SIGNED_1,      // for signed 32-bit displacement with a -1 addend
    Mach_X86_SIGNED_2,      // for signed 32-bit displacement with a -2 addend
    Mach_X86_SIGNED_4,      // for signed 32-bit displacement with a -4 addend
    Mach_X86_TLV,           // for thread local variables

    _MACH_X86_RELOC_TYPE_COUNT
};

SOL_STATIC_ASSERT(_MACH_X86_RELOC_TYPE_COUNT <= 0xF, "Reloc type must fit within 4 bits");

struct MachSymbol {
    struct SolSymbol *symbol;
    int offset;
    int stringOffset;
};

int machSymbolSort(struct MachSymbol *a, struct MachSymbol *b) {
    return strcmp(a->symbol->name, b->symbol->name);
}

void OutputMachO(
    const char *path,
    char *data,
    int dataLen,
    struct SolSymbol *symbols,
    int symbolCount
) {
    int commandCount = 2;
    int commandsSize = sizeof(struct MachSegmentCommand) + sizeof(struct MachSymtabCommand);

    if (symbolCount) {
        commandCount++;
        commandsSize += sizeof(struct MachDsymtabCommand);
    }

    int sectionCount = 1;
    int sectionsSize = sectionCount * sizeof(struct MachSection);

    int segmentOffset = sizeof(struct MachHeader) + commandsSize + sectionsSize;

    struct MachSegmentCommand segmentCommand = {0};
    segmentCommand.cmd = MachLoadCommandType_Segment;
    segmentCommand.cmdsize = sizeof(struct MachSegmentCommand) + sectionsSize;
    segmentCommand.vmaddr = 0;
    segmentCommand.vmsize = dataLen;
    segmentCommand.fileoff = segmentOffset;
    segmentCommand.filesize = dataLen;
    segmentCommand.maxprot = Mach_VM_PROT_ALL;
    segmentCommand.initprot = Mach_VM_PROT_ALL;
    segmentCommand.nsects = sectionCount;
    segmentCommand.flags = 0;

    struct MachSection textSection = {0};
    strncpy(&textSection.sectname[0], "__text", 16);
    strncpy(&textSection.segname[0],  "__TEXT", 16);
    textSection.size = dataLen;
    textSection.offset = segmentOffset;
    textSection.align = 4;
    textSection.reloff = 0;
    textSection.nreloc = 0;
    textSection.flags = 0;

    struct MachHeader header = {
        .magic = MachMagic,
        .cputype = Mach_CPU_TYPE_X86_64,
        .cpusubtype = Mach_CPU_SUBTYPE_X86_ALL,
        .filetype = MachObject,
        .ncmds = commandCount,
        .sizeofcmds = commandsSize + sectionsSize,
        .flags = 0x02000000,
        .reserved = 0
    };

    int segmentLen = dataLen;
    int symOffset = segmentOffset + segmentLen;

    int strOff = symOffset + (symbolCount * sizeof(struct MachNList));

    int definedSymbolCount = 0;
    int undefinedSymbolCount= 0;

    for (int i = 0; i < symbolCount; i += 1) {
        if (!SOL_DEFINED(symbols[i]) && SOL_EXTERNAL(symbols[i]))
            undefinedSymbolCount++;
        else
            definedSymbolCount++;
    }

    struct MachSymbol *definedSymbols   = malloc(definedSymbolCount   * sizeof(*definedSymbols));
    struct MachSymbol *undefinedSymbols = malloc(undefinedSymbolCount * sizeof(*undefinedSymbols));

    int strTableSize = 1;

    int definedOffset = 0;
    int undefinedOffset = 0;

    for (int i = 0; i < symbolCount; i += 1) {
        if (!SOL_DEFINED(symbols[i]) || !SOL_EXTERNAL(symbols[i])) {
            continue;
        }

        definedSymbols[definedOffset].symbol = &symbols[i];
        definedSymbols[definedOffset].offset = symbols[i].offset;
        definedSymbols[definedOffset].stringOffset = strTableSize;

        definedOffset++;
        strTableSize += strlen(symbols[i].name) + 1;
    }

    for (int i = 0; i < symbolCount; i += 1) {
        if (SOL_DEFINED(symbols[i]) || !SOL_EXTERNAL(symbols[i])) {
            continue;
        }

        undefinedSymbols[undefinedOffset].symbol = &symbols[i];
        undefinedSymbols[undefinedOffset].offset = symbols[i].offset;
        undefinedSymbols[undefinedOffset].stringOffset = strTableSize;

        undefinedOffset++;
        strTableSize += strlen(symbols[i].name) + 1;
    }

    qsort(definedSymbols, definedSymbolCount, sizeof(*definedSymbols), ( int (*)(const void *, const void *))machSymbolSort);
    qsort(undefinedSymbols, undefinedSymbolCount, sizeof(*undefinedSymbols), ( int (*)(const void *, const void *))machSymbolSort);

    struct MachSymtabCommand symtabCommand = {0};
    symtabCommand.cmd = MachLoadCommandType_SymbolTable;
    symtabCommand.cmdsize = sizeof(struct MachSymtabCommand);
    symtabCommand.symoff = symOffset;
    symtabCommand.nsyms = symbolCount;
    symtabCommand.stroff = strOff;
    symtabCommand.strsize = strTableSize;

    FILE *output = fopen(path, "w");
    if (!output) return;

    fwrite(&header, sizeof(struct MachHeader), 1, output);
    fwrite(&segmentCommand, sizeof(struct MachSegmentCommand), 1, output);
    fwrite(&textSection, sizeof(struct MachSection), 1, output);
    fwrite(&symtabCommand, sizeof(struct MachSymtabCommand), 1, output);

    if (symbolCount) {
        struct MachDsymtabCommand command = {0};
        command.cmd = MachLoadCommandType_DynamicSymbolTable;
        command.cmdsize = sizeof(command);
        command.iextdefsym = 0;
        command.nextdefsym = definedSymbolCount;
        command.iundefsym = definedSymbolCount;
        command.nundefsym = undefinedSymbolCount;
        fwrite(&command, sizeof(struct MachDsymtabCommand), 1, output);
    }

    fwrite(data, 1, dataLen, output);

    for (int i = 0; i < definedSymbolCount; i += 1) {
        struct MachNList symbol = {
            .n_un.n_strx = definedSymbols[i].stringOffset,
            .n_type = 0x0F,
            .n_sect = 1,
            .n_desc = 0,
            .n_value = definedSymbols[i].offset
        };

        fwrite(&symbol, sizeof(struct MachNList), 1, output);
    }

    for (int i = 0; i < undefinedSymbolCount; i += 1) {
        struct MachNList symbol = {
            .n_un.n_strx = undefinedSymbols[i].stringOffset,
            .n_type = 0x0F,
            .n_sect = 1,
            .n_desc = 0,
            .n_value = undefinedSymbols[i].offset
        };

        fwrite(&symbol, sizeof(struct MachNList), 1, output);
    }

    fprintf(output, "%c", 0);

    for (int i = 0; i < definedSymbolCount; i += 1) {
        fprintf(output, "%s%c", definedSymbols[i].symbol->name, 0);
    }

    for (int i = 0; i < undefinedSymbolCount; i += 1) {
        fprintf(output, "%s%c", undefinedSymbols[i].symbol->name, 0);
    }

}
