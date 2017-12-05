#pragma once
#include <time.h>
#include <map>
#include "QTool.h"
#include "XAutoLock.h"

typedef std::map<int64_t, char*>        MapNickName;
typedef std::map<int64_t, unsigned int> MapFortune;

class Process
{
public:
	Process();
	~Process();
public:
	static Process&Instance(void);
public:
	// 处理消息
	int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg);

	// 投掷骰子
	int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg);

	// 随机COC6版属性
	int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// 随机COC7版属性
	int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// 投掷多个骰子
	unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);

	// 获取昵称
	const char* GetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// 设置昵称
	int SetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* nick);

	// 清理昵称
	void ClearNickName();

	// 投掷运气
	int RollFortune(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// 获取运气
	unsigned int GetFortune(int64_t fromQQ);

	// 设置运气
	void SetFortune(int64_t fromQQ, int fortune);

	// 清理运气
	void ClearFortune();

	//发送消息
	void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);

	// 判断str1是否以str2开头 如果是返回1 不是返回0 出错返回-1
	int is_begin_with(const char * str1, char *str2);

	// 从字符串str中查找str0，复制后面的内容到str1里面
	int mysubstr(char *str, const char* str0, char str1[]);

	// 全角转半角
	void sbc_to_dbc(char *sbc, char *dbc);

private:

	XCritSec    csMapNickName_;
	MapNickName MapNickName_;

	XCritSec    csMapFortune_;
	MapFortune  MapFortune_;

	XCritSec    csRollDice;

	CQTool QTool;
	CQ_TYPE_QQ QInfo;
	CQ_Type_GroupMember QGroupInfo;

	const char* help;
	const char* r;
	const char* rd;
	const char* coc;
	const char* coc7;
	const char* nn;
	const char* jrrp;

	int lastday;
};

// // 处理消息
// int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg, char* OutputMsg);
// 
// // 投掷骰子
// int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg);
// 
// // 随机COC6版属性
// int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);
// 
// // 随机COC7版属性
// int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);
// 
// // 投掷多个骰子
// unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);
// 
// // 判断str1是否以str2开头 如果是返回1 不是返回0 出错返回-1
// int is_begin_with(const char * str1, char *str2);
// 
// // 从字符串str中查找str0，复制后面的内容到str1里面
// int mysubstr(char *str, const char* str0, char str1[]);
// 
// // 获取昵称
// const char* GetNickName(int ac, int64_t fromQQ);
// 
// //发送消息
// void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);