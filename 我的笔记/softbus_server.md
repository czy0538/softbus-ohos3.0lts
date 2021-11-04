# softbus_server分析

> 在系统启动时，会启动`softbus_server`程序，该程序是其他软总线功能的基本支持程序。且该程序中具有部分发现的内容。本次就从该程序开始分析。



## 从编译文件入手

> BUILD.gn

```gn
lite_component("dsoftbus") {
  features = [
    "core:softbus_server",
    "sdk:softbus_client",
    "tests:softbus_test",
  ]
}
```

可以看到`softbus_server`位于`core`目录下。

进入该目录下

> core\BUILD.gn

```gn
lite_component("softbus_server") {
  if (ohos_kernel_type == "liteos_m") {
    features = [ "frame:softbus_server_frame" ]
  } else {
    features = [ "frame:softbus_server" ]
  }
}
```

可以看到分成了两部分，`liteos_m`使用的是`softbus_server_frame`，其他系统则是`softbus_server`，均位于`frame`目录下。

> core\frame\BUILD.gn

![image-20211104102459628](https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202111041024775.png)

可以清晰地看到，`executable`指向了生成的可执行文件。标准系统主程序位于`"core/framesmall/init/src/softbus_server_main.c"`下。

> core\frame\small\init\src\softbus_server_main.c

```c
#include "softbus_server_frame.h"
#include <unistd.h>

int main(int argc, char** argv)
{
    InitSoftBusServer();
    while (1) 
    {
        pause();
    }
    return 0;
}
```

该函数调用了`InitSoftBusServer()`,然后`pause()`会令目前的进程暂停(进入睡眠状态), 直到被信号(signal)所中断，随后继续休眠，但接着会再次调用pause。。。效果上来看是保证了这个进程不退出？？？但为啥要加pause？？？

不管了，重点反正在`InitSoftBusServer();`

系统中存在两个该函数，路径分别为：

