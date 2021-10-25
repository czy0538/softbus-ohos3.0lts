# communication_dsoftbus分析

## 目录结构

![image-20211013193738075](https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202110140957454.png)



## 运行模式分析

dsoftbus采用了跟lite仓库完全不同的模式。在用户调用软总线功能前，需要先启动`softbus_server`，该进程一般跟随系统启动，在轻量级系统上已经测试过，如果没有该进程，软总线各项功能无法正常运行。

## 测试系统分析

按照代码模块的分析，我们可以推测出test模块下的各个文件夹的含义。OpenHarmony使用的是谷歌的gtest测试框架。

一个标准测试函数如下

```c
对待测内容的描述
/**
 * @tc.name: PublishServiceTest002
 * @tc.desc: Verify normal case
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F是一个封装的宏，调用了Disc_Test
其描述如下
//#define HWTEST_F(className, funcName, level) void className::funcName()
HWTEST_F(Disc_Test, PublishServiceTest002, TestSize.Level1)
{
    int ret;

    g_pInfo.publishId = GetPublishId();
    ret = PublishService(g_pkgName, &g_pInfo, &g_publishCb);
    EXPECT_TRUE(ret == 0);

    g_pInfo1.publishId = GetPublishId();
    ret = PublishService(g_pkgName, &g_pInfo1, &g_publishCb);
    EXPECT_TRUE(ret == 0);
}
```

## 命名模式分析

- 

## 一些不知道该分给谁但很重要的记录

- **PublishService**和**StartDiscovery**本体行为完全一致，区别在于最后的函数调用了不同的**ServerIpc**

- 解耦方式与lite仓库不同，在编译是调用不同文件夹下的同名函数来区分，在调用处不做区分

  ![image-20211014105319212](https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202110141053325.png)
  
- 在分析`core\discovery\manager\src\disc_manager.c DiscInterfaceByMedium ` 函数负责根据不同的medium调用不同的函数，注意到AUTO模式也会调用COAP。

  ![image-20211016200906996](https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202110162009108.png)

## StartDiscovery分析

一次发现成功的日记

```c++
OHOS # start discoveryTask
01-01 00:20:24.065 16 100 I 015C0/dsoftbus_standard: [LNN]NodeStateCbCount is 10
01-01 00:20:24.066 16 100 I 015C0/dsoftbus_standard: [LNN]bus center start get server proxy
01-01 00:20:24.066 16 100 I 01800/Samgr: Initialize Client Registry!
01-01 00:20:24.066 3 35 D 01800/Samgr: Judge Auth<softbus_service, (null)> ret:0
01-01 00:20:24.066 3 35 D 01800/Samgr: Find Feature<softbus_service, (null)> id<83, 0> ret:0
01-01 00:20:24.067 16 100 I 01800/Samgr: Create remote sa proxy[0x227ef1e0]<softbus_service, (null)>!
01-01 00:20:24.067 16 100 I 015C0/dsoftbus_standard: [LNN]bus center get server proxy ok
01-01 00:20:24.067 16 100 I 015C0/dsoftbus_standard: [LNN]BusCenterClientInit init OK!
01-01 00:20:24.067 16 100 I 015C0/dsoftbus_standard: [DISC]disc start get server proxy
01-01 00:20:24.067 16 100 I 015C0/dsoftbus_standard: [DISC]disc get server proxy ok
01-01 00:20:24.067 16 100 I 015C0/dsoftbus_standard: [DISC]Init success
[czy_test]ret is:0
[czy_test]waiting!!!
4 nStackXDFinder: NSTACKX_SetFilterCapability:[514] :Set Filter Capability

[czy_test]TestDiscoverySuccess//调用了发现成功的回调函数

3 nStackXCoAP: CoapServiceDiscoverStop:[634] :clear device list backup
4 nStackXCoAP: CoapServiceDiscoverStopInner:[785] :device discover stopped
2 nStackXDFinder: BackupDeviceDB:[1180] :clear backupDB error
3 nStackXCoAP: CoapServiceDiscoverInner:[727] :clear device list
2 nStackXDev: GetInterfaceInfo:[58] :ioctl fail, errno = 38
2 nStackXCoAP: CoapServiceDiscoverInner:[732] :failed to post service discover request
write file switch /storage/data/log/hilog1.txt

01-01 00:20:24.145 16 100 I 015C0/dsoftbus_standard: [TRAN]trans start get server proxy
01-01 00:20:24.145 16 100 I 015C0/dsoftbus_standard: [TRAN]trans get server proxy ok
01-01 00:20:24.151 16 100 I 015C0/dsoftbus_standard: [TRAN]init tcp direct channel success.
01-01 00:20:24.159 16 100 I 015C0/dsoftbus_standard: [TRAN]trans udp channel manager init success.
01-01 00:20:24.159 16 100 I 015C0/dsoftbus_standard: [TRAN]init succ
01-01 00:20:24.160 16 100 I 01800/Samgr: Bootstrap core services(count:0).
01-01 00:20:24.160 16 100 I 01800/Samgr: Initialized all core system services!
01-01 00:20:24.160 16 100 I 01800/Samgr: Goto next boot step return code:-9
01-01 00:20:24.160 16 100 I 015C0/dsoftbus_standard: [COMM]start get client proxy
01-01 00:20:24.160 16 100 I 015C0/dsoftbus_standard: [COMM]frame get client proxy ok
01-01 00:20:24.160 16 100 I 015C0/dsoftbus_standard: [COMM]ServerProvideInterfaceInit ok
01-01 00:20:24.160 16 100 I 015C0/dsoftbus_standard: [COMM]server register service client push.
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]RECEIVE FUNCID:0
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]register service ipc server pop.
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]CheckSoftBusSysPermission uid:0 success
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]new client register:czypkgName
01-01 00:20:24.163 16 100 I 015C0/dsoftbus_standard: [COMM]retvalue:0
01-01 00:20:24.163 16 100 I 015C0/dsoftbus_standard: [COMM]success
    
//InitSoftBus运行成功返回值
01-01 00:20:24.163 16 100 I 015C0/dsoftbus_standard: [COMM]softbus sdk frame init success.
01-01 00:20:24.163 16 100 I 015C0/dsoftbus_standard: [DISC]start discovery ipc client push.
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]RECEIVE FUNCID:137
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]start discovery ipc server pop.
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [COMM]CheckSoftBusSysPermission uid:0 success
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [DISC]register input bitmap = [32].
01-01 00:20:24.163 14 82 I 015C0/dsoftbus_standard: [DISC]register all cap bitmap = [96].
write file switch /storage/data/log/hilog2.txt

//core\discovery\ipc\small\disc_client_proxy.c ClientIpcDiscoverySuccess开始
01-01 00:20:24.164 14 82 I 015C0/dsoftbus_standard: [DISC]on discovery success callback ipc server push.
 
//core\discovery\coap\src\disc_coap.cCoapStartAdvertise CoapStartAdvertise 运行成功返回值
01-01 00:20:24.164 14 82 I 015C0/dsoftbus_standard: [DISC]coap start active discovery.
    
//core\discovery\manager\src\softbus_disc_server.c DiscStartDiscovery 运行成功返回值
01-01 00:20:24.164 14 82 I 015C0/dsoftbus_standard: [DISC]ServerStartDiscovery success!
    

01-01 00:20:24.164 16 101 I 015C0/dsoftbus_standard: [COMM]receive ipc transact code(260)
01-01 00:20:24.164 16 101 I 015C0/dsoftbus_standard: [DISC]Sdk OnDiscoverySuccess, subscribeId = 233
```



