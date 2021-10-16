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

