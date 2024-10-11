#include "modding.h"
#include "global.h"

RECOMP_IMPORT("*", int recomp_printf(const char* fmt, ...));

extern s32 sUpdateCameraDirection;
extern s32 sCameraInterfaceFlags;

typedef struct {
    /* 0x0 */ s16 val;
    /* 0x2 */ s16 param;
} CameraModeValue; // size = 0x4

typedef struct {
    /* 0x0 */ s16 funcId;
    /* 0x2 */ s16 numValues;
    /* 0x4 */ CameraModeValue* values;
} CameraMode; // size = 0x8

typedef struct {
    /* 0x0 */ u32 validModes;
    /* 0x4 */ u32 flags;
    /* 0x8 */ CameraMode* cameraModes;
} CameraSetting;

extern CameraSetting sCameraSettings[];
s32 Camera_GetFocalActorPos(Vec3f* dst, Camera* camera);
f32 Camera_GetFocalActorHeight(Camera* camera);
f32 Camera_Vec3fMagnitude(Vec3f* vec);
f32 Camera_GetFloorY(Camera* camera, Vec3f* pos);
f32 Camera_GetFloorYNorm(Camera* camera, Vec3f* floorNorm, Vec3f* chkPos, s32* bgId);
s16 Camera_ScaledStepToFloorS(s16 target, s16 cur, f32 stepScale, s16 minDiff);
s32 func_800CBC84(Camera* camera, Vec3f* from, CameraCollision* to, s32 arg3);
void func_800CBFA4(Camera* camera, Vec3f* arg1, Vec3f* arg2, s32 arg3);
s32 Camera_CalcAtForParallel(Camera* camera, VecGeo* arg1, f32 yOffset, f32 xzOffsetMax, f32* focalActorPosY,
                             s16 flags);
Vec3s* Camera_GetBgCamOrActorCsCamFuncData(Camera* camera, u32 camDataId);
s32 Camera_IsClimbingLedge(Camera* camera);
void Camera_ScaledStepToCeilVec3f(Vec3f* target, Vec3f* cur, f32 xzStepScale, f32 yStepScale, f32 minDiff);
void Camera_SetFocalActorAtOffset(Camera* camera, Vec3f* focalActorPos);
s32 Camera_CalcAtForHorse(Camera* camera, VecGeo* eyeAtDir, f32 yOffset, f32* yPosOffset, s16 calcSlope);
f32 Camera_fabsf(f32 f);
f32 Camera_GetRunSpeedLimit(Camera* camera);

#define CAM_CHANGE_SETTING_0 (1 << 0)
#define CAM_CHANGE_SETTING_1 (1 << 1)
#define CAM_CHANGE_SETTING_2 (1 << 2)
#define CAM_CHANGE_SETTING_3 (1 << 3)

extern SwingAnimation D_801EDC30[4];
s32 Camera_CalcAtDefault(Camera* camera, VecGeo* eyeAtDir, f32 yOffset, s16 calcSlope);
s32 Camera_CalcAtForNormal1(Camera* camera, VecGeo* arg1, f32 yOffset, f32 forwardDist);
s32 Camera_CalcAtForScreen(Camera* camera, VecGeo* eyeAtDir, f32 yOffset, f32* focalActorPosY, f32 deltaYMax);
s16 Camera_CalcDefaultPitch(Camera* camera, s16 pitch, s16 flatSurfacePitchTarget, s16 slopePitchAdj);
void Camera_CalcDefaultSwing(Camera* camera, VecGeo* arg1, VecGeo* arg2, f32 arg3, f32 arg4, SwingAnimation* swing2,
                             s16* flags);
s16 Camera_CalcDefaultYaw(Camera* camera, s16 yaw, s16 target, f32 attenuationYawDiffRange,
                          f32 attenuationYawDiffInterpParam);
