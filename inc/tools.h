#pragma once
#include "dweep_config.h"
#include <genesis.h>

enum ToolId {
    TOOL_NONE,
    TOOL_MOVE,
    TOOL_PLACE_MIRROR_LEFT_DOWN,
    TOOL_PLACE_MIRROR_LEFT_UP,
    TOOL_PLACE_LASER_UP,
    TOOL_PLACE_LASER_DOWN,
    TOOL_PLACE_LASER_LEFT,
    TOOL_PLACE_LASER_RIGHT,
    TOOL_PLACE_FAN_UP,
    TOOL_PLACE_FAN_DOWN,
    TOOL_PLACE_FAN_LEFT,
    TOOL_PLACE_FAN_RIGHT,
    TOOL_PLACE_BOMB,
    TOOL_HAMMER,
    TOOL_ROTATE_CW,
    TOOL_ROTATE_CCW,
    TOOL_TORCH,
    TOOL_BUCKET,
    TOOL_COUNT
} ENUM_PACK;

bool TOOL_get_gfx(enum ToolId tool, u16 *out_frame, bool *out_flip_h, bool *out_flip_v);
