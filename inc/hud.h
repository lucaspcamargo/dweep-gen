#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"

enum HUDState
{
    HUD_ST_NORMAL,              // normal gameplay state
    HUD_ST_DIALOG_STARTING,     // drawing the dialog background and portrait
    HUD_ST_DIALOG_TYPING,       // drawing the works animatedly
    HUD_ST_DIALOG_IDLE,         // dialog written
    HUD_ST_UNINITIALIZED = 0xff
} ENUM_PACK;

void HUD_preinit(); // just load graphics data used by rest of system
void HUD_init();
void HUD_setVisible(bool visible);
void HUD_update();

void HUD_dialog_start(u8 character_id, u16 string_id);  // animated, requires per frame HUD_update
void HUD_dialog_end();
