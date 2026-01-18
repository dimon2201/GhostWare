// category.hpp

#pragma once

namespace arachne
{
    enum class eCategory
    {
        PHYSICS_ACTOR_NONE = 0,
        PHYSICS_ACTOR_STATIC = 1,
        PHYSICS_ACTOR_DYNAMIC = 2,
        PHYSICS_SHAPE_PLANE = 3,
        PHYSICS_SHAPE_BOX = 4,
        VERTEX_BUFFER_FORMAT_NONE = 5,
        VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3 = 6,
        SOUND_FORMAT_WAV = 7,
        CAMERA_ANGLE_PITCH = 8,
        CAMERA_ANGLE_YAW = 9,
        CAMERA_ANGLE_ROLL = 10,
        PRIMITIVE_TRIANGLE = 11,
        PRIMITIVE_QUAD = 12,
        RENDER_PATH_NONE = 13,
        RENDER_PATH_OPAQUE = 14,
        RENDER_PATH_TRANSPARENT = 15,
        RENDER_PATH_TEXT = 16,
        RENDER_PATH_TRANSPARENT_COMPOSITE = 17,
        RENDER_PATH_QUAD = 18
    };
}