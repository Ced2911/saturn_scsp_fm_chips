#include <yaul.h>
#include <stdio.h>
#include <stdlib.h>
#include "vgm.h"
#include "empty_drv.h"
#include "menu/menu.h"
#include "tracks/tracks.h"

#define RESOLUTION_WIDTH (352)
#define RESOLUTION_HEIGHT (224)

void snd_init()
{
    *(volatile uint8_t *)(0x25B00400) = 0x02;
    // Turn off Sound CPU
    smpc_smc_sndoff_call();

    // Make sure SCSP is set to 512k mode
    *(volatile uint8_t *)(0x25B00400) = 0x02;

    // Clear Sound Ram
    for (int i = 0; i < 0x80000; i += 4)
        *(volatile uint32_t *)(0x25A00000 + i) = 0x00000000;

    // Copy driver over
    for (int i = 0; i < sound_driver_size; i++)
        *(volatile uint8_t *)(0x25A00000 + i) = sound_driver[i];

    // Turn on Sound CPU again
    smpc_smc_sndon_call();
}

// Menu
void _vgm_play_track(const uint8_t *track);
#define _TRACK_FN(x)             \
    static void track_play_##x() \
    {                            \
        _vgm_play_track(x);      \
    }

_TRACK_FN(alexkidd_sn);
_TRACK_FN(mj_smooth_sn);
_TRACK_FN(sonic_sn76496);
_TRACK_FN(aleste_2_op_msx2);

static smpc_peripheral_digital_t _digital;
static menu_entry_t _menu_entries[] = {
    MENU_ACTION_ENTRY("SN: Alex Kidd", track_play_alexkidd_sn),
    MENU_ACTION_ENTRY("SN: Smooth Criminal", track_play_mj_smooth_sn),
    MENU_ACTION_ENTRY("SN: Sonic", track_play_sonic_sn76496),
    MENU_ACTION_ENTRY("YM2413: Aleste 2", track_play_aleste_2_op_msx2)};

static void _menu_input(menu_t *menu)
{
    if ((_digital.held.button.down) != 0)
    {
        menu_cursor_down_move(menu);
    }
    else if ((_digital.held.button.up) != 0)
    {
        menu_cursor_up_move(menu);
    }
    else if ((_digital.held.button.a) != 0)
    {
        menu_action_call(menu);
    }
}

// Vgm
#define SAMPLE_PER_VBK (44100 / 60)

vgm_player_t vgm_player;

void _vgm_play_track(const uint8_t *track)
{
    vgm_init(&vgm_player, track);
    vgm_player.sample_count = 0;
    vgm_player.cycles_played = 0;
}

void vgm_update()
{
    if (vgm_player.vgm)
    {
        uint16_t s;

        while (vgm_player.cycles_played < SAMPLE_PER_VBK)
        {
            vgm_player.cycles_played += vgm_parse(&vgm_player);
        }
        vgm_player.cycles_played -= SAMPLE_PER_VBK;
        dbgio_flush();
    }
}

int main(void)
{
    snd_init();

    dbgio_init();
    dbgio_dev_default_init(DBGIO_DEV_VDP2_ASYNC);
    dbgio_dev_font_load();

    menu_t state;

    menu_init(&state);
    menu_entries_set(&state, _menu_entries, sizeof(_menu_entries) / sizeof(*_menu_entries));
    menu_input_set(&state, _menu_input);

    state.data = NULL;
    state.flags = MENU_ENABLED | MENU_INPUT_ENABLED;

    // dbgio_printf("vgm player...");
    menu_update(&state);

    while (1)
    {
        smpc_peripheral_process();
        smpc_peripheral_digital_port(1, &_digital);
        dbgio_printf("[H[2J");

        menu_update(&state);

        dbgio_flush();
        vdp2_sync();
        vdp2_sync_wait();
    }
    // screen->destroy();
}

static void _vblank_out_handler(void *work __unused)
{
    smpc_peripheral_intback_issue();
    vgm_update();
}
static void _vblank_in_handler(void *work __unused)
{
    // sdrv_vblank_rq();
    // fm_test();
}

void user_init(void)
{
    smpc_peripheral_init();

    vdp2_tvmd_display_res_set(VDP2_TVMD_INTERLACE_NONE, VDP2_TVMD_HORZ_NORMAL_A,
                              VDP2_TVMD_VERT_224);

    vdp2_scrn_back_color_set(VDP2_VRAM_ADDR(3, 0x01FFFE),
                             RGB1555(1, 0, 3, 15));

    vdp_sync_vblank_out_set(_vblank_out_handler, NULL);

    vdp2_tvmd_display_set();
}
