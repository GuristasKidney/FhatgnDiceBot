#pragma once
#include <time.h>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include "QTool.h"
#include "XAutoLock.h"

typedef std::map<int64_t, char*>         MapNickName;//Map容器存放当前设置的昵称信息，key为QQ号，value为临时昵称
//typedef std::map<int64_t, unsigned int>  MapFortune;//Map容器存放本日运势，key为QQ号，value为运势指数
typedef std::multimap<int64_t, std::pair<int64_t, unsigned int>>  MapFortune;//Map容器存放本日运势，key为QQ号，value的first的群号，second为运势指数
typedef std::map<int64_t, std::pair<int64_t, int>>       Mapluckiest;//Map容器存放每个群本日运势最高的人，key为群号，value为QQ号
typedef std::vector<std::pair<int, int>> DiceVector;//vector队列存放当前骰点的指令，前者为骰子数，后者为面数，当后者为负数时代表此次计算为减法

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

	// 普通骰点消息处理
	int RollSegmentation(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg);

	// 投掷骰子
	int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg, int plus, bool secretly);

	// 随机COC6版属性
	int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// 随机COC7版属性
	int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// 投掷多个骰子
	unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);

	// 获取昵称
	const char* GetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, int mode=0);

	// 设置昵称
	int SetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* nick);

	// 清理昵称
	void ClearNickName();

	// 投掷运气
	int RollFortune(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// 获取运气
	unsigned int GetFortune(int64_t fromQQ);

	// 获取群运气列表
	int GetFortuneList(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// 设置运气
	void SetFortune(int64_t fromQQ, int64_t fromGroup, unsigned int fortune);

	// 清理运气
	void ClearFortune();

	// Luckiest消息处理
	int ProcessLuckiest(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// 获取Luckiest
	int GetLuckiest(int64_t fromGroup, int GetType);

	// 设置Luckiest
	void SetLuckiest(int64_t fromQQ, int64_t fromGroup, int forturn);

	// 清理Luckiest
	void ClearLuckiest();

	//发送消息
	void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);

	// 判断str1是否以str2开头 如果是返回1 不是返回0 出错返回-1
	int is_begin_with(const char * str1, char *str2);

	// 从字符串str中查找str0，复制后面的内容到str1里面
	int mysubstr(char *str, const char* str0, char str1[]);

	// 全角转半角
	void sbc_to_dbc(char *sbc, char *dbc);

	// 清除字符串前的空格
	void VS_StrLTrim(char *pStr);

private:

	XCritSec    csMapNickName_;
	MapNickName MapNickName_;//临时昵称容器

	XCritSec    csMapFortune_;
	MapFortune  MapFortune_;//运势指数容器，每日12点过后清空
	int lastday;//运势容器对应的日期

	XCritSec    csLuckiest_;
	Mapluckiest Mapluckiest_;//

	XCritSec    csRollDice_;
	DiceVector	DiceVect_;//骰子队列

	CQTool QTool;
	CQ_TYPE_QQ QInfo;
	CQ_Type_GroupMember QGroupInfo;

// 	unsigned int topfortune;//本日最高的运势值
// 	int64_t topQQ;//本日运势最高的QQ号

	//指令集
	const char* help;
	const char* r;
	const char* rd;
	const char* coc;
	const char* coc7;
	const char* nn;
	const char* jrrp;
	const char* luckiest;
	const char* rplist;
};