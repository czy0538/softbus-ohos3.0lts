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

#include "bus_center_server_proxy.h"

#include "securec.h"

#include "iproxy_client.h"
#include "samgr_lite.h"
#include "serializer.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_ipc_def.h"
#include "softbus_log.h"
#include "softbus_mem_interface.h"
#include "softbus_os_interface.h"

typedef enum {
    GET_ALL_ONLINE_NODE_INFO = 0,
    GET_LOCAL_DEVICE_INFO,
    GET_NODE_KEY_INFO,
} FunID;
typedef struct {
    FunID id;
    int ret;
    void* data;
} Reply;

static IClientProxy *g_serverProxy = NULL;

static int ClientBusCenterResultCb(IOwner owner, int code, IpcIo *reply)
{
    Reply *info = (Reply *)owner;
    uint32_t infoSize;
    switch (info->id) {
        case GET_ALL_ONLINE_NODE_INFO:
            info->ret = IpcIoPopInt32(reply);
            if (info->ret > 0) {
                info->data = (void *)IpcIoPopFlatObj(reply, &infoSize);
            }
            break;
        case GET_LOCAL_DEVICE_INFO:
        case GET_NODE_KEY_INFO:
            info->data = (void *)IpcIoPopFlatObj(reply, &infoSize);
            break;
        default:
            LOG_ERR("unknown funid");
            return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t BusCenterServerProxyInit(void)
{
    if (g_serverProxy != NULL) {
        LOG_INFO("server proxy has initialized.");
        return SOFTBUS_OK;
    }

    IUnknown *iUnknown = NULL;
    int ret;

    LOG_INFO("bus center start get server proxy");
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
    LOG_INFO("bus center get server proxy ok");
    return SOFTBUS_OK;
}

int ServerIpcGetAllOnlineNodeInfo(const char *pkgName, void **info, uint32_t infoTypeLen, int32_t *infoNum)
{
    if (info == NULL || infoNum == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (g_serverProxy == NULL) {
        LOG_ERR("ServerIpcGetAllOnlineNodeInfo g_serverProxy is NULL!\n");
        return SOFTBUS_ERR;
    }
    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushUint32(&request, infoTypeLen);
    Reply reply = {0};
    reply.id = GET_ALL_ONLINE_NODE_INFO;
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_GET_ALL_ONLINE_NODE_INFO, &request, &reply,
        ClientBusCenterResultCb);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("GetAllOnlineNodeInfo invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    *infoNum = reply.ret;
    int32_t infoSize = (*infoNum) * (int32_t)infoTypeLen;
    *info = NULL;
    if (infoSize > 0) {
        if (reply.data == NULL) {
            LOG_ERR("GetAllOnlineNodeInfo read node info failed!");
            return SOFTBUS_ERR;
        }
        *info = SoftBusMalloc(infoSize);
        if (*info == NULL) {
            LOG_ERR("GetAllOnlineNodeInfo malloc failed!");
            return SOFTBUS_ERR;
        }
        if (memcpy_s(*info, infoSize, reply.data, infoSize) != EOK) {
            LOG_ERR("GetAllOnlineNodeInfo copy node info failed!");
            SoftBusFree(*info);
            return SOFTBUS_ERR;
        }
    }
    return SOFTBUS_OK;
}

int32_t ServerIpcGetLocalDeviceInfo(const char *pkgName, void *info, uint32_t infoTypeLen)
{
    if (info == NULL) {
        return SOFTBUS_ERR;
    }
    if (g_serverProxy == NULL) {
        LOG_ERR("ServerIpcGetLocalDeviceInfo g_serverProxy is nullptr!\n");
        return SOFTBUS_ERR;
    }

    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushUint32(&request, infoTypeLen);
    Reply reply = {0};
    reply.id = GET_LOCAL_DEVICE_INFO;
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_GET_LOCAL_DEVICE_INFO, &request, &reply,
        ClientBusCenterResultCb);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("GetLocalDeviceInfo invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    if (reply.data == NULL) {
        LOG_ERR("GetLocalDeviceInfo read node info failed!");
        return SOFTBUS_ERR;
    }
    if (memcpy_s(info, infoTypeLen, reply.data, infoTypeLen) != EOK) {
        LOG_ERR("GetLocalDeviceInfo copy node info failed!");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t ServerIpcGetNodeKeyInfo(const char *pkgName, const char *networkId, int key, unsigned char *buf, uint32_t len)
{
    if (networkId == NULL || buf == NULL) {
        LOG_ERR("params are nullptr!");
        return SOFTBUS_ERR;
    }
    if (g_serverProxy == NULL) {
        LOG_ERR("ServerIpcGetNodeKeyInfo g_serverProxy is nullptr!\n");
        return SOFTBUS_ERR;
    }

    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushString(&request, networkId);
    IpcIoPushInt32(&request, key);
    IpcIoPushInt32(&request, len);
    Reply reply = {0};
    reply.id = GET_NODE_KEY_INFO;
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_GET_NODE_KEY_INFO, &request, &reply,
        ClientBusCenterResultCb);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("GetNodeKeyInfo invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    if (reply.data == NULL) {
        LOG_ERR("GetNodeKeyInfo read retBuf failed!");
        return SOFTBUS_ERR;
    }
    if (memcpy_s(buf, len, reply.data, len) != EOK) {
        LOG_ERR("GetNodeKeyInfo copy node key info failed");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int ServerIpcJoinLNN(const char *pkgName, void *addr, unsigned int addrTypeLen)
{
    LOG_INFO("join Lnn ipc client push.");
    if (addr == NULL || pkgName == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (g_serverProxy == NULL) {
        LOG_ERR("ServerIpcJoinLNN g_serverProxy is nullptr!\n");
        return SOFTBUS_ERR;
    }
    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushUint32(&request, addrTypeLen);
    IpcIoPushFlatObj(&request, addr, addrTypeLen);
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_JOIN_LNN, &request, NULL, NULL);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("join Lnn invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int ServerIpcLeaveLNN(const char *pkgName, const char *networkId)
{
    LOG_INFO("leave Lnn ipc client push.");
    if (pkgName == NULL || networkId == NULL) {
        LOG_ERR("Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (g_serverProxy == NULL) {
        LOG_ERR("ServerIpcLeaveLNN g_serverProxy is nullptr!\n");
        return SOFTBUS_ERR;
    }
    uint8_t data[MAX_SOFT_BUS_IPC_LEN] = {0};
    IpcIo request = {0};
    IpcIoInit(&request, data, MAX_SOFT_BUS_IPC_LEN, 0);
    IpcIoPushString(&request, pkgName);
    IpcIoPushString(&request, networkId);
    /* asynchronous invocation */
    int32_t ans = g_serverProxy->Invoke(g_serverProxy, SERVER_LEAVE_LNN, &request, NULL, NULL);
    if (ans != SOFTBUS_OK) {
        LOG_ERR("leave Lnn invoke failed[%d].", ans);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}