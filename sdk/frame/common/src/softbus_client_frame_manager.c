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

#include "softbus_client_frame_manager.h"

#include <securec.h>

#include <string.h>
#include "client_bus_center_manager.h"
#include "client_disc_manager.h"
#include "client_trans_session_manager.h"
#include "softbus_client_event_manager.h"
#include "softbus_client_stub_interface.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_feature_config.h"
#include "softbus_log.h"

static bool g_isInited = false;
static pthread_mutex_t g_isInitedLock = PTHREAD_MUTEX_INITIALIZER;
static char g_pkgName[PKG_NAME_SIZE_MAX] = {0};

static void ClientModuleDeinit(void)
{
    EventClientDeinit();
    BusCenterClientDeinit();
    TransClientDeinit();
    DiscClientDeinit();
}

static int32_t ClientModuleInit()
{
    //设置一堆默认的参数，包括g_tranConfig等，暂时没有具体看参数内容
    SoftbusConfigInit();

    //初始化g_observerList,一种SoftBusList
    if (EventClientInit() == SOFTBUS_ERR)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "init event manager failed");
        goto ERR_EXIT;
    }

    //初始化g_busCenterClient中的各个SoftBusList,初始化BusCenterServerProxy
    if (BusCenterClientInit() == SOFTBUS_ERR)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "init bus center failed");
        goto ERR_EXIT;
    }

    //g_discInfo初始化，DiscServerProxy初始化
    if (DiscClientInit() == SOFTBUS_ERR)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "init service manager failed");
        goto ERR_EXIT;
    }

    //g_idFlagBitmap、g_clientSessionServerList、TransServerProxy初始化，注册各种回调函数
    if (TransClientInit() == SOFTBUS_ERR)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "init connect manager failed");
        goto ERR_EXIT;
    }

    return SOFTBUS_OK;

ERR_EXIT:
    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus sdk frame init failed.");
    ClientModuleDeinit();
    return SOFTBUS_ERR;
}

int32_t InitSoftBus(const char *pkgName)
{
    //参数检查
    if (pkgName == NULL || strlen(pkgName) >= PKG_NAME_SIZE_MAX)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "init softbus sdk fail.");
        return SOFTBUS_ERR;
    }
    //若已经初始化完成，直接退出
    if (g_isInited == true)
    {
        return SOFTBUS_OK;
    }
    //加锁
    if (pthread_mutex_lock(&g_isInitedLock) != 0)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "lock failed");
        return SOFTBUS_LOCK_ERR;
    }
    //加锁后再次检查，若已经被初始化则释放锁
    if (g_isInited == true)
    {
        pthread_mutex_unlock(&g_isInitedLock);
        return SOFTBUS_OK;
    }

    //把pkgName赋给g_pkgName
    if (strcpy_s(g_pkgName, sizeof(g_pkgName), pkgName) != EOK)
    {
        pthread_mutex_unlock(&g_isInitedLock);
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "strcpy_s failed.");
        return SOFTBUS_MEM_ERR;
    }
    // 详见该函数的注释，一句话总结：初始化一堆变量，设置一堆回调函数
    if (ClientModuleInit() != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "ctx init fail");
        pthread_mutex_unlock(&g_isInitedLock);
        return SOFTBUS_ERR;
    }
    //serverProxyFrame等初始化，没看明白在干嘛
    if (ClientStubInit() != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "service init fail");
        pthread_mutex_unlock(&g_isInitedLock);
        return SOFTBUS_ERR;
    }
    //标记初始化成功
    g_isInited = true;
    pthread_mutex_unlock(&g_isInitedLock);
    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "softbus sdk frame init success.");
    return SOFTBUS_OK;
}

//把 g_pkgName赋给 name
int32_t GetSoftBusClientName(char *name, uint32_t len)
{
    if (name == NULL || len < PKG_NAME_SIZE_MAX)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "invalid param");
        return SOFTBUS_ERR;
    }

    if (strncpy_s(name, len, g_pkgName, strlen(g_pkgName)) != EOK)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "strcpy fail");
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

int32_t CheckPackageName(const char *pkgName)
{
    char clientPkgName[PKG_NAME_SIZE_MAX] = {0};
    //把 g_pkgName赋给了clientPkgName
    if (GetSoftBusClientName(clientPkgName, PKG_NAME_SIZE_MAX) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "GetSoftBusClientName err");
        return SOFTBUS_INVALID_PKGNAME;
    }
    //然后检查clientPkgName和pkgName是否一致，packageName为传入的
    if (strcmp(clientPkgName, pkgName) != 0)
    {
        return SOFTBUS_INVALID_PKGNAME;
    }
    return SOFTBUS_OK;
}