![image-20211104105006963](https://picgo-1305367394.cos.ap-beijing.myqcloud.com/picgo/202111041050029.png)

`core\frame\common\src\softbus_server_frame.c`和`core\frame\standard\server\src\softbus_server_frame.c`，但结合前面的编译文件来看，`include_dirs`中仅有`common\include`，与`softbus_server_frame.h`对应，故应先去那里面看看。

> core\frame\common\include\softbus_server_frame.h

```c
#ifndef SOFTBUS_SERVER_FRAME_H
#define SOFTBUS_SERVER_FRAME_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void InitSoftBusServer();
bool GetServerIsInit();
void ClientDeathCallback(const char *pkgName);

#ifdef __cplusplus
}
#endif

#endif
```

并没有区分small和stand系统的内容，是不是stand系统用的是另一套东西？

这里先留下这个疑问，总之对弈small系统，调用的应该是`core\frame\common\src\softbus_server_frame.c`下的文件了。就从他出发吧。

主要负责各种调用了各种Init函数，我们接下来将逐个进行分析。

此外还有个`static bool g_isInit `变量，为`true`时说明初始化成功。

```c
void InitSoftBusServer(void)
{
    SoftbusConfigInit();

    if (ServerStubInit() != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "server stub init failed.");
        return;
    }

    if (SoftBusTimerInit() == SOFTBUS_ERR) {
        return;
    }

    if (LooperInit() == -1) {
        return;
    }
    if (ConnServerInit() == SOFTBUS_ERR) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus conn server init failed.");
        goto ERR_EXIT;
    }

    if (TransServerInit() == SOFTBUS_ERR) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus trans server init failed.");
        goto ERR_EXIT;
    }

    if (AuthInit() == SOFTBUS_ERR) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus auth init failed.");
        goto ERR_EXIT;
    }

    if (DiscServerInit() == SOFTBUS_ERR) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus disc server init failed.");
        goto ERR_EXIT;
    }

    if (BusCenterServerInit() == SOFTBUS_ERR) {
        SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus buscenter server init failed.");
        goto ERR_EXIT;
    }

    g_isInit = true;
    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_INFO, "softbus framework init success.");
    return;

ERR_EXIT:
    ServerModuleDeinit();
    SoftBusLog(SOFTBUS_LOG_COMM, SOFTBUS_LOG_ERROR, "softbus framework init failed.");
    return;
}
```

  `SoftBusLog`主要用来打印错误信息，不再过多解读。

## SoftbusConfigInit

该函数最终目的就是初始化`g_configItems`，给他赋予各种各样的值，目的不详。

> core\common\softbus_property\src\softbus_feature_config.c

涉及变量：`g_tranConfig`、`g_configItems`、`g_config`

```c
void SoftbusConfigInit(void)
{
    ConfigSetProc sets;//结构体，成员是一个函数指针
    SoftbusConfigSetDefaultVal();//套壳，内部是SoftbusConfigSetTransDefaultVal()，最终效果是初始化g_tranConfig中的参数。
    sets.SetConfig = &SoftbusSetConfig;//函数指针指向了SoftbusSetConfig
    SoftbusConfigAdapterInit(&sets);//最终调用SoftbusSetConfig，该函数作用是对g_configItems中SOFTBUS_INT_MAX_BYTES_LENGTH项的值进行修改。
}

static void SoftbusConfigSetDefaultVal(void)
{
    SoftbusConfigSetTransDefaultVal();
}

static void SoftbusConfigSetTransDefaultVal(void)
{
    g_tranConfig.isSupportTcpProxy = DEFAULT_IS_SUPPORT_TCP_PROXY;
    g_tranConfig.selectInterval = DEFAULT_SElECT_INTERVAL;
    g_tranConfig.maxBytesLen = DEFAULT_MAX_BYTES_LEN;
    g_tranConfig.maxMessageLen = DEFAULT_MAX_MESSAGE_LEN;
}

typedef struct {
    int32_t isSupportTcpProxy;
    int32_t selectInterval;
    int32_t maxBytesLen;
    int32_t maxMessageLen;
} TransConfigItem;

static TransConfigItem g_tranConfig = {0};
```

> `ConfigSetProc`定义在adapter\default_config\spec_config\softbus_config_type.h

该结构体内唯一的成员是一个函数指针。

```c
typedef struct {
    int32_t (* SetConfig)(ConfigType type, const unsigned char *val, int32_t len);
} ConfigSetProc;
```

### SoftbusConfigAdapterInit

> adapter\default_config\spec_config\softbus_config_adapter.c

```c
void SoftbusConfigAdapterInit(const ConfigSetProc *sets)
{
    int32_t val;
    val = MAX_BYTES_LENGTH;
    sets->SetConfig(SOFTBUS_INT_MAX_BYTES_LENGTH, (unsigned char*)&val, sizeof(val));
}
```

该函数结合上面的内容等效如下效果

> core\common\softbus_property\src\softbus_feature_config.c

检查传入的`type`和`len`是否合规，然后将`val`赋给对应`type`的`g_configItems[type].val`

```c
type = SOFTBUS_INT_MAX_BYTES_LENGTH
val = (unsigned char*)&val
len = sizeof(val)
int SoftbusSetConfig(ConfigType type, const unsigned char *val, int32_t len)
{
    if (len > g_configItems[type].len) {
        return SOFTBUS_ERR;
    }
    if ((type >= SOFTBUS_CONFIG_TYPE_MAX) || (type != g_configItems[type].type)) {
        return SOFTBUS_ERR;
    }
    if (memcpy_s(g_configItems[type].val, g_configItems[type].len, val, len) != EOK) {
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

typedef struct {
    ConfigType type;
    unsigned char *val;
    int32_t len;
} ConfigVal;

typedef enum {
    //对应type的值应该是从0开始的，依次递增
    SOFTBUS_INT_MAX_BYTES_LENGTH, /* the default val is 4194304 */
    SOFTBUS_INT_MAX_MESSAGE_LENGTH, /* the default val is 4096 */
    SOFTBUS_INT_CONN_BR_MAX_DATA_LENGTH, /* the default val is 4096 */
    SOFTBUS_INT_CONN_RFCOM_SEND_MAX_LEN, /* the default val is 990 */
    SOFTBUS_INT_CONN_BR_RECEIVE_MAX_LEN, /* the default val is 10 */
    SOFTBUS_INT_CONN_TCP_MAX_LENGTH, /* the default val is 3072 */
    SOFTBUS_INT_CONN_TCP_MAX_CONN_NUM, /* the default val is 30 */
    SOFTBUS_INT_CONN_TCP_TIME_OUT, /* the default val is 100 */
    SOFTBUS_INT_MAX_NODE_STATE_CB_CNT, /* the default val is 10 */
    SOFTBUS_INT_MAX_LNN_CONNECTION_CNT, /* the default val is 10 */
    SOFTBUS_INT_LNN_SUPPORT_CAPBILITY, /* the default val is 22 */
    SOFTBUS_INT_AUTH_ABILITY_COLLECTION, /* the default val is 0 */
    SOFTBUS_INT_ADAPTER_LOG_LEVEL, /* the default val is 0 */
    SOFTBUS_STR_STORAGE_DIRECTORY, /* the max length is MAX_STORAGE_PATH_LEN */
    SOFTBUS_CONFIG_TYPE_MAX,
} ConfigType;
```

### g_configItems

存储各个`ConfigType`对应的type，有些地址指向`g_tranConfig.maxBytesLen`,该值在后面被初始化，有些指向`gconfig`，而该值在前面被初始化

```c
typedef struct {
    ConfigType type;
    unsigned char *val;
    int32_t len;
} ConfigVal;

ConfigVal g_configItems[SOFTBUS_CONFIG_TYPE_MAX] = {
    {
        SOFTBUS_INT_MAX_BYTES_LENGTH, 
        (unsigned char*)&(g_tranConfig.maxBytesLen),
        sizeof(g_tranConfig.maxBytesLen)
    },
    {
        SOFTBUS_INT_MAX_MESSAGE_LENGTH, 
        (unsigned char*)&(g_tranConfig.maxMessageLen),
        sizeof(g_tranConfig.maxMessageLen)
    },
    {
        SOFTBUS_INT_CONN_BR_MAX_DATA_LENGTH, 
        (unsigned char*)&(g_config.connBrMaxDataLen), 
        sizeof(g_config.connBrMaxDataLen)
    },
    {
        SOFTBUS_INT_CONN_RFCOM_SEND_MAX_LEN, 
        (unsigned char*)&(g_config.connRfcomSendMaxLen), 
        sizeof(g_config.connRfcomSendMaxLen)
    },
    {
        SOFTBUS_INT_CONN_BR_RECEIVE_MAX_LEN, 
        (unsigned char*)&(g_config.connBrRecvMaxLen), 
        sizeof(g_config.connBrRecvMaxLen)
    },
    {
        SOFTBUS_INT_CONN_TCP_MAX_LENGTH, 
        (unsigned char*)&(g_config.connTcpMaxLen), 
        sizeof(g_config.connTcpMaxLen)
    },
    {
        SOFTBUS_INT_CONN_TCP_MAX_CONN_NUM, 
        (unsigned char*)&(g_config.connTcpMaxConnNum), 
        sizeof(g_config.connTcpMaxConnNum)
    },
    {
        SOFTBUS_INT_CONN_TCP_TIME_OUT, 
        (unsigned char*)&(g_config.connTcpTimeOut), 
        sizeof(g_config.connTcpTimeOut)
    },
    {
        SOFTBUS_INT_MAX_NODE_STATE_CB_CNT, 
        (unsigned char*)&(g_config.maxNodeStateCbCnt), 
        sizeof(g_config.maxNodeStateCbCnt)
    },
    {
        SOFTBUS_INT_MAX_LNN_CONNECTION_CNT, 
        (unsigned char*)&(g_config.maxLnnConnCnt), 
        sizeof(g_config.maxLnnConnCnt)
    },
    {
        SOFTBUS_INT_LNN_SUPPORT_CAPBILITY,
        (unsigned char*)&(g_config.maxLnnSupportCap), 
        sizeof(g_config.maxLnnSupportCap)
    },
    {
        SOFTBUS_INT_AUTH_ABILITY_COLLECTION, 
        (unsigned char*)&(g_config.authAbilityConn), 
        sizeof(g_config.authAbilityConn)
    },
    {
        SOFTBUS_INT_ADAPTER_LOG_LEVEL, 
        (unsigned char*)&(g_config.adapterLogLevel), 
        sizeof(g_config.adapterLogLevel)
    },
    {
        SOFTBUS_STR_STORAGE_DIRECTORY, 
        (unsigned char*)(g_config.storageDir), 
        sizeof(g_config.storageDir)
    },
};

typedef struct {
    int32_t authAbilityConn;
    int32_t connBrMaxDataLen;
    int32_t connRfcomSendMaxLen;
    int32_t connBrRecvMaxLen;
    int32_t connTcpMaxLen;
    int32_t connTcpMaxConnNum;
    int32_t connTcpTimeOut;
    int32_t maxNodeStateCbCnt;
    int32_t maxLnnConnCnt;
    int32_t maxLnnSupportCap;
    int32_t adapterLogLevel;
    char storageDir[MAX_STORAGE_PATH_LEN];
} ConfigItem;

ConfigItem g_config = {
    AUTH_ABILITY_COLLECTION,
    CONN_BR_MAX_DATA_LENGTH,
    CONN_RFCOM_SEND_MAX_LEN,
    CONN_BR_RECEIVE_MAX_LEN,
    CONN_TCP_MAX_LENGTH,
    CONN_TCP_MAX_CONN_NUM,
    CONN_TCP_TIME_OUT,
    MAX_NODE_STATE_CB_CNT,
    MAX_LNN_CONNECTION_CNT,
    LNN_SUPPORT_CAPBILITY,
    ADAPTER_LOG_LEVEL,
    DEFAULT_STORAGE_PATH,
};
```



## ServerStubInit



## SoftBusTimerInit



## LooperInit



## ConnServerInit



## TransServerInit



## AuthInit



## DiscServerInit



## BusCenterServerInit

