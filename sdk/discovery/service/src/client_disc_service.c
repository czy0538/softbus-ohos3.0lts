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

#include "client_disc_service.h"

#include <string.h>
#include "softbus_client_frame_manager.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_type_def.h"
#include "stdbool.h"

static int32_t PublishInfoCheck(const PublishInfo *info)
{
    if ((info->mode != DISCOVER_MODE_PASSIVE) && (info->mode != DISCOVER_MODE_ACTIVE))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "mode is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->medium < AUTO) || (info->medium > COAP))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "medium is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->freq < LOW) || (info->freq > SUPER_HIGH))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "freq is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->capabilityData == NULL) && (info->dataLen != 0))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "data is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if (info->dataLen == 0)
    {
        return SOFTBUS_OK;
    }

    if ((info->dataLen > MAX_CAPABILITYDATA_LEN) ||
        (strlen((char *)(info->capabilityData)) >= MAX_CAPABILITYDATA_LEN))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "data exceeds the maximum length");
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

static int32_t SubscribeInfoCheck(const SubscribeInfo *info)
{
    if ((info->mode != DISCOVER_MODE_PASSIVE) && (info->mode != DISCOVER_MODE_ACTIVE))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "mode is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->medium < AUTO) || (info->medium > COAP))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "medium is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->freq < LOW) || (info->freq > SUPER_HIGH))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "freq is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if ((info->capabilityData == NULL) && (info->dataLen != 0))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "data is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if (info->dataLen == 0)
    {
        return SOFTBUS_OK;
    }

    if ((info->dataLen > MAX_CAPABILITYDATA_LEN) ||
        (strlen((char *)(info->capabilityData)) >= MAX_CAPABILITYDATA_LEN))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "data exceeds the maximum length");
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

int32_t PublishService(const char *packageName, const PublishInfo *info, const IPublishCallback *cb)
{
    if ((packageName == NULL) || (strlen(packageName) >= PKG_NAME_SIZE_MAX) || (info == NULL) || (cb == NULL))
    {
        return SOFTBUS_INVALID_PARAM;
    }

    if (InitSoftBus(packageName) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "init softbus err");
        return SOFTBUS_DISCOVER_NOT_INIT;
    }

    if (CheckPackageName(packageName) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "check packageName failed");
        return SOFTBUS_INVALID_PARAM;
    }

    if (PublishInfoCheck(info) != SOFTBUS_OK)
    {
        return SOFTBUS_INVALID_PARAM;
    }

    return PublishServiceInner(packageName, info, cb);
}

int32_t UnPublishService(const char *packageName, int32_t publishId)
{
    if ((packageName == NULL) || (strlen(packageName) >= PKG_NAME_SIZE_MAX))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "invalid packageName");
        return SOFTBUS_INVALID_PARAM;
    }

    if (CheckPackageName(packageName) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "check packageName failed");
        return SOFTBUS_INVALID_PARAM;
    }

    return UnPublishServiceInner(packageName, publishId);
}

int32_t StartDiscovery(const char *packageName, const SubscribeInfo *info, const IDiscoveryCallback *cb)
{
    //
    if ((packageName == NULL) || (strlen(packageName) >= PKG_NAME_SIZE_MAX) || (info == NULL) || (cb == NULL))
    {
        return SOFTBUS_INVALID_PARAM;
    }

    /*
    *1.把pkgName赋给g_pkgName
    *2.初始化一堆变量，设置一堆回调函数，最终返回消息softbus sdk frame init success
    */
    if (InitSoftBus(packageName) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "init softbus err");
        return SOFTBUS_DISCOVER_NOT_INIT;
    }
    //把g_pkgName赋给了clientPkgName,检查clientPkgName与传入的packageName是否一样，不知道图个啥。
    if (CheckPackageName(packageName) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "check packageName failed");
        return SOFTBUS_INVALID_PARAM;
    }
    //检测一些参数
    if (SubscribeInfoCheck(info) != SOFTBUS_OK)
    {
        return SOFTBUS_INVALID_PARAM;
    }
    //将我们的回调函数注册给g_discInfo,调用ServerIpcStartDiscovery，最终调用DiscIpcStartDiscovery
    return StartDiscoveryInner(packageName, info, cb);
}

int32_t StopDiscovery(const char *packageName, int32_t subscribeId)
{
    if ((packageName == NULL) || (strlen(packageName) >= PKG_NAME_SIZE_MAX))
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "invalid packageName");
        return SOFTBUS_INVALID_PARAM;
    }

    if (CheckPackageName(packageName) != SOFTBUS_OK)
    {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "check packageName failed");
        return SOFTBUS_INVALID_PARAM;
    }

    return StopDiscoveryInner(packageName, subscribeId);
}
