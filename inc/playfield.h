#pragma once

#include <genesis.h>

#include "dweep_config.h"
#include "resources.h"
#include "pathfind.h"

// TYPE DFINITIONS

// uniform attributes for every playfield tile
enum PlfAttrBits {
    PLF_ATTR_SOLID = 1 << 0,    // is solid
    PLF_ATTR_HOT   = 1 << 1,    // is hot plate
    PLF_ATTR_COLD  = 1 << 2,    // is cold plate
    PLF_ATTR_OBJ   = 1 << 3,    // contains object, can be solid or not
    PLF_ATTR_DIR_FLIP = 1 << 4, // for every flippable object (incl mirrors)
    PLF_ATTR_DIR_VERT = 1 << 5, // four-way objects like lasers and fans, default up, stacks with _FLIP
    PLF_ATTR_EXTRA_A  = 1 << 6, // reserved
    PLF_ATTR_EXTRA_B = 1 << 7   // reserved
} ENUM_PACK;

// laser attributes for every playfield tile
// for every quadrant direction we can have lasers in and out of the tile
enum PlfLaserBits {
    PLF_LASER_IN_R  = 1 << 0,
    PLF_LASER_IN_L  = 1 << 1,
    PLF_LASER_IN_U  = 1 << 2,
    PLF_LASER_IN_D  = 1 << 3,
    PLF_LASER_OUT_R = 1 << 4,
    PLF_LASER_OUT_L = 1 << 5,
    PLF_LASER_OUT_U = 1 << 6,
    PLF_LASER_OUT_D = 1 << 7
} ENUM_PACK;

#define PLF_LASER_Y_DELTA (-3)

// Do not confuse with Tiled internal "type" attribute, only used for type_filter OBJECTS functionality
// I could add flip/rotation flags for directional objects, BUT i'll just list them for every direction,
//     since most objects are not directional
// This is also different then POBJ_TYPE, although it may seem redundant
// PlfObjType are the object types that can come from level data
// PobjType are the types of objects that exist at runtime
enum PlfObjectType {
    PLF_OBJ_WALL    = 0, // usually rect, marks any tile that contains it as a wall
    PLF_OBJ_PLAYER  = 1, // usually point, where dweep begins
    PLF_OBJ_GOAL    = 2, // end goal of level
    PLF_OBJ_LASER_R = 3, // laser, right
    PLF_OBJ_LASER_L = 4, // laser, left
    PLF_OBJ_LASER_U = 5, // laser, up
    PLF_OBJ_LASER_D = 6, // laser, down
    PLF_OBJ_MIRROR  = 7, // laser, reflects left up /.
    PLF_OBJ_MIRROR2 = 8, // mirror, reflects left down \.
    PLF_OBJ_MAX,         // number of existing specs, value >= to this is invalid, ignored
} ENUM_PACK;

#define PLF_OBJ_IS_Y_BOTTOM(spec_enum) ((spec_enum)>PLF_OBJ_WALL) // specs that have y origin in bottom of box (tile/obj)


typedef struct PlfTile_st {
    u8 attrs;
    u8 laser;
    void* pobj;
} PlfTile;



void PLF_init(u16 lvl_id);

void PLF_cam_to(fix16 cx, fix16 cy);

PlfTile * PLF_get_tile(u16 pf_x, u16 pf_y);
PlfTile * PLF_get_tile_safe(u16 pf_x, u16 pf_y);  // this one return NULL if tile out of bounds

void PLF_player_get_initial_pos(fix16 *dest_x, fix16 *dest_y);

bool PLF_player_pathfind(u16 px, u16 py, u16 destx, u16 desty);

bool PLF_player_path_next(u16 px, u16 py, u16 *nextx, u16 *nexty);

bool PLF_laser_put(u16 orig_x, u16 orig_y, u8 dir);

inline s16 PLF_get_sprite_depth(fix16 x, fix16 y) { (void) x; return 0x8000 - y; }

/*
 * Note: forceRedraw will call vblank processing twice for uploading map data to planes
 * */
void PLF_update_scroll(bool forceRedraw);
