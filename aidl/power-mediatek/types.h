/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define PERF_HAL_PATH "libpowerhal.so"

#define MTKPOWER_HINT_LAUNCH 11
#define MTKPOWER_HINT_INTERACTIVE 13
#define MTKPOWER_HINT_SUSTAINED_PERFORMANCE 8
#define MTKPOWER_HINT_FIXED_PERFORMANCE 9
#define MTKPOWER_HINT_EXPENSIVE_RENDERING 12
#define MTKPOWER_HINT_APP_TOUCH 25
#define MTKPOWER_HINT_UX_SCROLLING 43

#ifdef __cplusplus
extern "C" {
#endif

static const int32_t kDefaultTouchBoostDuration = 120; /* ms */
static const int32_t kLaunchBoostDuration = 2000;      /* ms */
static const int32_t kMaxInteractiveDuration = 5000;   /* ms */
static const int32_t kMinInteractiveDuration = 400;    /* ms */

static void* mtkopt_handle;
static int (*init)(int flags);
static int (*lockAcq)(int* resources, int size, int handle, int duration, int extra1, int extra2);
static int (*lockRel)(int handle);
static int (*cusLockHint)(int hint, int duration, int pid);
static int (*notifyAppState)(const char* package_name, const char* activity_name, int state,
                             int pid, int uid);
static int (*userGetCapability)(void);
static void (*userScnDisableAll)(void);
static void (*userScnRestoreAll)(void);
static int (*setSysInfo)(int info_type, const char* data);

#ifdef __cplusplus
}
#endif
