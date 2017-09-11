#pragma once
#include <time.h>

// 处理消息
int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg, char* OutputMsg);

// 投掷骰子
int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg);

// 随机COC6版属性
int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

// 随机COC7版属性
int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

// 投掷多个骰子
unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);

// 判断str1是否以str2开头 如果是返回1 不是返回0 出错返回-1
int is_begin_with(const char * str1, char *str2);

// 从字符串str中查找str0，复制后面的内容到str1里面
int mysubstr(char *str, const char* str0, char str1[]);

// 获取昵称
const char* GetNickName(int ac, int64_t fromQQ);

//发送消息
void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);