# coap发现消息分析

> 之前分析start discovery函数会调用到 core\discovery\coap\src\disc_coap.c : CoapStartAdvertise函数，而该函数在设置完参数后，会调用DiscCoapStartDiscovery函数，本次就从该函数开始分析。

该函数首先负责根据模式的不同，调用相应的函数，这里我们优先分析**ACTIVE_DISCOVERY**

```c
//core\discovery\coap\src\disc_nstackx_adapter.c
int32_t DiscCoapStartDiscovery(DiscCoapMode mode)
{
    if (mode < ACTIVE_PUBLISH || mode > ACTIVE_DISCOVERY) {
        SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    switch (mode) {
        case ACTIVE_PUBLISH:
            if (NSTACKX_StartDeviceFindAn(PUBLISH_MODE_PROACTIVE) != SOFTBUS_OK) {
                return SOFTBUS_DISCOVER_COAP_START_PUBLISH_FAIL;
            }
            break;
        case ACTIVE_DISCOVERY:
            if (NSTACKX_StartDeviceFind() != SOFTBUS_OK) {
                return SOFTBUS_DISCOVER_COAP_START_DISCOVER_FAIL;
            }
            break;
        default:
            SoftBusLog(SOFTBUS_LOG_DISC, SOFTBUS_LOG_ERROR, "unsupport coap mode.");
            return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}
```

### NSTACKX_StartDeviceFind

注意到该函数对应了两个函数，应该是根据系统版本进行的区分。

<img src="https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202110240928812.png" alt="image-20211024092812722" style="zoom:67%;" />

通过分析编译文件`core\discovery\coap\BUILD.gn`来进行判断

可以看到仅有`liteos_m`是调用的`mini`打头的代码。其余系统均为`$dsoftbus_root_path/components/nstackx/nstackx_ctrl:nstackx_ctrl"`下的代码

<img src="https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202110240934799.png" alt="image-20211024093406653" style="zoom:67%;" />

```c
//components\nstackx\nstackx_ctrl\core\nstackx_common.c
int32_t NSTACKX_StartDeviceFind(void)
{
    if (g_nstackInitState != NSTACKX_INIT_STATE_DONE) {
        LOGE(TAG, "NSTACKX_Ctrl is not initiated yet");
        return NSTACKX_EFAILED;
    }
    if (PostEvent(&g_eventNodeChain, g_epollfd, DeviceDiscoverInner, NULL) != NSTACKX_EOK) {
        LOGE(TAG, "Failed to start device discover!");
        return NSTACKX_EFAILED;
    }
    return NSTACKX_EOK;
}
```

该函数干的事情也很简单，就是检查`g_nstackInitState`变量，然后发送一个`Event`,但显然总重要的是调用函数`DeviceDiscoverInner`

### DeviceDiscoverInner

```c
//components\nstackx\nstackx_ctrl\core\nstackx_common.c
static void DeviceDiscoverInner(void *argument)
{
    (void)argument;
    CoapServiceDiscoverInner(INNER_DISCOVERY);

    /* If both Wifi AP and BLE are disabled, we should also notify user, with empty list. */
    if (!IsWifiApConnected()) {
        NotifyDeviceFound(NULL, 0);
    }
}

```

该函数也调用了两个函数，下面的逻辑比较简单，且还有注释，先分析这个：

####  IsWifiApConnected

