# WangTian
**这是一个采用C++11编写的简易Linux HTTP服务器,目前仅支持GET和HEAD方法，可处理静态资源，支持HTTP长连接。项目学习了muduo，实现了异步日志的记录服务器运行状态。**
# 开发
由于我的系统是Window10,装虚拟机又十分的卡，开发是在VS2017中的Linux开发环境进行，基本编译通过后，在腾讯云服务器(Ubuntu 16.04)上进行压力测试。

 - 操作系统：Ubuntu 16.04
 - IDE：VS 2017
 - 编译器：gcc version 7.3.0
 - 版本控制：VS + git
 - 压力测试： webbench-1.5

# 用法
 `git clone https://github.com/fdHYF/HttpServer.git`  
 `cd HttpServer/HttpServer`  
 `make`  
 `./WangTian`
