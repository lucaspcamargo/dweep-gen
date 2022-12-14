#include "playfield.h"
#include "palette_ctrl.h"


enum PLFLaserFrame // description of laser sprite frames
{
    PLF_LASER_FRAME_H,
    PLF_LASER_FRAME_UL,
    PLF_LASER_FRAME_DL,
    PLF_LASER_FRAME_V,
    PLF_LASER_FRAME_DT,
    PLF_LASER_FRAME_DR,
    PLF_LASER_FRAME_UR,
    PLF_LASER_FRAME_LT,
} ENUM_PACK;


typedef struct MapObject_st {
    u16 type;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} MapObject;



// VARS
u16 plf_w;
u16 plf_h;
PlfTile plf_tiles[PLAYFIELD_STD_SZ];
Map* m_a;
Map* m_b;
fix16 plf_cam_cx;
fix16 plf_cam_cy;
fix16 plf_player_initial_x;
fix16 plf_player_initial_y;

#define TILE_AT(x, y) (plf_tiles[(x) + (y)*plf_w])

void PLF_init()
{
    // init playfield vars
    plf_w = PLAYFIELD_STD_W;
    plf_h = PLAYFIELD_STD_H;
    m_a = NULL;
    m_b = NULL;
    plf_cam_cx = FIX16(16*(PLAYFIELD_STD_W/2));
    plf_cam_cy = FIX16(16*(PLAYFIELD_STD_H/2));

    // setup palettes
    PCTRL_set_source(PAL_LINE_BG_0, pal_tset_0.data, FALSE);
    PCTRL_set_source(PAL_LINE_BG_1, pal_tset_0.data+16, FALSE);

    // TODO store these palette ops with tileset somehow?
    PalCtrlOperatorDescriptor pal_op =
    {
        PCTRL_OP_CYCLE,
        44, 3, 0x1f, NULL, NULL
    };
    PCTRL_op_add(&pal_op);
    pal_op.idx_base = 62;
    pal_op.len = 2;
    pal_op.period_mask = 0x03;
    PCTRL_op_add(&pal_op);


    // load bg graphics
    VDP_loadTileSet(&tset_0, TILE_USER_INDEX, DMA);

    // load map data
    m_a = MAP_create(&map_0_a, BG_A, TILE_ATTR_FULL(PAL_LINE_BG_0, 1, 0, 0, TILE_USER_INDEX));
    m_b = MAP_create(&map_0_b, BG_B, TILE_ATTR_FULL(PAL_LINE_BG_0, 0, 0, 0, TILE_USER_INDEX));

    // init camera
    PLF_cam_to( FIX16(16*(PLAYFIELD_STD_W/2)),FIX16(16*(PLAYFIELD_STD_H/2)) );

    // init plane data
    PLF_update_scroll(TRUE);

    // clear field data
    for(u16 x = 0; x < plf_w; x++)
        for(u16 y = 0; y < plf_h; y++)
        {
            PlfTile tile;
            tile.attrs = 0x00;
            tile.laser = 0x00;
            TILE_AT(x, y) = tile;
        }

    // process object definitions
    const void** const objs = map_0_o;
    const u16 obj_count = sizeof(map_0_o)/sizeof(void*);
    if(DEBUG_MAP_OBJS)
    {
        char buf[5];
        intToHex(obj_count, buf,4);
        VDP_drawTextBG(BG_A, buf, 0, 23);
        VDP_drawTextBG(BG_A, " OBJS", 4, 23);
    }

    for(u16 iobj=0; iobj<obj_count; iobj++)
    {
        const MapObject* const obj = objs[iobj];
        if(obj->type >= PLF_OBJ_MAX)
            continue; // invalid obj spec

        if(obj->w || obj->h)
        {
            // wall rect or tile object
            u16 startx = obj->x/16;
            u16 endx = (obj->x+obj->w-1)/16;
            u16 starty;
            u16 endy;

            if(PLF_OBJ_IS_Y_BOTTOM(obj->type))
            {
                starty = (obj->y-obj->h)/16;
                endy = (obj->y-1)/16;
            }
            else{
                starty = obj->y/16;
                endy = (obj->y+obj->h-1)/16;
            }

            for(u16 x = startx; x <= endx; x++)
                for(u16 y = starty; y <= endy; y++)
                {
                    TILE_AT(x,y).attrs |= PLF_ATTR_SOLID;
                }
        }
        else
        {
            // point object - player initial pos
            plf_player_initial_x = FIX16(obj->x/16);
            plf_player_initial_y = FIX16(obj->y/16);
        }

        if(DEBUG_MAP_OBJS && iobj < 28)
        {
            char buf[5];
            intToHex(obj->type,buf,2);
            VDP_drawTextBG(BG_A, buf, 0, iobj);
            VDP_drawTextBG(BG_B, buf, 0, iobj);
            intToHex(obj->x,buf,4);
            VDP_drawTextBG(BG_A, buf, 3, iobj);
            VDP_drawTextBG(BG_B, buf, 3, iobj);
            intToHex(obj->y,buf,4);
            VDP_drawTextBG(BG_A, buf, 8, iobj);
            VDP_drawTextBG(BG_B, buf, 8, iobj);
            intToHex(obj->w,buf,4);
            VDP_drawTextBG(BG_A, buf, 13, iobj);
            VDP_drawTextBG(BG_B, buf, 13, iobj);
            intToHex(obj->h,buf,4);
            VDP_drawTextBG(BG_A, buf, 18, iobj);
            VDP_drawTextBG(BG_B, buf, 18, iobj);
        }
    }

    if (DEBUG_TILES)
    {
        for(u16 x = 0; x < plf_w; x++)
            for(u16 y = 0; y < plf_h; y++)
            {
                // debug metatile using layer A
                char buf[4];
                intToHex(plf_tiles[x + y*plf_w].attrs,buf,2);
                VDP_drawTextBG(BG_A, buf, x*2, y*2);
            }
    }

    // LASER TEST
    Sprite * laser_test = SPR_addSprite(&spr_laser, 6*16, 9*16-5,TILE_ATTR_FULL(PAL_LINE_BG_1,0,0,0,0));
    SPR_setAnimAndFrame(laser_test, 0, 0);
}