```c
//components\nstackx\nstackx_ctrl\core\nstackx_device.c
uint8_t IsWifiApConnected(void)
{
    struct in_addr ip;
    GetLocalIp(&ip);
    if (ip.s_addr != 0) {
        return NSTACKX_TRUE;//NSTACKX_TRUE==1
    }
    return NSTACKX_FALSE;//NSTACKX_FALSE==0
}
```

就是尝试去获取本地ip，如果得不到ip那么就返回0。

#### NotifyDeviceFound

```c
void NotifyDeviceFound(const NSTACKX_DeviceInfo *deviceList, uint32_t deviceCount)
{
    if (g_parameter.onDeviceFound != NULL) {
        LOGI(TAG, "notify callback: device found");
        g_parameter.onDeviceFound(deviceList, deviceCount);
        LOGI(TAG, "finish to notify device found");
    } else {
        LOGI(TAG, "notify callback: device found callback is null");
    }
}
```

结合传进来的参数，如果本地网络没有初始化，执行了`LOGI(TAG, "notify callback: device found callback is null");`。

重点分析`CoapServiceDiscoverInner`

### CoapServiceDiscoverInner

```c
//components\nstackx\nstackx_ctrl\core\coap_discover\coap_discover.c 
//这里的传入参数为INNER_DISCOVERY，值为1
void CoapServiceDiscoverInner(uint8_t userRequest)
{
    uint32_t discoverInterval;
    //判断网络是否连接
    if (!IsWifiApConnected() || g_context == NULL)
    {
        return;
    }

    //userRequest==INNER_DISCOVERY时，设置下面两个参数。
    if (userRequest)
    {
        g_userRequest = NSTACKX_TRUE;
        g_forceUpdate = NSTACKX_TRUE;
    }

    //计数值设置为零，清空device list backup
    if (g_coapDiscoverTargetCount > 0 && g_discoverCount >= g_coapDiscoverTargetCount)
    {
        g_discoverCount = 0;
        SetModeInfo(DISCOVER_MODE);
        ClearDevices(GetDeviceDBBackup());
        LOGW(TAG, "clear device list backup");
        TimerSetTimeout(g_discoverTimer, 0, NSTACKX_FALSE);
    }

    //discover若正在进行则返回
    if (g_discoverCount)
    {
        /* Service discover is ongoing, return. */
        return;
    }
    else
    {
        /* First discover */
        //备份当前g_deviceList，然后清空当前g_deviceList
        if (BackupDeviceDB() != NSTACKX_EOK)
        {
            LOGE(TAG, "backup device list fail");
            return;
        }
        ClearDevices(GetDeviceDB());
        LOGW(TAG, "clear device list");
        g_coapDiscoverTargetCount = g_coapMaxDiscoverCount;
    }
    //设置模式为DISCOVER_MODE
    SetModeInfo(DISCOVER_MODE);

    if (CoapPostServiceDiscover() != NSTACKX_EOK)
    {
        LOGE(TAG, "failed to post service discover request");
        return;
    }

    discoverInterval = GetDiscoverInterval(g_discoverCount);
    if (TimerSetTimeout(g_discoverTimer, discoverInterval, NSTACKX_FALSE) != NSTACKX_EOK)
    {
        LOGE(TAG, "failed to set timer for service discover");
        return;
    }
    ++g_discoverCount;
    LOGI(TAG, "the first time for device discover.");

    return;
}
```

### CoapPostServiceDiscover

```c
//components\nstackx\nstackx_ctrl\core\coap_discover\coap_discover.c
//获得InterfaceName，BroadcastIp，discoverUri,准备发送
static int32_t CoapPostServiceDiscover(void)
{
    char ifName[NSTACKX_MAX_INTERFACE_NAME_LEN] = {0};
    char ipString[NSTACKX_MAX_IP_STRING_LEN] = {0};
    char discoverUri[COAP_URI_BUFFER_LENGTH] = {0};
    char *data = NULL;

    if (GetLocalInterfaceName(ifName, sizeof(ifName)) != NSTACKX_EOK)
    {
        return NSTACKX_EFAILED;
    }

    if (GetIfBroadcastIp(ifName, ipString, sizeof(ipString)) != NSTACKX_EOK)
    {
        return NSTACKX_EFAILED;
    }
    //这个东西应该是发送广播包中用的，3861中也有这个东西
    if (sprintf_s(discoverUri, sizeof(discoverUri), "coap://%s/%s", ipString, COAP_DEVICE_DISCOVER_URI) < 0)
    {
        return NSTACKX_EFAILED;
    }
    //生成json信息，其中包含了设备的各种信息
    data = PrepareServiceDiscover(NSTACKX_TRUE);
    if (data == NULL)
    {
        LOGE(TAG, "failed to prepare coap data");
        return NSTACKX_EFAILED;
    }

    return CoapSendRequest(COAP_MESSAGE_NON, discoverUri, data, strlen(data) + 1, SERVER_TYPE_WLANORETH);
}
```

在该函数的`GetIfBroadcastIp`处出现错误，对该函数进行分析。注意该函数也有多个，此处以liteos目录下为例分析。

#### GetIfBroadcastIp

