# FhatgnDiceBot [![Build status](https://ci.appveyor.com/api/projects/status/b45ik9dass1rnrnj?svg=true)](https://ci.appveyor.com/project/Coxxs/cqsdk-vc)
COC跑团自用骰娘
基于个人兴趣临时起意写的，结构略显混乱，计划重构中。
基于酷Q V9 Visual C++ Demo 编写的应用。

指令支持
--------
- 骰点                                                          .r 1d100+3
- 暗骰 私信结果                                                 .rh 1d20
- 投掷 coc属性                                                  !coc 5
- 投掷 coc 7版属性                                              !coc7 5
- 投掷 本日运势                                                 .jrrp
- 获取 群运气列表 私信结果                                      .jrrplist
- 设置昵称                                                      .nn 新昵称

作者:腰子
QQ:236820311

文件说明
--------
`ProcessMsg.cpp` - 消息处理逻辑，包含了消息处理、指令解析、骰点计算等功能

`CQPdemo.sln` - 示例项目，可以直接在此基础上编写应用

您可以编译为 `com.example.democ.dll`，与 `CQPdemo/com.example.democ.json` 一起放置在 酷Q 的 app 目录下测试

`CQPdemo/com.example.democ.json` - 样例应用的对应信息文件，包含应用的基础信息、事件列表等，请放置在 酷Q 的 app 目录下（无需使用的事件、菜单、权限请在此删除）

`CQPdemo/cqp.h` - 酷Q SDK 头文件，通常无需修改

`CQPdemo/CQP.lib` - CQP.dll 的动态连接 .lib 文件，便于 C、C++ 等调用 酷Q 的方法。

官方网站
--------
主站：https://cqp.cc

文库：https://d.cqp.me
