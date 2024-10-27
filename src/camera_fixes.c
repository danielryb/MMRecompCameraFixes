#include "modding.h"
#include "global.h"

RECOMP_IMPORT("*", void recomp_set_camera_fixes(bool new_val))

RECOMP_CALLBACK("*", recomp_on_init) void on_init()
{
    recomp_set_camera_fixes(true);
}