```c
int32_t GetIfBroadcastIp(const char *ifName, char *ipString, size_t ipStringLen)
{
    struct ifreq buf[INTERFACE_MAX];
    struct ifconf ifc;
    uint8_t foundIp = NSTACKX_FALSE;

    if (ifName == NULL) {
        return NSTACKX_EFAILED;
    }

    //获取接口列表
    int32_t fd = GetInterfaceList(&ifc, buf, sizeof(buf));
    if (fd < 0) {
        return NSTACKX_EFAILED;
    }

    int32_t ifreqLen = (int32_t)sizeof(struct ifreq);
    int32_t interfaceNum = (int32_t)(ifc.ifc_len / ifreqLen);

    //进行比较，通过ifName找到对应的ip地址，然后置foundIp为真，没找到则为假
    for (int32_t i = 0; i < interfaceNum && i < INTERFACE_MAX; i++)
    {

        if (strlen(buf[i].ifr_name) < strlen(ifName)) {
            continue;
        }
        if (memcmp(buf[i].ifr_name, ifName, strlen(ifName)) != 0) {
            continue;
        }
        //问题
        if (GetInterfaceInfo(fd, SIOCGIFBRDADDR, &buf[i]) != NSTACKX_EOK) {
            continue;
        }
        if (buf[i].ifr_addr.sa_family != AF_INET) {
            continue;
        }

        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&(buf[i].ifr_addr))->sin_addr), ipString,
            (socklen_t)ipStringLen) == NULL) {
            continue;
        }
        //出现问题的地方
        foundIp = NSTACKX_TRUE;
        break;
    }
    CloseSocketInner(fd);

    if (!foundIp) {
        return NSTACKX_EFAILED;
    }

    return NSTACKX_EOK;
}
```





#### PrepareServiceDiscover

主要负责制作coap报文

```c
//components\nstackx\nstackx_ctrl\core\coap_discover\json_payload.c
char *PrepareServiceDiscover(uint8_t isBroadcast)
{
    char coapUriBuffer[NSTACKX_MAX_URI_BUFFER_LENGTH] = {0};
    char host[NSTACKX_MAX_IP_STRING_LEN] = {0};
    char *formatString = NULL;
    const DeviceInfo *deviceInfo = GetLocalDeviceInfoPtr();
    cJSON *data = NULL;
    cJSON *localCoapString = NULL;

    data = cJSON_CreateObject();
    if (data == NULL) {
        goto L_END_JSON;
    }

    /* Prepare local device info */
    if ((AddDeviceJsonData(data, deviceInfo) != NSTACKX_EOK) ||
        (AddWifiApJsonData(data) != NSTACKX_EOK) ||
        (AddCapabilityBitmap(data, deviceInfo) != NSTACKX_EOK)) {
        goto L_END_JSON;
    }

    if (isBroadcast) {
        if (GetLocalIpString(host, sizeof(host)) != NSTACKX_EOK) {
            goto L_END_JSON;
        }
        if (sprintf_s(coapUriBuffer, sizeof(coapUriBuffer), "coap://%s/" COAP_DEVICE_DISCOVER_URI, host) < 0) {
            goto L_END_JSON;
        }
        localCoapString = cJSON_CreateString(coapUriBuffer);
        if (localCoapString == NULL || !cJSON_AddItemToObject(data, JSON_COAP_URI, localCoapString)) {
            cJSON_Delete(localCoapString);
            goto L_END_JSON;
        }
    }

    formatString = cJSON_PrintUnformatted(data);
    if (formatString == NULL) {
        LOGE(TAG, "cJSON_PrintUnformatted failed");
    }

L_END_JSON:
    cJSON_Delete(data);
    return formatString;
}
```



## 分析日记

- fb06b309f4cf3889705dff28362253a1f7c62525 在StarDiscovery 分析路径上打印信息，并且打印制作包的信息。

  运行日记输出如下：

  ```c
  OHOS # start discoveryTask
  [czy_test]ret is:0
  [czy_test]waiting!!!
  [CZY_TEST] enter CoapStartAdvertise
  4 nStackXDFinder: NSTACKX_SetFilterCapability:[518] :Set Filter Capability
  [CZY_TEST] enter  DiscCoapStartDiscovery
  [CZY_TEST_DiscCoapStartDiscovery] mode is ACTIVE_DISCOVERY
  [CZY_TEST_NSTACKX_StartDeviceFind] enter
  [czy_test]TestDiscoverySuccess
  3 nStackXCoAP: CoapServiceDiscoverStop:[643] :clear device list backup
  4 nStackXCoAP: CoapServiceDiscoverStopInner:[795] :device discover stopped
  [CZY_TEST_DeviceDiscoverInner] enter
  [CZY_TEST_CoapServiceDiscoverInner] enter
  2 nStackXDFinder: BackupDeviceDB:[1180] :clear backupDB error
  3 nStackXCoAP: CoapServiceDiscoverInner:[737] :clear device list
  // 已经进入CoapPostServiceDiscover函数，该函数负责制作包，最终调用发送
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY-TEST]按int型输出该值:35097
  2 nStackXDev: GetInterfaceInfo:[80] :ioctl fail, errno = 38
  2 nStackXCoAP: CoapServiceDiscoverInner:[742] :failed to post service discover request
  //该函数后续还有很多输出信息，结合目前报错判断是GetInterfaceInfo错误
  ```

  即`GetLocalInterfaceName`函数附近存在错误。

- 5c11d80d875e88dbbc2d6f4ec9f96be5c3c26c50 在`GetLocalInterfaceName`附近继续加入代码，以分析问题

  ```c
  OHOS # 
  [czy_test]ret is:0
  [czy_test]waiting!!!
  [CZY_TEST] enter CoapStartAdvertise
  [CZY_TEST] enter  DiscCoapStartDiscovery
  [CZY_TEST_DiscCoapStartDiscovery] mode is ACTIVE_DISCOVERY
  [CZY_TEST_NSTACKX_StartDeviceFind] enter
  [czy_test]TestDiscoverySuccess
  [CZY_TEST_DeviceDiscoverInner] enter
  [CZY_TEST_CoapServiceDiscoverInner] enter
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY-TEST]按int型输出该值:35097
  //可以清晰地定位到问题出在GetIfBroadcastIp上
  [CZY_TEST_CoapPostServiceDiscover]GetIfBroadcastIp error
  ```