f32 Camera_ClampDist1(Camera* camera, f32 dist, f32 minDist, f32 maxDist, s16 timer);
f32 Camera_ClampDist2(Camera* camera, f32 dist, f32 minDist, f32 maxDist, s16 timer);
f32 Camera_ClampLerpScale(Camera* camera, f32 maxLerpScale);
s16 Camera_GetPitchAdjFromFloorHeightDiffs(Camera* camera, s16 viewYaw, s16 shouldInit);
f32 Camera_ScaledStepToCeilF(f32 target, f32 cur, f32 stepScale, f32 minDiff);
s16 Camera_ScaledStepToCeilS(s16 target, s16 cur, f32 stepScale, s16 minDiff);
void Camera_SetUpdateRatesFastYaw(Camera* camera);
s32 func_800CB924(Camera* camera);
s32 func_800CB950(Camera* camera);
s32 func_800CBA7C(Camera* camera);

#define GET_NEXT_RO_DATA(values) ((values++)->val)
#define RELOAD_PARAMS(camera) ((camera->animState == 0) || (camera->animState == 10) || (camera->animState == 20))

#define CAM_RODATA_UNSCALE(x) ((x)*0.01f)
#define GET_NEXT_SCALED_RO_DATA(values) CAM_RODATA_UNSCALE(GET_NEXT_RO_DATA(values))

/**
 * Used for targeting
 */
