
----

概述
----

本 project 是本人从事 Linux 开发以来，对 Linux / C 积累的一些基础知识的总结。内容比较多比较杂，大致分类如下：

### POSIX 和 ANSI-C ###

主要是 `AMCCommonLib.c/h` 文件，其中包含一些主要的标准系统 API 以及 POSIX API 的封装或者是原型注释。以 socket 为例，只需要在使用 socket 的头文件前面这么写：

    #define CFG_LIB_SOCKET
    #include "AMCCommonLib.h"


这样，`AMCCommonLib` 就帮你完成了相应头文件的包含。具体是怎么实现的，可以直接参照 `AMCCommonLib.h` 文件。


### 简单的工具封装 ###

在 ANSI-C 和 POSIX 之外，还有一些平常开发中经常使用到的函数和宏。我按照经验对这些功能封装了出来。同样是定义在 `AMCCommonLib.c/h` 库中。比如：

1. 加入了日期 / 时间打印（参照了 `NSLog()`）的输出函数 `AMCPrintf()`
2. 判断一个 POSIX 线程是否正在运行的函数 `pthread_alive()`
3. 安全地调用 shell 命令的函数 `AMCCommand()`
4. 位操作的宏 `SET_BITS()`、`CLR_BITS()`
5. 。。。。。。 

### 常用数据结构 ###

使用 C 往往会面临缺乏常用数据结构的窘境。实际上，这些数据结构的实现太简单了，以至于没有大的开源组织来完成这样的事情（自己写就好了）。但实际生活中又确实经常用，所以我就做上来了。

以下是我的库中提供的常用数据结构 / 算法工具

1. `AMCArray`：可变数组，保存 `void *` 数据，基于内存池实现
2. `AMCDictionary`：可变关联数组，保存 `void *` 数据，提供 key-value 关联数组。采用红黑树，基于内存池实现。
3. `AMCBinSearchTree`：二叉查找树，研究性代码，实用性不高，但是源代码可以用来拷贝从而实现具体功能
4. `AMCRedBlackTree`：红黑树，同上，研究性代码，不过其变体已经用在关联数组实现中。
5. `AMCBmp`：BMP 图读写，只支持 24bit 格式的位图，不支持 alpha 通道。有简单的直线和矩形绘制功能。
6. `AMCConfigParser`：配置文件读写，研究性代码。
7. `AMCHash`：简单哈希函数
8. `AMCMd5`：MD5
9. `AMCBase64`：Base64
10. `AMCMemPool`：内存池，目前只支持数据区块定长（内存池初始化时指定）的内存池
11. `AMCMutableBuffer`：类似于 Cocoa 的 `NSMutableData`，变长数据缓存，基于内存池实现。
12. `AMCRandom`：各种各样的随机函数
13. `AMCThreadPool`：线程池，基于内存池实现。

----

TBD
----

先完成本文档的英文版，然后后续如果有时间的话写一些各个库的具体说明好了——工作忙啊……


