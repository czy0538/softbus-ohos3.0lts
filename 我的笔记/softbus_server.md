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

