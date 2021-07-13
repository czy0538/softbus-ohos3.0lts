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

#include "bus_center_manager.h"

#include <stdint.h>
#include <stdlib.h>

#include "lnn_discovery_manager.h"
#include "lnn_distributed_net_ledger.h"
#include "lnn_event_monitor.h"
#include "lnn_lane_info.h"
#include "lnn_local_net_ledger.h"
#include "lnn_net_builder.h"
#include "lnn_sync_item_info.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_utils.h"

void __attribute__ ((weak)) LnnLanesInit(void)
{
}

int32_t BusCenterServerInit(void)
{
    if (LnnInitLocalLedger() != SOFTBUS_OK) {
        LOG_ERR("init local net ledger fail!");
        return SOFTBUS_ERR;
    }
    if (LnnInitDistributedLedger() != SOFTBUS_OK) {
        LOG_ERR("init distributed net ledger fail!");
        return SOFTBUS_ERR;
    }
    if (LnnInitSyncLedgerItem() != SOFTBUS_OK) {
        LOG_ERR("init sync ledger item fail!");
        return SOFTBUS_ERR;
    }
    if (LnnInitEventMonitor() != SOFTBUS_OK) {
        LOG_ERR("init event monitor failed");
        return SOFTBUS_ERR;
    }
    if (LnnInitDiscoveryManager() != SOFTBUS_OK) {
        LOG_ERR("init lnn discovery manager fail!");
        return SOFTBUS_ERR;
    }
    if (LnnInitNetBuilder() != SOFTBUS_OK) {
        LOG_ERR("init net builder fail!");
        return SOFTBUS_ERR;
    }
    LnnLanesInit();
    LOG_INFO("bus center server init ok");
    return SOFTBUS_OK;
}

void BusCenterServerDeinit(void)
{
    LnnDeinitLocalLedger();
    LnnDeinitDistributedLedger();
    LnnDeinitNetBuilder();
    LnnDeinitSyncLedgerItem();
    LnnDeinitEventMonitor();
    LOG_INFO("bus center server deinit");
}