void PLF_cam_to(fix16 cx, fix16 cy)
{
    plf_cam_cx = cx;
    plf_cam_cy = cy;
}

PlfTile PLF_get_tile(u16 pf_x, u16 pf_y)
{
    return plf_tiles[pf_x + pf_y*plf_w];
}

void PLF_player_get_initial_pos(f16 *dest_x, f16 *dest_y)
{
    *dest_x = plf_player_initial_x;
    *dest_y = plf_player_initial_y;
}

bool PLF_player_pathfind(u16 px, u16 py, u16 destx, u16 desty)
{
    // NOTE if/when scrolling levels are implemented, we'll need to do some coordinate conversion here
    //      and limit the pathfinding to the current screen via stride_y
    //      for now, just convert to u8
    enum PathfindingResult res = PATH_find(PLAYFIELD_STD_W, PLAYFIELD_STD_H, (u8)px, (u8)py, (u8)destx, (u8)desty,
                                           ((u8*)&(plf_tiles->attrs)), sizeof(PlfTile), sizeof(PlfTile)*PLAYFIELD_STD_W, PLF_ATTR_SOLID);

    if(DEBUG_PATHFINDING)
    {
        char buf[20];
        sprintf(buf, "%d,%d to %d,%d: %d(%d)", (int) px, (int) py, (int) destx, (int) desty, (int) res, (int) PATH_curr_node_count() );
        VDP_drawText(buf, 16, 27);
    }
    return res==PATH_FOUND;
}

bool PLF_player_path_next(u16 px, u16 py, u16 *nextx, u16 *nexty)
{
    u8 bufx, bufy;
    bool ret = PATH_next(PLAYFIELD_STD_W, PLAYFIELD_STD_H, (u8)px, (u8)py, &bufx, &bufy);
    if(ret)
    {
        *nextx = bufx;
        *nexty = bufy;
    }
    return ret;
}

/*
 * Note: forceRedraw will call vblank processing twice for uploading map data to planes
 * */
void PLF_update_scroll(bool forceRedraw)
{
    s16 plf_scroll_x = fix16ToRoundedInt(fix16Sub(plf_cam_cx, FIX16(160)));
    s16 plf_scroll_y = fix16ToRoundedInt(fix16Sub(plf_cam_cy, FIX16(96)));

    VDP_setHorizontalScroll(BG_A, -plf_scroll_x);
    VDP_setHorizontalScroll(BG_B, -plf_scroll_x);
    VDP_setVerticalScroll(BG_A, plf_scroll_y);
    VDP_setVerticalScroll(BG_B, plf_scroll_y);

    MAP_scrollToEx(m_a, plf_scroll_x, plf_scroll_y, forceRedraw);
    if (forceRedraw)
        SYS_doVBlankProcess();
    MAP_scrollToEx(m_b, plf_scroll_x, plf_scroll_y, forceRedraw);
    if (forceRedraw)
        SYS_doVBlankProcess();

    if(DEBUG_CAMERA)
    {
        char buf[40];
        sprintf(buf, "@%d,%d   ", fix16ToRoundedInt(plf_cam_cx), fix16ToRoundedInt(plf_cam_cy));
        VDP_drawTextBG(BG_A, buf, 0, 27);
    }
}

void PLF_laser_place() {

}
