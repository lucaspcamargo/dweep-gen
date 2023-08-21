#include "player.h"

#include <genesis.h>
#include "resources.h"
#include "palette_ctrl.h"
#include "playfield.h"
#include "sfx.h"

static fix16 player_pf_x;
static fix16 player_pf_y;
static fix16 player_pf_z;
static fix16 dest_pf_x;
static fix16 dest_pf_y;
static fix16 final_dest_pf_x;
static fix16 final_dest_pf_y;
static bool player_float;
static enum PlayerState player_state;
static Sprite *spr_player;
static Sprite *spr_player_shadow;

void _PLR_update_gfx();
void _PLR_update_bounce(u32 framecounter);

void PLR_init()
{
        // player
    PCTRL_set_source(PAL_LINE_SPR_A, spr_dweep.palette->data, FALSE);
    spr_player = SPR_addSprite(&spr_dweep, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    spr_player_shadow = SPR_addSprite(&spr_shadow, 0, 0, PAL_LINE_SPR_A<<TILE_ATTR_PALETTE_SFT);
    SPR_setPriority(spr_player_shadow, FALSE);
    SPR_setAlwaysVisible(spr_player, 1);

    PLF_player_get_initial_pos(&player_pf_x, &player_pf_y);
    player_pf_z = FIX16(0);
    dest_pf_x = player_pf_x;
    dest_pf_y = player_pf_y;
    final_dest_pf_x = player_pf_x;
    final_dest_pf_y = player_pf_y;
    player_float = FALSE;
    player_state = PLR_STATE_IDLE;

    _PLR_update_gfx();

}

bool PLR_goto(f16 dest_x, f16 dest_y)
{
    bool found = PLF_player_pathfind(fix16ToInt(player_pf_x),fix16ToInt(player_pf_y), dest_x, dest_y);
    if (found)
    {
        SFX_play(SFX_go1+(random()%3));
        player_state = PLR_STATE_MOVING_PATH;
        final_dest_pf_x = FIX16(dest_x);
        final_dest_pf_y = FIX16(dest_y);
        u16 bufx, bufy;
        bool found_next_pos = PLF_player_path_next(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y), &bufx, &bufy);
        if(found_next_pos)
        {
            dest_pf_x = FIX16(bufx);
            dest_pf_y = FIX16(bufy);
            return TRUE;
        }
        else
        {
            // this not supposed to happen, given PLF_player_pathfind succeeded
            return FALSE;
        }
    }
    else
    {
        SFX_play(SFX_no);
        return FALSE;
    }
}

void _PLR_update_bounce(u32 framecounter)
{
    int angle = (framecounter * 4)&1023;
    if(player_float)
    {
        player_pf_z = FIX16(6)+fix16Mul(sinFix16(angle/2), FIX16(4));
    }
    else
    {
        player_pf_z = fix16Mul(sinFix16(angle*2), FIX16(6));
        player_pf_z = player_pf_z<0?fix16Neg(player_pf_z):player_pf_z;
    }
}


void _PLR_update_gfx()
{
    // player position
    SPR_setPosition(spr_player,
    fix16ToRoundedInt(fix16Mul(player_pf_x, FIX16(16)))-4,
    fix16ToRoundedInt(fix16Sub(fix16Mul(player_pf_y, FIX16(16)), player_pf_z))-10);
    SPR_setDepth(spr_player, PLF_get_sprite_depth(player_pf_x, player_pf_y));

    // shadow pos
    SPR_setPosition(spr_player_shadow,
        fix16ToRoundedInt(fix16Mul(player_pf_x, FIX16(16))),
        fix16ToRoundedInt(fix16Mul(player_pf_y, FIX16(16)))+4+3);

    // shadow scale
    SPR_setFrame(spr_player_shadow, \
    ((s16) (fix16Div(player_pf_z, FIX16(3)) >> FIX16_FRAC_BITS)) );
}


void PLR_update(u32 framecounter)
{

    _PLR_update_bounce(framecounter);

    if(player_state == PLR_STATE_MOVING_PATH)
    {
        bool changed = FALSE;
        if(player_pf_x != dest_pf_x)
        {
            fix16 delta = ((player_pf_x < dest_pf_x)?PLAYER_SPEED:-PLAYER_SPEED);
            player_pf_x += delta;
            changed = TRUE;
            //SPR_setHFlip(spr_player, delta<0);
        }
        if(player_pf_y != dest_pf_y)
        {
            player_pf_y += ((player_pf_y < dest_pf_y)?PLAYER_SPEED:-PLAYER_SPEED);
            changed = TRUE;
        }

        // TODO instead of doing this, keep track of which tile dweep is actually in, and then act on that
        if(changed && fix16Frac(player_pf_x)==0 && fix16Frac(player_pf_y)==0)
        {
            // arrived on a tile
            PlfTile tile = *PLF_get_tile(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y));
            player_float = (tile.attrs & PLF_ATTR_HOT)? TRUE : FALSE;
            if(player_pf_x == dest_pf_x && player_pf_y == dest_pf_y)
            {
                // arrived at immediate dest, either we arrived at final destination, or we find our next imm. dest.
                if(player_pf_x == final_dest_pf_x && player_pf_y == final_dest_pf_y)
                {
                    player_state = PLR_STATE_IDLE;
                }
                else
                {
                    u16 bufx, bufy;
                    bool found_next_pos = PLF_player_path_next(fix16ToInt(player_pf_x), fix16ToInt(player_pf_y), &bufx, &bufy);
                    if(found_next_pos)
                    {
                        dest_pf_x = FIX16(bufx);
                        dest_pf_y = FIX16(bufy);
                    }
                    else player_state = PLR_STATE_IDLE;
                }
            }
        }
    }

    _PLR_update_gfx();

    if(DEBUG_PLAYER)
    {
        char buf[20];
        sprintf(buf, "S%X@%X,%X,%X",
                (int) player_state,
                (int) player_pf_x, (int) player_pf_y, (int) player_pf_z);
        VDP_drawText(buf, 0, 24);
        sprintf(buf, "%X,%X", (int) dest_pf_x, (int) dest_pf_y);
        VDP_drawText(buf, 20, 24);
        sprintf(buf, "%X,%X", (int) final_dest_pf_x, (int) final_dest_pf_y);
        VDP_drawText(buf, 30, 24);
    }
}
