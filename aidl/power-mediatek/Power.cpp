/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>

#ifdef TAP_TO_WAKE_NODE
#include <android-base/file.h>
#endif

#include "Power.h"

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace impl {
namespace mediatek {

#ifdef MODE_EXT
extern bool isDeviceSpecificModeSupported(Mode type, bool* _aidl_return);
extern bool setDeviceSpecificMode(Mode type, bool enabled);
#endif

Power::Power() {
    LOG(INFO) << "Initialize PowerHAL";
}

ndk::ScopedAStatus Power::setMode(Mode type, bool enabled) {
    LOG(INFO) << "Power setMode: " << static_cast<int32_t>(type) << " to: " << enabled;

#ifdef MODE_EXT
    if (setDeviceSpecificMode(type, enabled)) {
        return ndk::ScopedAStatus::ok();
    }
#endif
    switch (type) {
#ifdef TAP_TO_WAKE_NODE
        case Mode::DOUBLE_TAP_TO_WAKE:
            ::android::base::WriteStringToFile(enabled ? "1" : "0", TAP_TO_WAKE_NODE, true);
            break;
#endif
        case Mode::EXPENSIVE_RENDERING:
            if (enabled) {
                if (expensiveRenderingHandle == 0) {
                    expensiveRenderingHandle =
                            wrap_cusLockHint(MTKPOWER_HINT_EXPENSIVE_RENDERING, 0, getpid());
                }
            } else {
                if (expensiveRenderingHandle > 0) {
                    wrap_lockRel(expensiveRenderingHandle);
                    expensiveRenderingHandle = 0;
                }
            }
            break;
        case Mode::LAUNCH:
            if (enabled) {
                if (launchHandle == 0) {
                    launchHandle =
                            wrap_cusLockHint(MTKPOWER_HINT_LAUNCH, kLaunchBoostDuration, getpid());
                }
            } else {
                if (launchHandle > 0) {
                    wrap_lockRel(launchHandle);
                    launchHandle = 0;
                }
            }
            break;
        case Mode::INTERACTIVE:
            if (enabled) {
                wrap_userScnRestoreAll();
            } else {
                wrap_userScnDisableAll();
            }
            break;
        default:
            LOG(INFO) << "Mode " << static_cast<int32_t>(type) << " not supported";
            break;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isModeSupported(Mode type, bool* _aidl_return) {
    LOG(INFO) << "Power isModeSupported: " << static_cast<int32_t>(type);

#ifdef MODE_EXT
    if (isDeviceSpecificModeSupported(type, _aidl_return)) {
        return ndk::ScopedAStatus::ok();
    }
#endif

    switch (type) {
#ifdef TAP_TO_WAKE_NODE
        case Mode::DOUBLE_TAP_TO_WAKE:
#endif
        case Mode::EXPENSIVE_RENDERING:
        case Mode::LAUNCH:
        case Mode::INTERACTIVE:
            *_aidl_return = true;
            break;
        default:
            *_aidl_return = false;
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::setBoost(Boost type, int32_t durationMs) {
    LOG(INFO) << "Power setBoost: " << static_cast<int32_t>(type) << ", duration: " << durationMs;

    switch (type) {
        case Boost::INTERACTION:
            process_interaction_hint(durationMs);
            break;
        default:
            LOG(INFO) << "Boost " << static_cast<int32_t>(type) << " not supported";
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isBoostSupported(Boost type, bool* _aidl_return) {
    LOG(INFO) << "Power isBoostSupported: " << static_cast<int32_t>(type);

    switch (type) {
        case Boost::INTERACTION:
            *_aidl_return = true;
            break;
        default:
            *_aidl_return = false;
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::createHintSession(int32_t, int32_t, const std::vector<int32_t>&, int64_t,
                                            std::shared_ptr<IPowerHintSession>* _aidl_return) {
    *_aidl_return = nullptr;
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Power::getHintSessionPreferredRate(int64_t* outNanoseconds) {
    *outNanoseconds = -1;
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

}  // namespace mediatek
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
