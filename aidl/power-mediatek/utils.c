/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>
#include <time.h>

#define LOG_TAG "Mediatek PowerHAL"
#include <log/log.h>

#include "types.h"
#include "utils.h"

static struct timespec last_interaction_ts;
static int last_interaction_duration = 0;
static int last_interaction_handle = 0;

static void* get_mtkopt_handle(void) {
    void* handle = NULL;

    dlerror();

    handle = dlopen(PERF_HAL_PATH, RTLD_NOW);
    if (!handle) {
        ALOGE("Unable to open %s: %s\n", PERF_HAL_PATH, dlerror());
    }

    return handle;
}

static void __attribute__((constructor)) initialize(void) {
    mtkopt_handle = get_mtkopt_handle();

    if (!mtkopt_handle) ALOGE("Failed to get mtkopt handle.\n");

    init = dlsym(mtkopt_handle, "libpowerhal_Init");
    if (!init) {
        ALOGE("Unable to get libpowerhal_Init function handle: %s\n", dlerror());
    }

    cusLockHint = dlsym(mtkopt_handle, "libpowerhal_CusLockHint");
    if (!cusLockHint) {
        ALOGE("Unable to get libpowerhal_CusLockHint function handle: %s\n", dlerror());
    }

    lockRel = dlsym(mtkopt_handle, "libpowerhal_LockRel");
    if (!lockRel) {
        ALOGE("Unable to get libpowerhal_LockRel function handle: %s\n", dlerror());
    }

    userScnDisableAll = dlsym(mtkopt_handle, "libpowerhal_UserScnDisableAll");
    if (!userScnDisableAll) {
        ALOGE("Unable to get libpowerhal_UserScnDisableAll function handle: %s\n", dlerror());
    }

    userScnRestoreAll = dlsym(mtkopt_handle, "libpowerhal_UserScnRestoreAll");
    if (!userScnRestoreAll) {
        ALOGE("Unable to get libpowerhal_UserScnRestoreAll function handle: %s\n", dlerror());
    }

    if (init) {
        init(1);
        ALOGI("libpowerhal successfully loaded");
    }
}

static void __attribute__((destructor)) cleanup(void) {
    if (last_interaction_handle > 0) {
        wrap_lockRel(last_interaction_handle);
    }

    if (mtkopt_handle) {
        if (dlclose(mtkopt_handle)) ALOGE("Error occurred while closing powerhal library.");
    }
}

int wrap_cusLockHint(int hint, int duration, int pid) {
    if (!cusLockHint) {
        ALOGE("cusLockHint is NULL");
        return -1;
    }
    return cusLockHint(hint, duration, pid);
}

int wrap_lockRel(int handle) {
    if (!lockRel) {
        ALOGE("lockRel is NULL");
        return -1;
    }
    return lockRel(handle);
}

void wrap_userScnDisableAll(void) {
    if (!userScnDisableAll) {
        ALOGE("userScnDisableAll is NULL");
        return;
    }
    userScnDisableAll();
}

void wrap_userScnRestoreAll(void) {
    if (!userScnRestoreAll) {
        ALOGE("userScnRestoreAll is NULL");
        return;
    }
    userScnRestoreAll();
}

static long long timespec_diff_us(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000000LL + (b.tv_nsec - a.tv_nsec) / 1000LL;
}

void process_interaction_hint(int32_t targetDuration) {
    struct timespec now;
    long long elapsed_us, remaining_us;
    int32_t duration_ms;

    clock_gettime(CLOCK_MONOTONIC, &now);

    // if targetDuration is 0, we perform a touch boost instead.
    if (targetDuration == 0) {
        wrap_cusLockHint(MTKPOWER_HINT_APP_TOUCH, kDefaultTouchBoostDuration, getpid());
        return;
    }

    // clamp duration
    duration_ms = kMinInteractiveDuration;
    if (targetDuration > duration_ms) {
        duration_ms = targetDuration;
        if (duration_ms > kMaxInteractiveDuration) duration_ms = kMaxInteractiveDuration;
    }

    // debounce repeated short boosts
    elapsed_us = timespec_diff_us(last_interaction_ts, now);
    if (elapsed_us < 250000 && duration_ms <= kMinInteractiveDuration) {
        return;
    }

    // don't hint if previous hint's duration covers this hint's duration
    if (duration_ms <= last_interaction_duration) {
        remaining_us = (last_interaction_duration - duration_ms) * 1000LL;
        if (elapsed_us <= remaining_us) return;
    }

    // drop previous interaction handle
    if (last_interaction_handle > 0) {
        wrap_lockRel(last_interaction_handle);
        last_interaction_handle = 0;
    }

    // new interaction
    last_interaction_ts = now;
    last_interaction_duration = duration_ms;

    // new handle
    last_interaction_handle = wrap_cusLockHint(MTKPOWER_HINT_UX_SCROLLING, duration_ms, getpid());
}