- 在`GetIfBroadcastIp`中加入分析函数，问题最终定位到

  ```c
  01-01 00:10:05.952 17 16 I 015C0/dsoftbus_standard: [LNN]NodeStateCbCount is 10
  [czy_test]ret is:0
  [czy_test]waiting!!!
  [CZY_TEST] enter CoapStartAdvertise
  4 nStackXDFinder: NSTACKX_SetFilterCapability:[518] :Set Filter Capability
  [CZY_TEST] enter  DiscCoapStartDiscovery
  [CZY_TEST_DiscCoapStartDiscovery] mode is ACTIVE_DISCOVERY
  [CZY_TEST_NSTACKX_StartDeviceFind] enter
  [czy_test]TestDiscoverySuccess
  3 nStackXCoAP: CoapServiceDiscoverStop:[648] :clear device list backup
  4 nStackXCoAP: CoapServiceDiscoverStopInner:[800] :device discover stopped
  [CZY_TEST_DeviceDiscoverInner] enter
  [CZY_TEST_CoapServiceDiscoverInner] enter
  2 nStackXDFinder: BackupDeviceDB:[1180] :clear backupDB error
  3 nStackXCoAP: CoapServiceDiscoverInner:[742] :clear device list
  [CZY_TEST_CoapPostServiceDiscover] enter
  //ifName为eth？？？这可能是问题所在
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_GetIfBroadcastIp] enter
   [CZY_TEST_GetInterfaceList] enter
   [CZY-TEST]按int型输出该值:35097
  //GetInterfaceInfo在循环中被调用了，而且出了些问题，但函数继续运行
  2 nStackXDev: GetInterfaceInfo:[80] :ioctl fail, errno = 38
  //没有找到ip
  [CZY_TEST_GetIfBroadcastIp] !foundIp
  [CZY_TEST_CoapPostServiceDiscover]GetIfBroadcastIp error
  2 nStackXCoAP: CoapServiceDiscoverInner:[747] :failed to post service discover request
  ```

- c17b544e9e221306add7b74dc2ffe09d4e7799b9：为了解决该问题，`GetIfBroadcastIp`中输出找到的设备列表中的所有信息

  ```c
  [czy_test]ret is:0
  [czy_test]waiting!!!
  [CZY_TEST] enter CoapStartAdvertise
  4 nStackXDFinder: NSTACKX_SetFilterCapability:[518] :Set Filter Capability
  [CZY_TEST] enter  DiscCoapStartDiscovery
  [CZY_TEST_DiscCoapStartDiscovery] mode is ACTIVE_DISCOVERY
  [CZY_TEST_NSTACKX_StartDeviceFind] enter
  [czy_test]TestDiscoverySuccess
  3 nStackXCoAP: CoapServiceDiscoverStop:[648] :clear device list backup
  4 nStackXCoAP: CoapServiceDiscoverStopInner:[800] :device discover stopped
  [CZY_TEST_DeviceDiscoverInner] enter
  [CZY_TEST_CoapServiceDiscoverInner] enter
  2 nStackXDFinder: BackupDeviceDB:[1180] :clear backupDB error
  3 nStackXCoAP: CoapServiceDiscoverInner:[742] :clear device list
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_GetIfBroadcastIp] enter
   [CZY_TEST_GetInterfaceList] enter
   [CZY_TEST_GetIfBroadcastIp]interfaceNum==3
  [CZY_TEST_GetIfBroadcastIp]print all interfaceInfo
  [CZY_TEST_GetIfBroadcastIp]ifr_name==wlan0
  [CZY_TEST_GetIfBroadcastIp]wlan0 != ifName
  01-01 00:07:25.980 16 100 I 015C0/dsoftbus_standard: [TRAN]trans udp channel manager init success.
  [CZY_TEST_GetIfBroadcastIp]ifr_name==lo
  [CZY_TEST_GetIfBroadcastIp]ifr_name==lo len<ifName
  [CZY_TEST_GetIfBroadcastIp]ifr_name==eth0
  [CZY_TEST_GetIfBroadcastIp] enter GetInterfaceInfo
  [CZY-TEST]按int型输出该值:35097
  2 nStackXDev: GetInterfaceInfo:[80] :ioctl fail, errno = 38
  //定位到
  [CZY_TEST_GetIfBroadcastIp] GetInterfaceInfo error
  [CZY_TEST_GetIfBroadcastIp] !foundIp
   [CZY_TEST_CoapPostServiceDiscover]GetIfBroadcastIp error
  2 nStackXCoAP: CoapServiceDiscoverInner:[747] :failed to post service discover request
  ```

