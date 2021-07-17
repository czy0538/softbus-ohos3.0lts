/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRANS_PENDING_PKT_H
#define TRANS_PENDING_PKT_H

#include <pthread.h>
#include <stdint.h>

#include "common_list.h"
#include "softbus_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

enum {
    PENDING_TYPE_PROXY = 0,
    PENDING_TYPE_DIRECT,
    PENDING_TYPE_BUTT,
};

typedef struct {
    ListNode node;
    pthread_cond_t cond;
    pthread_mutex_t lock;
    int32_t channelId;
    int32_t seq;
    bool finded;
} PendingPktInfo;

int32_t PendingInit(int type);
void PendingDeinit(int type);
int32_t ProcPendingPacket(int32_t channelId, int32_t seqNum, int type);
int32_t SetPendingPacket(int32_t channelId, int32_t seqNum, int type);
int32_t DelPendingPacket(int32_t channelId, int type);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif // TRANS_PENDING_PKT_H

