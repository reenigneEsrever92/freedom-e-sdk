/* Copyright 2021 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <metal/machine/platform.h>
#include <metal/remapper.h>

#define VALID_REGS_NUM  7

/* rom_data should be placed in ROM section, total size: 16 bytes. */
const volatile uint32_t rom_data[] __attribute__((aligned(16))) = {1, 2, 3, 4};

/* ram_data should be placed in RAM section, total size: 16 bytes. */
volatile uint32_t ram_data[] __attribute__((aligned(16))) = {5, 6, 7, 8};

void print_rom_data(void) {
    for (int i = 0; i < 4; i++) {
        printf("rom_data[%d] = %d\n", i, rom_data[i]);
    }
}

void valid_rom_data(uint32_t answers[]) {
    for (int i = 0; i < 4; i++) {
        if (rom_data[i] != answers[i]) {
            printf("rom_data[%d] = %d, correct value = %d\n",
                   i, rom_data[i], answers[i]);
            exit(1);
        }
    }
}

void print_remappervalid(int idx) {
    printf("remappervalid[%d] = 0x%08x\n", idx, metal_remapper_get_valid(idx));
}

int main(void) {
    size_t rom_data_addr = (size_t)rom_data;
    size_t ram_data_addr = (size_t)ram_data;

    printf("rom_data address: 0x%08x\n", rom_data_addr);
    printf("ram_data address: 0x%08x\n", ram_data_addr);

    int entry_idx = 113;
    int valid_reg_idx = entry_idx / 32;
    uint64_t from_region_base = metal_remapper_get_from_region_base();
    uint64_t to_region_base = metal_remapper_get_to_region_base();

    /* Remap ram_data to rom_data with remapping size: 16 bytes. */
    struct metal_remapper_entry entry = {
        .idx = entry_idx,
        /* We are safe to OR 0x7 here as rom_data is already 16-bytes aligned. */
        .from_addr = (rom_data_addr - from_region_base) | 0x7,
        .to_addr = (ram_data_addr - to_region_base),
    };
    metal_remapper_set_remap(&entry);

    uint32_t version = metal_remapper_get_version();
    uint64_t from_addr = metal_remapper_get_from(entry.idx);
    uint64_t to_addr = metal_remapper_get_to(entry.idx);

    printf("Address Remapper version: %d\n", version);
    printf("Remap entry - From[]: 0x%016lx\n", from_addr);
    printf("Remap entry - To[]: 0x%016lx\n", to_addr);
    printf("Remap rom_data to ram_data, remap size: %d bytes.\n.",
           sizeof(rom_data));

    printf("Enable remap...\n");
    metal_remapper_enable_remap(entry.idx);
    print_rom_data();
    valid_rom_data((uint32_t[]){5, 6, 7, 8});
    print_remappervalid(valid_reg_idx);
    printf("\n");

    printf("Disable remap...\n");
    metal_remapper_disable_remap(entry.idx);
    print_rom_data();
    valid_rom_data((uint32_t[]){1, 2, 3, 4});
    print_remappervalid(valid_reg_idx);
    printf("\n");

    printf("Enable remap...\n");
    metal_remapper_enable_remap(entry.idx);
    print_rom_data();
    valid_rom_data((uint32_t[]){5, 6, 7, 8});
    print_remappervalid(valid_reg_idx);
    printf("\n");

    printf("Flush all remaps...\n");
    metal_remapper_flush();
    print_rom_data();
    valid_rom_data((uint32_t[]){1, 2, 3, 4});
    printf("\n");

    for (int i = 0; i < VALID_REGS_NUM; i++) {
        print_remappervalid(i);
    }

    printf("End of example.\n");

    return 0;
}