RECOMP_PATCH s32 Camera_Parallel1(Camera* camera) {
    Vec3f* eye = &camera->eye;
    Vec3f* at = &camera->at;
    Vec3f* eyeNext = &camera->eyeNext;
    Vec3f spB0;
    Vec3f spA4;
    f32 spA0;
    f32 sp9C;
    PosRot* focalActorPosRot = &camera->focalActorPosRot;
    VecGeo sp90;
    VecGeo sp88;
    VecGeo sp80;
    VecGeo sp78;
    BgCamFuncData* bgCamFuncData;
    s16 sp72;
    s16 tangle;
    Parallel1ReadOnlyData* roData = &camera->paramData.para1.roData;
    Parallel1ReadWriteData* rwData = &camera->paramData.para1.rwData;
    s32 parallelFlagCond;
    f32 focalActorHeight = Camera_GetFocalActorHeight(camera);
    s16 new_var2;
    s16 phi_a0;
    s32 phi_a0_2;
    CameraModeValue* values;
    f32 yNormal;

    if (!RELOAD_PARAMS(camera)) {
    } else {
        values = sCameraSettings[camera->setting].cameraModes[camera->mode].values;
        roData->unk_00 =
            GET_NEXT_SCALED_RO_DATA(values) * focalActorHeight * (0.8f - ((68.0f / focalActorHeight) * -0.2f));
        roData->unk_04 =
            GET_NEXT_SCALED_RO_DATA(values) * focalActorHeight * (0.8f - ((68.0f / focalActorHeight) * -0.2f));
        roData->unk_08 =
            GET_NEXT_SCALED_RO_DATA(values) * focalActorHeight * (0.8f - ((68.0f / focalActorHeight) * -0.2f));
        roData->unk_20 = CAM_DEG_TO_BINANG(GET_NEXT_RO_DATA(values));
        roData->unk_22 = CAM_DEG_TO_BINANG(GET_NEXT_RO_DATA(values));
        roData->unk_0C = GET_NEXT_RO_DATA(values);
        roData->unk_10 = GET_NEXT_RO_DATA(values);
        roData->unk_14 = GET_NEXT_RO_DATA(values);
        roData->unk_18 = GET_NEXT_SCALED_RO_DATA(values);
        roData->interfaceFlags = GET_NEXT_RO_DATA(values);
        roData->unk_1C = GET_NEXT_SCALED_RO_DATA(values);
        roData->unk_24 = GET_NEXT_RO_DATA(values);
        rwData->unk_00 = roData->unk_04;
    }

    OLib_Vec3fDiffToVecGeo(&sp80, at, eye);
    OLib_Vec3fDiffToVecGeo(&sp78, at, eyeNext);
    Camera_GetFocalActorPos(&spA4, camera);

    // @mod
    // TODO Replace static prev_targeting_held and timer4 with new fields in rwData.
    static bool prev_targeting_held = false;
    static s16 timer4 = 0; // @mod Used to check if z-target can be quit. Mirrors timer2 values during z-target in unmodified function.

    switch (camera->animState) {
        case 20:
            if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) == 0) {
                Camera_SetUpdateRatesFastYaw(camera);
            }
            // fallthrough
        case 0:
        case 10:
            if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
                (PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) {
                rwData->unk_10 = focalActorPosRot->pos;
            } else {
                camera->xzOffsetUpdateRate = 0.5f;
                camera->yOffsetUpdateRate = 0.5f;
            }

            if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
                PARALLEL1_FLAG_3) {
                rwData->unk_10 = camera->focalActorPosRot.pos;
            }

            rwData->timer1 = 200.0f;

            if ((2.0f * roData->unk_04) < camera->dist) {
                camera->dist = 2.0f * roData->unk_04;
                sp78.r = camera->dist;
                sp80.r = sp78.r;
                OLib_AddVecGeoToVec3f(eye, at, &sp80);
                *eyeNext = *eye;
            }

            rwData->unk_1C = 0;

            if (roData->interfaceFlags & PARALLEL1_FLAG_2) {
                rwData->timer2 = 20;
            } else {
                rwData->timer2 = 6;
                // @mod Initiate timer4 for z-target.
                timer4 = 6;
            }

            if ((camera->focalActor == &GET_PLAYER(camera->play)->actor) && (camera->mode == CAM_MODE_CHARGE)) {
                rwData->timer2 = 30;
                if (((Player*)camera->focalActor)->transformation == PLAYER_FORM_DEKU) {
                    roData->unk_24 = -1;
                }
            }

            if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
                (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_1)) {
                rwData->timer2 = 1;
                yNormal = 0.8f - ((68.0f / focalActorHeight) * -0.2f);

                bgCamFuncData = (BgCamFuncData*)Camera_GetBgCamOrActorCsCamFuncData(camera, camera->bgCamIndex);

                rwData->unk_20 = bgCamFuncData->rot.x;
                rwData->unk_1E = bgCamFuncData->rot.y;
                rwData->unk_08 = (bgCamFuncData->fov == -1)   ? roData->unk_14
                                 : (bgCamFuncData->fov > 360) ? CAM_RODATA_UNSCALE(bgCamFuncData->fov)
                                                              : bgCamFuncData->fov;
                rwData->unk_00 = (bgCamFuncData->unk_0E == -1)
                                     ? roData->unk_04
                                     : CAM_RODATA_UNSCALE(bgCamFuncData->unk_0E) * focalActorHeight * yNormal;
            } else {
                rwData->unk_08 = roData->unk_14;
                rwData->unk_00 = roData->unk_04;
            }

            rwData->timer3 = roData->unk_24;
            rwData->unk_04 = focalActorPosRot->pos.y - camera->unk_0F0.y;
            rwData->unk_26 = 1;
            camera->animState = 1;
            sCameraInterfaceFlags = roData->interfaceFlags;

            // @mod Reset prev_targeting_held after transition.
            prev_targeting_held = false;

            break;
    }

    // @mod Change behavior for z-target only.
    if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) == PARALLEL1_FLAG_1) {
        Player* player = GET_PLAYER(camera->play);
        bool targeting_held = func_80123434(player) || player->lockOnActor != NULL;

        // @mod Fix camera rotating with player if z-target gets released too fast after transition.
        if (targeting_held &! prev_targeting_held) {
            // @mod Reset timer2 to avoid immediate rotation, if player presses, releases and presses z-target in a very short time-window.
            rwData->timer2 = 6;
            rwData->unk_1E = BINANG_ROT180(camera->focalActorPosRot.rot.y) + roData->unk_22;
        }

        // @mod Maintain vanilla behavior for quitting z-target.
        if ((timer4 == 0) && (!targeting_held)) {
            rwData->timer2 = 0;
        }

        // @mod Decrease timer4 only in cases where timer2 would be decreased.
        if ((timer4 > 0)
        && (rwData->timer3 <= 0)) {
            timer4--;
        }

        prev_targeting_held = targeting_held;
    }

    if (rwData->timer2 != 0) {
        switch (roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) {
            // case PARALLEL1_FLAG_1:
            case (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1):
                rwData->unk_1E = BINANG_ROT180(camera->focalActorPosRot.rot.y) + roData->unk_22;
                // @mod Fix camera rotating with player if z-target gets released too fast after transition.
            case PARALLEL1_FLAG_1:
                rwData->unk_20 = roData->unk_20;
                break;

            case PARALLEL1_FLAG_2:
                rwData->unk_1E = roData->unk_22;
                rwData->unk_20 = roData->unk_20;
                break;

            case (PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1):
                if (rwData->timer3 == 1) {
                    OLib_Vec3fDiffToVecGeo(&sp88, &rwData->unk_10, &spA4);
                    rwData->unk_1E = ((ABS(BINANG_SUB(sp88.yaw, sp80.yaw)) < 0x3A98) || Camera_IsClimbingLedge(camera))
                                         ? sp80.yaw
                                         : sp80.yaw + (s16)((BINANG_SUB(sp88.yaw, sp80.yaw) >> 2) * 3);
                }
                rwData->unk_20 = roData->unk_20;
                break;

            case PARALLEL1_FLAG_3:
                rwData->unk_1E = sp80.yaw;
                rwData->unk_20 = roData->unk_20;
                break;

            case (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_1):
                break;

            default:
                rwData->unk_1E = sp78.yaw + roData->unk_22;
                rwData->unk_20 = roData->unk_20;
                break;
        }
    } else if (roData->interfaceFlags & PARALLEL1_FLAG_5) {
        rwData->unk_1E = BINANG_ROT180(camera->focalActorPosRot.rot.y) + roData->unk_22;
    }

    if (camera->animState == 21) {
        camera->animState = 1;
    } else if (camera->animState == 11) {
        camera->animState = 1;
    }

    spA0 = camera->speedRatio * 0.5f;
    sp9C = camera->speedRatio * 0.2f;

    if (((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
         (PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ||
        ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) == PARALLEL1_FLAG_3) ||
        (roData->interfaceFlags & PARALLEL1_FLAG_5)) {
        camera->rUpdateRateInv = Camera_ScaledStepToCeilF(20.0f, camera->rUpdateRateInv, 0.5f, 0.1f);
        camera->yawUpdateRateInv = Camera_ScaledStepToCeilF(roData->unk_0C, camera->yawUpdateRateInv, 0.5f, 0.1f);
        camera->pitchUpdateRateInv = Camera_ScaledStepToCeilF(20.0f, camera->pitchUpdateRateInv, 0.5f, 0.1f);
    } else {
        camera->rUpdateRateInv = Camera_ScaledStepToCeilF(20.0f, camera->rUpdateRateInv, spA0, 0.1f);
        camera->yawUpdateRateInv = Camera_ScaledStepToCeilF(roData->unk_0C, camera->yawUpdateRateInv, spA0, 0.1f);
        camera->pitchUpdateRateInv = Camera_ScaledStepToCeilF(2.0f, camera->pitchUpdateRateInv, sp9C, 0.1f);
    }

    if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
        (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) {
        camera->yOffsetUpdateRate = Camera_ScaledStepToCeilF(0.1f, camera->yOffsetUpdateRate, spA0, 0.0001f);
        camera->xzOffsetUpdateRate = Camera_ScaledStepToCeilF(0.1f, camera->xzOffsetUpdateRate, sp9C, 0.0001f);
    } else if (roData->interfaceFlags & PARALLEL1_FLAG_7) {
        camera->yOffsetUpdateRate = Camera_ScaledStepToCeilF(0.5f, camera->yOffsetUpdateRate, spA0, 0.0001f);
        camera->xzOffsetUpdateRate = Camera_ScaledStepToCeilF(0.5f, camera->xzOffsetUpdateRate, sp9C, 0.0001f);
    } else {
        camera->yOffsetUpdateRate = Camera_ScaledStepToCeilF(0.05f, camera->yOffsetUpdateRate, spA0, 0.0001f);
        camera->xzOffsetUpdateRate = Camera_ScaledStepToCeilF(0.05f, camera->xzOffsetUpdateRate, sp9C, 0.0001f);
    }

    // TODO: Extra trailing 0 in 0.050f needed?
    camera->fovUpdateRate =
        Camera_ScaledStepToCeilF(0.050f, camera->fovUpdateRate, camera->speedRatio * 0.05f, 0.0001f);

    if (roData->interfaceFlags & PARALLEL1_FLAG_0) {
        tangle = Camera_GetPitchAdjFromFloorHeightDiffs(camera, BINANG_ROT180(sp80.yaw), rwData->unk_26 = 1);
        spA0 = ((1.0f / roData->unk_10));
        spA0 *= 0.6f;
        sp9C = ((1.0f / roData->unk_10) * 0.4f) * (1.0f - camera->speedRatio);
        rwData->unk_1C = Camera_ScaledStepToCeilS(tangle, rwData->unk_1C, spA0 + sp9C, 5);
    } else {
        rwData->unk_1C = 0;
    }

    if (func_800CB950(camera) || (((Player*)camera->focalActor)->stateFlags1 & PLAYER_STATE1_1000) ||
        (((Player*)camera->focalActor)->stateFlags3 & PLAYER_STATE3_100)) {
        rwData->unk_04 = camera->focalActorPosRot.pos.y;
        sp72 = false;
    } else {
        sp72 = true;
    }

    if ((((Player*)camera->focalActor)->stateFlags1 & PLAYER_STATE1_4000) ||
        (((Player*)camera->focalActor)->stateFlags1 & PLAYER_STATE1_4) ||
        ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
         (PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1))) {
        spB0 = spA4;
        spB0.y += ((focalActorHeight * 0.6f) + roData->unk_00);
        Camera_ScaledStepToCeilVec3f(&spB0, at, camera->xzOffsetUpdateRate, camera->yOffsetUpdateRate, 0.0001f);
        Camera_SetFocalActorAtOffset(camera, &focalActorPosRot->pos);
    } else if ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
               (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) {
        spB0 = focalActorPosRot->pos;
        spB0.y += focalActorHeight + roData->unk_00;
        Camera_ScaledStepToCeilVec3f(&spB0, at, camera->xzOffsetUpdateRate, camera->yOffsetUpdateRate, 0.0001f);
        Camera_SetFocalActorAtOffset(camera, &focalActorPosRot->pos);
    } else if (rwData->timer2 != 0) {
        Camera_CalcAtDefault(camera, &sp78, roData->unk_00, 0);
        rwData->timer1 = 200.0f;
    } else if (!(roData->interfaceFlags & PARALLEL1_FLAG_7) && !sp72) {
        Camera_CalcAtForParallel(camera, &sp78, roData->unk_00, roData->unk_08, &rwData->unk_04,
                                 roData->interfaceFlags & (PARALLEL1_FLAG_6 | PARALLEL1_FLAG_0));
        rwData->timer1 = 200.0f;
    } else {
        Camera_CalcAtForScreen(camera, &sp78, roData->unk_00, &rwData->unk_04, rwData->timer1);
        if (rwData->timer1 > 10.0f) {
            rwData->timer1--;
        }
    }

    camera->dist = Camera_ScaledStepToCeilF(rwData->unk_00, camera->dist, 1.0f / camera->rUpdateRateInv, 0.1f);

    if (rwData->timer2 != 0) {
        if (rwData->timer3 <= 0) {
            if (rwData->timer3 == 0) {
                Camera_SetStateFlag(camera, CAM_STATE_DISABLE_MODE_CHANGE);
            }

            tangle = ((rwData->timer2 + 1) * rwData->timer2) >> 1;
            sp90.yaw = sp80.yaw + ((BINANG_SUB(rwData->unk_1E, sp80.yaw) / tangle) * rwData->timer2);
            phi_a0 = ((roData->interfaceFlags & PARALLEL1_FLAG_0) ? BINANG_SUB(rwData->unk_20, rwData->unk_1C)
                                                                  : rwData->unk_20);
            new_var2 = BINANG_SUB(phi_a0, sp80.pitch);
            sp90.pitch = sp80.pitch + ((new_var2 / tangle) * rwData->timer2);
            sp90.r = camera->dist;
            rwData->timer2--;
        } else {
            sp90 = sp80;
            sp90.r = camera->dist;
        }
    } else {
        OLib_Vec3fDiffToVecGeo(&sp90, at, eyeNext);
        sp90.r = camera->dist;

        if (roData->interfaceFlags & PARALLEL1_FLAG_1) {
            sp90.yaw = Camera_ScaledStepToCeilS(rwData->unk_1E, sp78.yaw, 1.0f / camera->yawUpdateRateInv, 0xC8);
        }

        parallelFlagCond = (roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1));

        if (roData->interfaceFlags & PARALLEL1_FLAG_0) {
            phi_a0 = (rwData->unk_20 - rwData->unk_1C);
        } else {
            phi_a0 = rwData->unk_20;
        }

        if (parallelFlagCond == (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) {
            spA0 = CLAMP_MAX(camera->speedRatio, 1.0f);
            phi_a0 = (sp90.pitch * spA0) + (phi_a0 * (1.0f - spA0));
            sp90.pitch = Camera_ScaledStepToCeilS(phi_a0, sp78.pitch, 1.0f / camera->pitchUpdateRateInv, 5);
        } else if (parallelFlagCond != PARALLEL1_FLAG_3) {
            sp90.pitch = Camera_ScaledStepToCeilS(phi_a0, sp78.pitch, 1.0f / camera->pitchUpdateRateInv, 5);
        }

        if (sp90.pitch > DEG_TO_BINANG(79.655f)) {
            sp90.pitch = DEG_TO_BINANG(79.655f);
        }

        if (sp90.pitch < -DEG_TO_BINANG(29.995f)) {
            sp90.pitch = -DEG_TO_BINANG(29.995f);
        }
    }

    if (rwData->timer3 > 0) {
        rwData->timer3--;
    }

    OLib_AddVecGeoToVec3f(eyeNext, at, &sp90);

    if (camera->status == CAM_STATUS_ACTIVE) {
        if ((camera->play->envCtx.skyboxDisabled == 0) || (roData->interfaceFlags & PARALLEL1_FLAG_4)) {
            spB0 = *at;
            if ((((Player*)camera->focalActor)->stateFlags1 & PLAYER_STATE1_4000) ||
                (((Player*)camera->focalActor)->stateFlags1 & PLAYER_STATE1_4) ||
                ((roData->interfaceFlags & (PARALLEL1_FLAG_3 | PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1)) ==
                 (PARALLEL1_FLAG_2 | PARALLEL1_FLAG_1))) {
                spB0.y += focalActorHeight;
            }
            *eye = *eyeNext;
            func_800CBFA4(camera, &spB0, eye, 0);
        } else {
            *eye = *eyeNext;
            func_800CBFA4(camera, at, eye, 3);
        }

        // if (rwData->timer2 != 0) {
        //     sUpdateCameraDirection = true;
        // } else {
        //     sUpdateCameraDirection = false;
        // }

        // @mod Fix camera not updating input dir for the first few frames after transition.
        sUpdateCameraDirection = false;
    }

    camera->fov = Camera_ScaledStepToCeilF(rwData->unk_08, camera->fov, camera->fovUpdateRate, 0.1f);
    camera->roll = Camera_ScaledStepToCeilS(0, camera->roll, 0.5f, 5);
    camera->atLerpStepScale = Camera_ClampLerpScale(camera, sp72 ? roData->unk_1C : roData->unk_18);
    rwData->unk_26 &= ~1;

    return 1;
}