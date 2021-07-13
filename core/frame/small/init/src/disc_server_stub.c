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

#include "disc_server_stub.h"

#include "disc_serializer.h"
#include "discovery_service.h"
#include "liteipc_adapter.h"
#include "softbus_def.h"
#include "softbus_disc_server.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_permission.h"

int32_t ServerPublishService(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("publish service ipc server pop.");
    if (req == NULL || reply == NULL) {
        LOG_ERR("invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    size_t len;
    uint32_t size;
    const char *pkgName = (const char*)IpcIoPopString(req, &len);
    PublishSerializer *info = (PublishSerializer*)IpcIoPopFlatObj(req, &size);
    if (info == NULL) {
        LOG_ERR("ipc pop is null.");
        return SOFTBUS_ERR;
    }
    char *capability = (char*)IpcIoPopString(req, &len);
    unsigned char *capabilityData = (unsigned char*)IpcIoPopString(req, &len);
    PublishInfo publishInfo = {
        .capability = capability,
        .capabilityData = capabilityData,
        .dataLen = info->commonSerializer.dataLen,
        .freq = info->commonSerializer.freq,
        .medium = info->commonSerializer.medium,
        .mode = info->commonSerializer.mode,
        .publishId = info->commonSerializer.id.publishId
    };
    int32_t callingUid = GetCallingUid(origin);
    if (!CheckDiscPermission(callingUid, pkgName)) {
        LOG_ERR("publish service no permission.");
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = DiscIpcPublishService(pkgName, &publishInfo);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("publish service failed.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t ServerUnPublishService(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("unpublish service ipc server pop.");
    size_t len;
    const char *pkgName = (const char*)IpcIoPopString(req, &len);
    int32_t publishId = IpcIoPopInt32(req);
    int32_t callingUid = GetCallingUid(origin);
    if (!CheckDiscPermission(callingUid, pkgName)) {
        LOG_ERR("publish service no permission.");
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = DiscIpcUnPublishService(pkgName, publishId);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("unpublish service failed.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t ServerStartDiscovery(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("start discovery ipc server pop.");
    size_t len;
    uint32_t size;
    const char *pkgName = (const char *)IpcIoPopString(req, &len);
    SubscribeSerializer *info = (SubscribeSerializer *)IpcIoPopFlatObj(req, &size);
    if (info == NULL) {
        LOG_ERR("ipc pop is null.");
        return SOFTBUS_ERR;
    }
    char *capability = (char *)IpcIoPopString(req, &len);
    unsigned char *capabilityData = (unsigned char *)IpcIoPopString(req, &len);
    SubscribeInfo subscribeInfo = {
        .capability = capability,
        .capabilityData = capabilityData,
        .dataLen = info->commonSerializer.dataLen,
        .freq = info->commonSerializer.freq,
        .medium = info->commonSerializer.medium,
        .mode = info->commonSerializer.mode,
        .subscribeId = info->commonSerializer.id.subscribeId,
        .isSameAccount = info->isSameAccount,
        .isWakeRemote = info->isWakeRemote
    };
    int32_t callingUid = GetCallingUid(origin);
    if (!CheckDiscPermission(callingUid, pkgName)) {
        LOG_ERR("publish service no permission.");
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = DiscIpcStartDiscovery(pkgName, &subscribeInfo);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("start discovery failed.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t ServerStopDiscovery(void *origin, IpcIo *req, IpcIo *reply)
{
    LOG_INFO("stop discovery ipc server pop.");
    size_t len;
    const char *pkgName = (const char *)IpcIoPopString(req, &len);
    int32_t subscribeId = IpcIoPopInt32(req);
    int32_t callingUid = GetCallingUid(origin);
    if (!CheckDiscPermission(callingUid, pkgName)) {
        LOG_ERR("publish service no permission.");
        return SOFTBUS_PERMISSION_DENIED;
    }
    int32_t ret = DiscIpcStopDiscovery(pkgName, subscribeId);
    if (ret != SOFTBUS_OK) {
        LOG_ERR("stop discovery failed.");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}
