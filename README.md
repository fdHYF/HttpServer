# WangTian
**这是一个采用C++11编写的简易Linux HTTP服务器,目前仅支持GET和HEAD方法，可处理静态资源，支持HTTP长连接。项目学习了muduo，实现了异步日志的记录服务器运行状态。**
# 开发
由于我的系统是Window10,装虚拟机又十分的卡，开发是在VS2017中的Linux开发环境进行，基本编译通过后，在腾讯云服务器(Ubuntu 18.04)上进行压力测试。

 - 操作系统：Ubuntu 18.04
 - IDE：VS 2017
 - 编译器：gcc version 7.3.0
 - 版本控制：VS + git
 - 压力测试： webbench-1.5

# 用法
 `git clone https://github.com/fdHYF/HttpServer.git`  
 `cd HttpServer/HttpServer`  
 `cmake .`
 `make`
 `./WangTian`

#特点
 
 - 由priority_queue实现了小根堆的定时器，采用惰性删除方式
 - 支持HTTP长连接
 - 设计线程池，FIFO队列管理请求任务
 - 使用有限状态机解析HTTP请求，主状态机负责解析出一行，从状态机进行分析
 - 双缓冲区异步日志，实现了日志滚动
 - 广泛运用了RAII机制，避免内存泄漏
 
# 并发模型
本项目采用的是Reactor + 同步非阻塞IO + 线程池，epoll采用ET模式。主线程仅负责建立连接，然后返回就绪事件，加入线程池，由线程池线程负责读写，避免线程创建带来的开销。
关于阻塞、非阻塞、同步以及异步的详细介绍参见https://blog.csdn.net/historyasamirror/article/details/5778378。
### LOG的设计
LOG实现学习了muduo网络库，相关类包括LogFile、LogWrite、LogStream、Log
 
 - LogFile是对底层文件的分装，底层使用标准IO，封装了文件的写入、析构以及flush
 - LogWrite是LOG的核心，负责启动一个线程专门负责后端写，前端线程将日志写入缓冲区，后端线程负责将日志写入文件
 - LogStream实现格式化输出，重载了<<运算符，并未使用标准库的std::cout
 - Log是对外接口的封装，加上每条日志的格式化信息
 # 测试
 测试使用的机器是Ubuntu Server 18.04.1 LTS 64位 + 单核 + 2GB内存，公网带宽1Mbps。条件有限，进行本地测试。由于服务器是单核，测试时线程池仅开启一个    线程。分别进行了wget请求测试，firefox请求测试，以及webbench压力测试。
 ### wget
 ![](https://github.com/fdHYF/HttpServer/blob/master/picture/wget.png)
 ### firefox
 ![](https://github.com/fdHYF/HttpServer/blob/master/picture/firefox.png)
 ### webbench
 ![](https://github.com/fdHYF/HttpServer/blob/master/picture/webbench.png)
