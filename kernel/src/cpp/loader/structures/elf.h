#pragma once

#include "stdint.h"

const uint32_t ELF32_MAGIC_NUMER = 1179403647;

struct Elf32Header {
    uint32_t magic_number;
    enum Bits : uint8_t {
        X32 = 1,
        X64 = 2
    } bits;
    enum Endian : uint8_t {
        LITTLE = 1,
        BIG = 2
    } endian;
    uint8_t header_version;
    uint8_t os_abi;
    uint8_t _reserved[8];
    enum Type : uint16_t {
        RELOCATABLE = 1,
        EXECUTABLE = 2,
        SHARED = 3,
        CORE = 4
    } elf_type;
    enum Arch : uint16_t {
        X86 = 3,
        X86_64 = 0x3E
    } architecture;
    uint32_t elf_version;
    uint32_t virtual_entry_address;
    uint32_t program_header_table_offset;
    uint32_t section_header_table_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t section_header_names_index;
} __attribute__((packed));

struct Elf32ProgramHeader {
    enum SegmentType : uint32_t {
        NULL_S = 0,
        LOADABLE = 1,
        DYNAMIC = 2,
        INTERP = 3,
        NOTE = 4
    } segment_type;
    uint32_t data_offset;
    void* virtual_start_address;
    uint32_t _reserved;
    uint32_t segment_size;
    uint32_t segment_memory_size;
    struct Flags {
        bool EXECUTABLE : 1;
        bool WRITABLE : 1;
        bool READABLE : 1;
        uint32_t _reversed : 29;
    } __attribute__((packed)) flags;
    uint32_t data_required_alignment;
} __attribute__((packed));