#pragma once

#include <genesis.h>
#include "palette_ctrl.h"
#include "tools.h"

#define RGST_PCTRL_OP_MAX 4

// a level in the game
// specifies where all the data is
// and other metadata
typedef struct RGST_lvl_st {
    const char name[32];
    const MapDefinition *bg_a_map;
    const MapDefinition *bg_b_map;
    const TileSet *bg_tileset;
    const Palette *bg_tileset_pal;
    const void ** const obj_map;
    const u16 obj_count;
    const u8 * bg_a_allocation;
    const u8 * bgm_xgm;
    PalCtrlOperatorDescriptor pal_ops[RGST_PCTRL_OP_MAX];
    const enum ToolId tool_inventory[9];
} RGST_lvl;

extern const RGST_lvl RGST_levels[];
extern const uint16_t RGST_lvl_count;

extern const void* RGST_bgms[];
extern const uint16_t RGST_bgm_count;
