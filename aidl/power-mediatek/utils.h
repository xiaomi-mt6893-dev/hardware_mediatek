/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int wrap_cusLockHint(int hint, int duration, int pid);
int wrap_lockRel(int handle);
void wrap_userScnDisableAll(void);
void wrap_userScnRestoreAll(void);

void process_interaction_hint(int32_t targetDuration);

#ifdef __cplusplus
}
#endif