- 94672c694810bb01cfbb3cac7f89cf3c4739db90 上述问题不好解决，我们直接在`CoapPostServiceDiscover`上动手，手动输入广播地址为我们的地址

  ```c
  OHOS # start discoveryTask
  [czy_test]ret is:0
  [czy_test]waiting!!!
  [CZY_TEST] enter CoapStartAdvertise
  [CZY_TEST] enter  DiscCoapStartDiscovery
  [CZY_TEST_DiscCoapStartDiscovery] mode is ACTIVE_DISCOVERY
  [CZY_TEST_NSTACKX_StartDeviceFind] enter
  [czy_test]TestDiscoverySuccess
  [CZY_TEST_DeviceDiscoverInner] enter
  [CZY_TEST_CoapServiceDiscoverInner] enter
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] print some info might be sent--------------
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] ipString:192.168.137.255
  [CZY_TEST_CoapPostServiceDiscover] discoverUri:coap://192.168.137.255/device_discover
  [CZY_TEST_CoapPostServiceDiscover] data:{"deviceId":"{\"UDID\":\"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00\"}","devicename":"UNKNOWN","type":0,"hicomversion":"hm.1.0.0","mode":1,"deviceHash":"","serviceData":"port:59565,","wlanIp":"192.168.1.10","capabilityBitmap":[64],"coapUri":"coap://192.168.1.10/device_discover"}
  [CZY_TEST_CoapPostServiceDiscover] print some info end--------------
  [CZY_TEST_CoapSendRequest] enter
  Jan 01 00:01:53 ERR  interface is not up
  [CZY_TEST_CoapSendRequest] CoapGetSessionOnTargetServer success
  [CZY_TEST_CoapSendRequest] CoapPackToPdu success
  Jan 01 00:01:53 CRIT coap_network_send: Host is unreachable
  [CZY_TEST_CoapSendRequest] coap_send suwrite file switch /storage/data/log/hilog2.txt
  ccess
  01-01 00:01:53.175 15 48 I 015C0/dsoftbus_standard: [LNN]BusCenterClientInit init OK!
  2 nStackXCoAP: CoapSendRequest:[240] :coap send failed
  01-01 00:01:53.175 15 48 I 015C0/dsoftbus_standard: [DISC]disc start get server proxy
  2 nStackXCoAP: CoapServiceDiscoverInner:[750] :failed to post service discover request
  01-01 00:01:53.175 15 48 I 015C0/dsoftbus_standard: [DISC]disc get server proxy ok
  01-01 00:01:53.175 15 48 I 015C0/dsoftbus_standard: [DISC]Init success
  
  ```

  分析信息我们注意到，当前wlanIp为一个奇怪的地址，而非我们当前的地址。因此我们**杀死服务然后重启**，成功！

  3516端报文

  ```c
  OHOS # ./softbusDiscovery
  OHOS # start discoveryTask
  01-01 00:14:16.332 20 100 I 015C0/dsoftbus_standard: [LNN]NodeStateCbCount is 10
  [CZY_TEST] enter CoapStartAdvertise
  4 nStackXDFinder: NSTACKX_SetFilterCapability:[518] :Set Filter Capability
  [CZY_TEST] enter  DiscCoapStartDiscovery
  [CZY_TEST_DiscCoapStartDiscovery] mode is ACTIVE_DISCOVERY
  [CZY_TEST_NSTACKX_StartDeviceFind] enter
  3 nStackXCoAP: CoapServiceDiscoverStop:[651] :clear device list backup
  4 nStackXCoAP: CoapServiceDiscoverStopInner:[803] :device discover stopped
  [CZY_TEST_DeviceDiscoverInner] enter
  [CZY_TEST_CoapServiceDiscoverInner] enter
  2 nStackXDFinder: BackupDeviceDB:[1180] :clear backupDB error
  3 nStackXCoAP: CoapServiceDiscoverInner:[745] :clear device list
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] print some info might be sent--------------
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] ipString:192.168.137.255
  [CZY_TEST_CoapPostServiceDiscover] discoverUri:coap://192.168.137.255/device_discover
  [CZY_TEST_CoapPostServiceDiscover] data:{"deviceId":"{\"UDID\":\"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00\"}","devicename":"UNKNOWN","type":0,"hicomversion":"hm.1.0.0","mode":1,"deviceHash":"","serviceData":"port:59567,","wlanIp":"192.168.137.120","capabilityBitmap":[64],"coapUri":"coap://192.168.137.120/device_discover"}
  [CZY_TEST_CoapPostServiceDiscover] print some info end--------------
  [czy_test]ret is:0
  [czy_test]waiting!!!
  [czy_test]TestDiscoverySuccess
  [CZY_TEST_CoapSendRequest] enter
  Jan 01 00:14:16 ERR  interface is not up
  [CZY_TEST_CoapSendRequest] CoapGetSessionOnTargetServer success
  [CZY_TEST_CoapSendRequest] CoapPackToPdu success
      
  //报文发送成功
  [CZY_TEST_CoapSendRequest] coap_send success
      
  4 nStackXCoAP: CoapServiceDiscoverInner:[760] :the first time for device discover.
  01-01 00:14:16.332 20 100 I 015C0/dsoftbus_standard: [LNN]bus center start get server proxy
  01-01 00:14:16.332 20 100 I 01800/Samgr: Initialize Client Registry!
  01-01 00:14:16.333 3 35 D 01800/Samgr: Judge Auth<softbus_service, (null)> ret:0
  01-01 00:14:16.333 3 35 D 01800/Samgr: Find Feature<softbus_service, (null)> id<91, 0> ret:0
  01-01 00:14:16.333 20 100 I 01800/Samgr: Create remote sa proxy[0x24f84260]<softbus_service, (null)>!
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [LNN]bus 4 nStackXCoAP: GetServiceDiscoverInfo:[280] :data is not end with 0
  3 nStackXCoAP: ParseDeviceJsonData:[172] :Can't find hicom version
  4 nStackXDFinder: NotifyDeviceListChanged:[763] :notify callback: device list changed
  4 nStackXDFinder: NotifyDeviceListChanged:[765] :finish to notify device list changed
  4 nStackXDFinder: NotifyDeviceFound:[778] :notify callback: device found callback is null
  //设备发现！！！
  [czy_test]TestDeviceFound
      
  center get server proxy ok
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [LNN]BusCenterClientInit init OK!
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [DISC]disc start get server proxy
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [DISC]disc get server proxy ok
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [DISC]Init success
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [TRAN]trans start get server proxy
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [TRAN]trans get server proxy ok
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [TRAN[CZY_TEST_CoapPostServiceDiscover] enter
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] print some info might be sent--------------
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] ipString:192.168.137.255
  [CZY_TEST_CoapPostServiceDiscover] discoverUri:coap://192.168.137.255/device_discover
  [CZY_TEST_CoapPostServiceDiscover] data:{"deviceId":"{\"UDID\":\"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00\"}","devicename":"UNKNOWN","type":0,"hicomversion":"hm.1.0.0","mode":1,"deviceHash":"","serviceData":"port:59567,","wlanIp":"192.168.137.120","capabilityBitmap":[64],"coapUri":"coap://192.168.137.120/device_discover"}
  [CZY_TEST_CoapPostServiceDiscover] print some info end--------------
  [CZY_TEST_CoapSendRequest] enter
  Jan 01 00:14:16 ERR  interface is not up
  [CZY_TEST_CoapSendRequest] CoapGetSessionOnTargetServer success
  [CZY_TEST_CoapSendRequest] CoapPackToPdu success
  [CZY_TEST_CoapSendRequest] coap_send success
  4 nStackXCoAP: CoapServiceDiscoverTimerHandle:[672] :the 2 times for device discover.
  4 nStackXCoAP: GetServiceDiscoverInfo:[280] :data is not end with 0
  3 nStackXCoAP: ParseDeviceJsonData:[172] :Can't find hicom version
  ]init tcp direct channel success.
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [TRAN]trans udp channel manager init success.
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [TRAN]init succ
  write file switch /storage/data/log/hilog2.txt
  01-01 00:14:16.333 20 100 I 01800/Samgr: Bootstrap core services(count:0).
  01-01 00:14:16.333 20 100 I 01800/Samgr: Initialized all core system services!
  01-01 00:14:16.333 20 100 I 01800/Samgr: Goto next boot step return code:-9
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [COMM]start get client proxy
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [COMM]frame get client proxy ok
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [COMM]ServerProvideInterfaceInit ok
  01-01 00:14:16.333 20 100 I 015C0/dsoftbus_standard: [COMM]server register service client push.
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]RECEIVE FUNCID:0
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]register service ipc server pop.
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]CheckSoftBusSysPermission uid:2 success
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]new client register:czypkgName
  01-01 00:14:16.334 20 100 I 015C0/dsoftbus_standard: [COMM]retvalue:0
  01-01 00:14:16.334 20 100 I 015C0/dsoftbus_standard: [COMM]success
  01-01 00:14:16.334 20 100 I 015C0/dsoftbus_standard: [COMM]softbus sdk frame init success.
  01-01 00:14:16.334 20 100 I 015C0/dsoftbus_standard: [DISC]start discovery ipc client push.
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]RECEIVE FUNCID:137
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]start discovery ipc server pop.
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [COMM]CheckSoftBusSysPermission uid:2 success
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [DISC]register input bitmap = [32].
  01-01 00:14:16.334 17 95 I 015C0/dsoftbus_standard: [DISC]register all cap bitmap = [96].
  01-01 00:14:16.335 17 95 I 015C0/dsoftbus_standard: [DISC]coap start active discovery.
  01-01 00:14:16.335 17 95 I 015C0/dsoftbus_standard: [DISC]ServerStartDiscovery success!
  [CZY_TEST_CoapPostServiceDiscover] enters_standard: [DISC]on discovery success callback ipc server push.
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] print some info might be sent--------------
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] ipString:192.168.137.255
  [CZY_TEST_CoapPostServiceDiscover] discoverUri:coap://192.168.137.255/device_discover
  [CZY_TEST_CoapPostServiceDiscover] data:{"deviceId":"{\"UDID\":\"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00\"}","devicename":"UNKNOWN","type":0,"hicomversion":"hm.1.0.0","mode":1,"deviceHash":"","serviceData":"port:59567,","wlanIp":"192.168.137.120","capabilityBitmap":[64],"coapUri":"coap://192.168.137.120/device_discover"}
  [CZY_TEST_CoapPostServiceDiscover] print some info end--------------
  [CZY_TEST_CoapSendRequest] enter
  Jan 01 00:14:16 ERR  interface is not up
  [CZY_TEST_CoapSendRequest] CoapGetSessionOnTargetServer success
  [CZY_TEST_CoapSendRequest] CoapPackToPdu success
  [CZY_TEST_CoapSendRequest] coap_send success
  4 nStackXCoAP: CoapServiceDiscoverTimerHandle:[672] :the 3 times for device discover.
  4 nStackXCoAP: GetServiceDiscoverInfo:[280] :data is not end with 0
  3 nStackXCoAP: ParseDeviceJsonData:[172] :Can't find hicom version
  
  01-01 00:14:16.358 20 98 I 015C0/dsoftbus_standard: [COMM]receive ipc transact code(260)
  write file switch /storage/data/log/hilog1.txt
  01-01 00:14:16.358 20 98 I 015C0/dsoftbus_standard: [DISC]Sdk OnDiscoverySuccess, subscribeId = 233
  01-01 00:14:16.448 17 87 I 015C0/dsoftbus_standard: [DISC]Server OnDeviceFound capabilityBitmap = 32
  01-01 00:14:16.448 17 87 I 015C0/dsoftbus_standard: [DISC]find callback:id = 233
  01-01 00:14:16.448 17 87 I 015C0/dsoftbus_standard: [DISC]ondevice found ipc server push.
  01-01 00:14:16.449 20 98 I 015C0/dsoftbus_standard: [COMM]receive ipc transact code(262)
  01-01 00:14:16.449 20 98 I 015C0/dsoftbus_standard: [DISC]Sdk OnDeviceFound, capabilityBitmap = 32
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] print some info might be sent--------------
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] ipString:192.168.137.255
  [CZY_TEST_CoapPostServiceDiscover] discoverUri:coap://192.168.137.255/device_discover
  [CZY_TEST_CoapPostServiceDiscover] data:{"deviceId":"{\"UDID\":\"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00\"}","devicename":"UNKNOWN","type":0,"hicomversion":"hm.1.0.0","mode":1,"deviceHash":"","serviceData":"port:59567,","wlanIp":"192.168.137.120","capabilityBitmap":[64],"coapUri":"coap://192.168.137.120/device_discover"}
  [CZY_TEST_CoapPostServiceDiscover] print some info end--------------
  [CZY_TEST_CoapSendRequest] enter
  Jan 01 00:14:17 ERR  interface is not up
  [CZY_TEST_CoapSendRequest] CoapGetSessionOnTargetServer success
  [CZY_TEST_CoapSendRequest] CoapPackToPdu success
  [CZY_TEST_CoapSendRequest] coap_send success
      
  .
  [CZY_TEST_CoapPostServiceDiscover] enter
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] print some info might be sent--------------
  [CZY_TEST_CoapPostServiceDiscover] ifName:eth
  [CZY_TEST_CoapPostServiceDiscover] ipString:192.168.137.255
  [CZY_TEST_CoapPostServiceDiscover] discoverUri:coap://192.168.137.255/device_discover
  [CZY_TEST_CoapPostServiceDiscover] data:{"deviceId":"{\"UDID\":\"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00\"}","devicename":"UNKNOWN","type":0,"hicomversion":"hm.1.0.0","mode":1,"deviceHash":"","serviceData":"port:59567,","wlanIp":"192.168.137.120","capabilityBitmap":[64],"coapUri":"coap://192.168.137.120/device_discover"}
  [CZY_TEST_CoapPostServiceDiscover] print some info end--------------
  [CZY_TEST_CoapSendRequest] enter
  Jan 01 00:14:21 ERR  interface is not up
  [CZY_TEST_CoapSendRequest] CoapGetSessionOnTargetServer success
  [CZY_TEST_CoapSendRequest] CoapPackToPdu success
  [CZY_TEST_CoapSendRequest] coap_send success
  4 nStackXCoAP: CoapServiceDiscoverTimerHandle:[672] :the 12 times for device discover.
  4 nStackXCoAP: GetServiceDiscoverInfo:[280] :data is not end with 0
  3 nStackXCoAP: ParseDeviceJsonData:[172] :Can't find hicom version
  3 nStackXCoAP: CoapServiceDiscoverStop:[651] :clear device list backup
  
  ```

  3861端：

  ```c
  enter SoftBus Task
  [DISCOVERY] InitLocalDeviceInfo ok
  [DISCOVERY] CoapInitWifiEvent
  [DISCOVERY] CoapWriteMsgQueue
  [DISCOVERY] InitService ok
  [DISCOVERY] PublishCallback publishId=233, result=0
  publish succeeded, publishId = 233
  PublishService init success
  CoapGetIp = 192.168.137.183
  StartSessionServer successed!
  [TRANS] WaitProcess begin
  [TRANS] StartListener ok
  [TRANS] SelectSessionLoop begin
  [AUTH] StartBus ok
  [DISCOVERY] CoapReadHandle coin select begin
  hiview init success.[CZY_TEST]:IO requests are listened by CoapReadHandle ,ret is:1 
  [CZY_TEST]:printf addr info:port:13334,addr:192.168.137.120
  [CZT_TEST] HandleReadEvent nRead is:347
  [CZT_TEST] COAP_SoftBusDecode ret is:0,success
  [CZT_TEST]print deviceInfo:
  [CZT_TEST]deviceInfo-deviceName:UNKNOWN:
  [CZT_TEST]deviceInfo-deviceId:{"UDID":"ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00"}
  [CZT_TEST]deviceInfo-deviceType:0
  [CZT_TEST]deviceInfo-portNumber:0
  [CZT_TEST]deviceInfo-capabilityBitmapNum:1
  [CZT_TEST]deviceInfo-mode:
  [CZT_TEST]deviceInfo-serviceData:port:59567,:
  [CZT_TEST]print remoteUrl coap://192.168.137.120/device_discover:
  [CZT_TEST]print wifiIpAddr 192.168.137.120:
  [CZY_TEST]:IO requests are listened by CoapReadHandle ,ret is:1 
  [CZY_TEST]:printf addr info:port:13334,addr:192.168.137.120
  //以下内容均为16-31行的重复
  ```

  总结下3516目前发现的两个bug

  1. 根据接口获取广播地址
  2. 不能根据接口变化自动重新获取新的ip，必须要手动获取ip

