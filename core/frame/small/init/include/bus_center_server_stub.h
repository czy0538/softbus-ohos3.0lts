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

#ifndef BUS_CENTER_SERVER_STUB_H
#define BUS_CENTER_SERVER_STUB_H

#include <stdint.h>
#include "serializer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int32_t ServerJoinLNN(void *origin, IpcIo *req, IpcIo *reply);
int32_t ServerLeaveLNN(void *origin, IpcIo *req, IpcIo *reply);
int32_t ServerGetAllOnlineNodeInfo(void *origin, IpcIo *req, IpcIo *reply);
int32_t ServerGetLocalDeviceInfo(void *origin, IpcIo *req, IpcIo *reply);
int32_t ServerGetNodeKeyInfo(void *origin, IpcIo *req, IpcIo *reply);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif //