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

#include "disc_server_proxy.h"

#include "disc_serializer.h"
#include "iproxy_client.h"
#include "samgr_lite.h"
#include "serializer.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_ipc_def.h"
#include "softbus_log.h"
#include "softbus_os_interface.h"

static IClientProxy *g_serverProxy = NULL;

int32_t DiscServerProxyInit(void)
{
    if (g_serverProxy != NULL) {
        LOG_INFO("server proxy has initialized.");
        return SOFTBUS_OK;
    }

    IUnknown *iUnknown = NULL;
    int ret;

    LOG_INFO("disc start get server proxy");
    while (g_serverProxy == NULL) {
        iUnknown = SAMGR_GetInstance()->GetDefaultFeatureApi(SOFTBUS_SERVICE);
        if (iUnknown == NULL) {
            SoftBusSleepMs(WAIT_SERVER_READY_INTERVAL);
            continue;
        }

        ret = iUnknown->QueryInterface(iUnknown, CLIENT_PROXY_VER, (void **)&g_serverProxy);
        if (ret != EC_SUCCESS || g_serverProxy == NULL) {
            LOG_ERR("QueryInterface failed [%d]", ret);
            SoftBusSleepMs(WAIT_SERVER_READY_INTERVAL);
            continue;
        }
    }
    LOG_INFO("disc get server proxy ok");
    return SOFTBUS_OK;
}

int ServerIpcPublishService(const char *pkgName, const PublishInfo *info)
{
    LOG_INFO("publish service ipc client push.");
    if (pkgName == NULL || info == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    DiscSerializer serializer = {
        .dataLen = info->dataLen,
        .freq = info->freq,
        .medium = info->medium,
        .mode = info->mode,
        .id.publishId = info->publishId
    };
    PublishSerializer publishSerializer = {
        .commonSerializer = serializer
    };
    IpcIoPushFlatObj(&request, (void*)&publishSerializer, sizeof(PublishSerializer));
    IpcIoPushString(&request, info->capability);
    IpcIoPushString(&request, info->capabilityData);
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_PUBLISH_SERVICE, &request, NULL, NULL);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("publish service invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int ServerIpcUnPublishService(const char *pkgName, int publishId)
{
    LOG_INFO("unpublish service ipc client push.");
    if (pkgName == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushInt32(&request, publishId);
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_UNPUBLISH_SERVICE, &request, NULL, NULL);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("unpublish service invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int ServerIpcStartDiscovery(const char *pkgName, const SubscribeInfo *info)
{
    LOG_INFO("start discovery ipc client push.");
    if (pkgName == NULL || info == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    DiscSerializer serializer = {
        .dataLen = info->dataLen,
        .freq = info->freq,
        .medium = info->medium,
        .mode = info->mode,
        .id.subscribeId = info->subscribeId
    };
    SubscribeSerializer subscribeSerializer = {
        .commonSerializer = serializer,
        .isSameAccount = info->isSameAccount,
        .isWakeRemote = info->isWakeRemote
    };
    IpcIoPushFlatObj(&request, (void*)&subscribeSerializer, sizeof(SubscribeSerializer));
    IpcIoPushString(&request, info->capability);
    IpcIoPushString(&request, info->capabilityData);
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_START_DISCOVERY, &request, NULL, NULL);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("start discovery invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int ServerIpcStopDiscovery(const char *pkgName, int subscribeId)
{
    LOG_INFO("stop discovery ipc client push.");
    if (pkgName == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushInt32(&request, subscribeId);
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_STOP_DISCOVERY, &request, NULL, NULL);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("stop discovery invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}