#pragma once

#include <stdint.h>
#include <string.h>
#include "ym2413_map.h"

typedef struct
{
    uint8_t registers[0x40];

    uint8_t fnum_lsb[0x10];
    uint8_t fnum_msb[0x10];
    uint8_t oct[0x10];

} ym2413_t;

static ym2413_t ym2413_chip_0;

void ym2413_w(uint8_t aa, uint8_t dd);

void _ym2413_update_frequency(uint8_t slot_n)
{
    // freq = int(f_number * F_SAM_YM2413 * math.pow(2.0, block - 19)) # block starts from 0 (A4's fnum is 288 and b is 3 )

    volatile scsp_slot_regs_t *slots = get_scsp_slot(0);
    volatile scsp_slot_regs_t *slot = &slots[slot_n];
    slot->oct = ym2413_chip_0.oct[slot_n];
    int fnum = (ym2413_chip_0.fnum_lsb[slot_n] | (ym2413_chip_0.fnum_msb[slot_n] << 8)) & 511;

    slot->fns = ym2413_fnum_fns_map[fnum];
}

void ym2413_init()
{
    // copy sinetable
    for (int i = 0; i < 1; i++)
    {
        sn_fix_note((int16_t *)(SCSP_RAM + sn_square_addr), (uint16_t *)SN_NOTE, sizeof(SN_NOTE) / 2, 8192, 0);
        // sn_fix_note((uint16_t *)SN_PER_NOTE, sizeof(SN_PER_NOTE) / 2);
        sn_fix_note((int16_t *)(SCSP_RAM + sn_white_noise_addr), (uint16_t *)SN_NOISE_NOTE, sizeof(SN_NOISE_NOTE) / 2, 4096, -4096);

        // memcpy((void *)(SCSP_RAM + sn_square_addr + (i * sizeof(SN_NOTE))), SN_NOTE, sizeof(SN_NOTE));
        memcpy((void *)(SCSP_RAM + sn_periodic_addr + (i * sizeof(SN_PER_NOTE))), SN_PER_NOTE, sizeof(SN_PER_NOTE));
        //  memcpy((void *)(SCSP_RAM + sn_white_noise_addr + (i * sizeof(SN_NOISE_NOTE))), SN_NOISE_NOTE, sizeof(SN_NOISE_NOTE));
    }

    volatile uint16_t *control = (uint16_t *)0x25b00400;
    volatile uint16_t *intr = (uint16_t *)0x25b0042a;
    control[0] = 0xf; // master vol
    intr[0] = 0;      // irq

    // init slots...
    volatile scsp_slot_regs_t *slots = get_scsp_slot(0);

    for (int i = 0; i < 32; i++)
    {
        slots[i].kyonb = 0;
        slots[i].kyonex = 0;
    }

    for (int i = 0; i < 9; i++)
    {
        scsp_slot_regs_t slot = {};
        memset(&slot, 0, sizeof(scsp_slot_regs_t));

        //
        slot.sa = sn_square_addr;
        slot.lsa = 0;
        slot.lea = (sizeof(SN_NOTE) / 2);

        // uned 16 bit
        slot.pcm8b = 0;
        slot.sbctl = 0;

        slot.lpctl = 1;
        slot.attack_rate = 31;
        slot.release_r = 31;
        slot.loop_start = 1;
        slot.kr_scale = 0;
        slot.sdir = 0;
        slot.disdl = 7;
        slot.total_l = 0x0f;

        slot.fns = 0;
        slot.oct = 0;

        // memcpy(&slots[i], 0, sizeof(scsp_slot_regs_t));
        for (int r = 0; r < 16; r++)
        {
            slots[i].raw[r] = slot.raw[r];
        }
    }

    for (int i = 0; i < 9; i++)
    {
        slots[i].kyonb = 0;
        slots[i].kyonex = 0;
    }
}

void ym2413_w(uint8_t aa, uint8_t dd)
{
    volatile scsp_slot_regs_t *slots = get_scsp_slot(0);
    uint8_t reg = aa & 0x3f;
    if (dd == ym2413_chip_0.registers[reg])
    {
        // nothing changed...
        return;
    }

    // start impl...
    ym2413_chip_0.registers[reg] = dd;
    uint8_t chan = reg & 0xf;
    volatile scsp_slot_regs_t *slot = &slots[chan];

    switch (reg)
    {
    case 0x00 ... 0x07:
        // ignored
        break;
    case 0x0E:
        // Rhythm sound mode selection. 1: Rhythm sound mode 0: Melody sound mode
        if (dd & (1 << 5))
        {
        }

        break;
    case 0x10 ... 0x1F:
        // F-Number LSB 8 bits
        // convert the frequency to scsp fns:oct
        ym2413_chip_0.fnum_lsb[chan] = dd;
        _ym2413_update_frequency(chan);
        break;
    case 0x20 ... 0x2F:
        ym2413_chip_0.fnum_msb[chan] = dd & 1;
        ym2413_chip_0.oct[chan] = (dd >> 1) & 7;
        _ym2413_update_frequency(chan);

        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");

        slot->kyonb = (dd >> 4) & 1;
        slot->kyonex = (dd >> 4) & 1;
        break;
    case 0x30 ... 0x3F:

        slot->total_l = (dd & 0x0F) << 4;
        break;
    default:
        // ignored
        break;
    }
}
