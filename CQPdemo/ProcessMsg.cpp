#include "stdafx.h"
#include "ProcessMsg.h"
#include "appmain.h"
#include "cqp.h"
#include "mtrand.hpp"
#include <string.h>
#include <timeapi.h>

#pragma comment(lib,"winmm.lib")

static Process g_Process;

Process&Process::Instance(void)
{
	return g_Process;
}

Process::Process()
{
	help = ".help";
	r = ".r";
	rd = ".rd";
	coc = "!coc";
	coc7 = "!coc7";
	nn = ".nn";
	jrrp = ".jrrp";

	lastday = 0;
}

Process::~Process()
{
	ClearNickName();
}

// 处理消息
int Process::ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg)
{
	if (NULL == InputMsg)
	{
		return EVENT_IGNORE;
	}

	char buf[2048];
	char dbc[2048];
	strcpy(buf, InputMsg);

	//全角转半角
	sbc_to_dbc(buf, dbc);
	strcpy(buf, dbc);

	//处理开头的中文句号
	if (0 == strncmp(buf, "。", strlen("。")))
	{
		char tmp[2048];
		strcpy(tmp, ".");
		strcat(tmp, buf+2);
		strcpy(buf,tmp);
	}

	char *ptr = strtok(buf, " ");

	if (NULL == ptr)
	{
		return EVENT_IGNORE;
	}

	//.rd命令
	if(0 == stricmp(ptr, rd))
	{
		ptr = strtok((char *)NULL, "");
		return RollDice(ac, fromQQ, fromGroup, fromDiscuss, 1, 100, ptr);
	}	
	
	//.r命令
	if (0 == stricmp(ptr, r))
	{
		ptr = strtok((char *)NULL, "d");
		unsigned int dicenum = atoi(ptr);
		int sides = 100;
		if (1<= dicenum && dicenum <=100)
		{
			char* plus = ptr;
			ptr = strtok((char *)NULL, "d");
			if (NULL != ptr)
			{
				sides = atoi(ptr);
				ptr = strtok((char *)NULL, " ");
			}

			return RollDice(ac, fromQQ, fromGroup, fromDiscuss, dicenum, sides, ptr);
		}

		return EVENT_IGNORE;
	}

	//!coc命令
	if (0 == stricmp(ptr, coc))
	{
		ptr = strtok((char *)NULL, "");

		unsigned int attributes_num = 1;
		if (NULL != ptr)
		{
			attributes_num = atoi(ptr);
		}
		
		return RollAttributes(ac, fromQQ, fromGroup, fromDiscuss, attributes_num);
	}

	//!coc7命令
	if (0 == stricmp(ptr, coc7))
	{
		ptr = strtok((char *)NULL, "");

		unsigned int attributes_num = 1;
		if (NULL != ptr)
		{
			attributes_num = atoi(ptr);
		}

		return RollAttributes7(ac, fromQQ, fromGroup, fromDiscuss, attributes_num);
	}

	//.nn命令
	if (0 == stricmp(ptr, nn))
	{
		ptr = strtok((char *)NULL, "");

		return SetNickName(ac, fromQQ, fromGroup, fromDiscuss, ptr);
	}

	//.jrrp命令
	if (0 == stricmp(ptr, jrrp))
	{
		return RollFortune(ac, fromQQ, fromGroup, fromDiscuss);
	}

	return EVENT_IGNORE;
}

// 投掷骰子
int Process::RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg)
{
	XAutoLock l(csRollDice);
	unsigned int sum = 0;
	unsigned int throwtimes = dicenum;
	mtsrand((unsigned)(::timeGetTime() + fromQQ));

	char str[2048];
	int plus = 0;

	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == nick)
	{
		return EVENT_IGNORE;
	}

	if (msg)
	{
		char tmp[100];
		if (mysubstr(msg, "+", tmp))
		{
			plus = atoi(tmp);
			sprintf(str, "* %s 投掷 %s : %dd%d+%d = ", nick, msg, throwtimes, sides, plus);
		}
		else if (mysubstr(msg, "-", tmp))
		{
			int minus = atoi(tmp);
			sprintf(str, "* %s 投掷 %s : %dd%d-%d = ", nick, msg, throwtimes, sides, minus);
			plus -= minus;
		}
		else
		{
			sprintf(str, "* %s 投掷 %s : %dd%d = ", nick, msg, throwtimes, sides);
		}
	}
	else
	{
		sprintf(str, "* %s 投掷 : %dd%d = ", nick, throwtimes, sides);
	}

	// 计算骰子总和
	for (size_t i = 0; i < throwtimes;)
	{
		unsigned int randnum = mtirand() % sides + 1;
		char string[10];
		itoa(randnum, string, 10);
		strcat(str, string);
		sum += randnum;

		i++;

		if (throwtimes > i)
		{
			strcat(str, "+");
		}
		else if(throwtimes == i && throwtimes != 1)
		{
			strcat(str, " = ");
			itoa(sum, string, 10);
			strcat(str, string);
		}

		Sleep(1);
	}

	SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);

	return EVENT_BLOCK;
	
}

// 随机COC6版属性
int Process::RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum)
{
	XAutoLock l(csRollDice);
	unsigned int sum = 0;
	mtsrand((unsigned)(::timeGetTime() + fromQQ));
	
	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == nick)
	{
		return EVENT_IGNORE;
	}

	char str[2048];
	sprintf(str, "* %s 投掷COC 6版 属性 :", nick);

	for (size_t i = 0; i < attributesnum; i++)
	{
		char string[100] = "";
		char attributes[10];
		unsigned int Str = MultiDiceSum(3, 6);    //力量

		strcat(string, "\n力量 ");
		if (Str < 10)
		{
			strcat(string, "  ");
		}
		itoa(Str, attributes, 10);
		strcat(string, attributes);

		unsigned int Dex = MultiDiceSum(3, 6);    //敏捷

		strcat(string, " 敏捷 ");
		if (Dex < 10)
		{
			strcat(string, "  ");
		}
		itoa(Dex, attributes, 10);
		strcat(string, attributes);

		unsigned int Con = MultiDiceSum(3, 6);    //体质

		strcat(string, " 体质 ");
		if (Con < 10)
		{
			strcat(string, "  ");
		}
		itoa(Con, attributes, 10);
		strcat(string, attributes);

		unsigned int App = MultiDiceSum(3, 6);    //外貌

		strcat(string, " 外貌 ");
		if (App < 10)
		{
			strcat(string, "  ");
		}
		itoa(App, attributes, 10);
		strcat(string, attributes);

		unsigned int Pow = MultiDiceSum(3, 6);    //意志

		strcat(string, " 意志 ");
		if (Pow < 10)
		{
			strcat(string, "  ");
		}
		itoa(Pow, attributes, 10);
		strcat(string, attributes);

		unsigned int Int = MultiDiceSum(2, 6) + 6;//智力

		strcat(string, " 智力 ");
		if (Int < 10)
		{
			strcat(string, "  ");
		}
		itoa(Int, attributes, 10);
		strcat(string, attributes);

		unsigned int Siz = MultiDiceSum(2, 6) + 6;//体型

		strcat(string, " 体型 ");
		if (Siz < 10)
		{
			strcat(string, "  ");
		}
		itoa(Siz, attributes, 10);
		strcat(string, attributes);

		unsigned int Edu = MultiDiceSum(3, 6) + 3;//教育

		strcat(string, " 教育 ");
		if (Edu < 10)
		{
			strcat(string, "  ");
		}
		itoa(Edu, attributes, 10);
		strcat(string, attributes);

		unsigned int Assets = MultiDiceSum(1, 10);//资产

		strcat(string, " 资产 ");
		if (Assets < 10)
		{
			strcat(string, "  ");
		}
		itoa(Assets, attributes, 10);
		strcat(string, attributes);

		strcat(str, string);
	}

	SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);

	return EVENT_BLOCK;
}

// 随机COC7版属性
int Process::RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum)
{
	XAutoLock l(csRollDice);
	unsigned int sum = 0;
	mtsrand((unsigned)(::timeGetTime() + fromQQ));
	
	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == nick)
	{
		return EVENT_IGNORE;
	}
	char str[2048];
	sprintf(str, "* %s 投掷COC 7版 属性 :", nick);

	for (size_t i = 0; i < attributesnum; i++)
	{
		char string[100] = "";
		char attributes[10];
		unsigned int Str = MultiDiceSum(3, 6) * 5;    //力量

		strcat(string, "\n力量 ");
		if (Str < 10)
		{
			strcat(string, "  ");
		}
		itoa(Str, attributes, 10);
		strcat(string, attributes);

		unsigned int Dex = MultiDiceSum(3, 6) * 5;    //敏捷

		strcat(string, " 敏捷 ");
		if (Dex < 10)
		{
			strcat(string, "  ");
		}
		itoa(Dex, attributes, 10);
		strcat(string, attributes);

		unsigned int Con = MultiDiceSum(3, 6) * 5;    //体质

		strcat(string, " 体质 ");
		if (Con < 10)
		{
			strcat(string, "  ");
		}
		itoa(Con, attributes, 10);
		strcat(string, attributes);

		unsigned int App = MultiDiceSum(3, 6) * 5;    //外貌

		strcat(string, " 外貌 ");
		if (App < 10)
		{
			strcat(string, "  ");
		}
		itoa(App, attributes, 10);
		strcat(string, attributes);

		unsigned int Pow = MultiDiceSum(3, 6) * 5;    //意志

		strcat(string, " 意志 ");
		if (Pow < 10)
		{
			strcat(string, "  ");
		}
		itoa(Pow, attributes, 10);
		strcat(string, attributes);

		unsigned int Int = (MultiDiceSum(2, 6) + 6) * 5;//智力

		strcat(string, " 智力 ");
		if (Int < 10)
		{
			strcat(string, "  ");
		}
		itoa(Int, attributes, 10);
		strcat(string, attributes);

		unsigned int Siz = (MultiDiceSum(2, 6) + 6) * 5;//体型

		strcat(string, " 体型 ");
		if (Siz < 10)
		{
			strcat(string, "  ");
		}
		itoa(Siz, attributes, 10);
		strcat(string, attributes);

		unsigned int Edu = (MultiDiceSum(2, 6) + 6) * 5;//教育

		strcat(string, " 教育 ");
		if (Edu < 10)
		{
			strcat(string, "  ");
		}
		itoa(Edu, attributes, 10);
		strcat(string, attributes);

		unsigned int Luc = MultiDiceSum(3, 6) * 5;//幸运

		strcat(string, " 幸运 ");
		if (Luc < 10)
		{
			strcat(string, "  ");
		}
		itoa(Luc, attributes, 10);
		strcat(string, attributes);

		strcat(str, string);
	}

	SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);

	return EVENT_BLOCK;
}

// 投掷多个骰子
unsigned int Process::MultiDiceSum(unsigned int dicenum, unsigned int sides)
{
	unsigned int sum = 0;
	for (size_t i = 0; i < dicenum; i++)
	{
		unsigned int randnum = mtirand() % sides + 1;
		sum += randnum;
		Sleep(1);
	}

	return sum;
}

// 判断str1是否以str2开头 如果是返回1 不是返回0 出错返回-1
int Process::is_begin_with(const char * str1, char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return -1;

	int len1 = strlen(str1);
	int len2 = strlen(str2);
	if ((len1 < len2) || (len1 == 0 || len2 == 0))
		return -1;

	char *p = str2;
	int i = 0;
	while (*p != '\0')
	{
		if (*p != str1[i])
			return 0;
		p++;
		i++;
	}
	return 1;
}

// 从字符串str中查找str0，复制后面的内容到str1里面
int Process::mysubstr(char *str, const char* str0, char str1[])
{
	str1[0] = 0;
	// 从字符串str中查找str0，如果存在，strp就是str0的开始位置
	char* strp = strstr(str, str0); 
	if (strp == NULL)
	{
		// 没有找到
		return 0; 
	}

	// 这个是字符串复制函数，strp+strlen(str0)就是str0后面的第一个字符的位置，从这个位置开始，复制后面的内容到str1里面
	strcpy(str1, strp + strlen(str0)); 

	return 1;
}

// 获取昵称
const char* Process::GetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss)
{
	XAutoLock l(csMapNickName_);
	// 查找临时昵称列表
	MapNickName::iterator it = MapNickName_.find(fromQQ);
	if (it != MapNickName_.end())
	{
		return (*it).second;
	}

	if (0 != fromGroup)
	{
		if (QTool.GetGroupMemberInfo(ac, fromGroup, fromQQ, QGroupInfo))
		{
			return QGroupInfo.nick.c_str();
		}
	}
	else
	{
		// 获取QQ昵称
		if (QTool.GetStrangerInfo(ac, fromQQ, QInfo))
		{
			return QInfo.nick.c_str();
		}
	}

	// 获取失败
	return NULL;
}

// 设置昵称
int Process::SetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* nick)
{
	char str[2048];
	char* deleteNick = NULL;
	const char* oldNick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == oldNick)
	{
		return EVENT_IGNORE;
	}

	XAutoLock l(csMapNickName_);
	// 取消临时昵称
	MapNickName::iterator it = MapNickName_.find(fromQQ);
	if (it != MapNickName_.end())
	{
		deleteNick = (*it).second;
		MapNickName_.erase(it);
	}

	if (NULL != nick)
	{
		// 设置临时昵称
		int len = strlen(nick);
		char* newNick = new char[len+1];
		if (NULL == newNick)
		{
			return EVENT_IGNORE;
		}

		strcpy(newNick, nick);

		MapNickName_[fromQQ] = newNick;

		sprintf(str, " * %s 的新昵称是 %s", oldNick, newNick);

		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
	}
	else
	{
		sprintf(str, " * %s 取消了自己的昵称", oldNick);

		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
	}

	if (NULL != deleteNick)
	{
		delete[] deleteNick;
		deleteNick = NULL;
	}

	return EVENT_BLOCK;
}

// 清理昵称
void Process::ClearNickName()
{
	XAutoLock l(csMapNickName_);
	char* clearNick = NULL;
	while (1)
	{
		MapNickName::iterator it = MapNickName_.begin();
		if (it != MapNickName_.end())
		{
			clearNick = (*it).second;
			MapNickName_.erase(it);
		}
		else
		{
			break;
		}

		if (clearNick != NULL)
		{
			delete[] clearNick;
			clearNick = NULL;
		}
		else
		{
			break;
		}
	}
}

// 投掷运气
int Process::RollFortune(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss)
{
	XAutoLock l(csRollDice);

	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);

	//日期变动后清空运气记录
	if (lastday != t->tm_yday)
	{
		ClearFortune();
		lastday = t->tm_yday;
	}

	char str[2048];
	int plus = 0;

	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == nick)
	{
		return EVENT_IGNORE;
	}

	unsigned int fortune = GetFortune(fromQQ);
	if (0 == fortune)
	{
		mtsrand((unsigned)(::timeGetTime() + fromQQ));
		fortune = mtirand() % 100 + 1;
		SetFortune(fromQQ, fortune);
	}
	
	sprintf(str, "* %s 今天的运势指数是 %u%% ! ", nick, fortune);

	for (int i = 0; i < fortune; i++)
	{
		strcat(str, "|");
	}

	SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);

	return EVENT_BLOCK;

}

// 获取运气
unsigned int Process::GetFortune(int64_t fromQQ)
{
	XAutoLock l(csMapFortune_);
	MapFortune::iterator it = MapFortune_.find(fromQQ);
	if (it != MapFortune_.end())
	{
		int fortune = (*it).second;
		return fortune;
	}

	return 0;
}

// 设置运气
void Process::SetFortune(int64_t fromQQ, int fortune)
{
	XAutoLock l(csMapFortune_);
	MapFortune_[fromQQ] = fortune;
}

// 清理运气
void Process::ClearFortune()
{
	XAutoLock l(csMapFortune_);
	MapFortune_.clear();
}

//发送消息
void Process::SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg)
{
	if (fromQQ)
	{
		if (fromGroup)
		{
			//发送群消息
			CQ_sendGroupMsg(ac, fromGroup, msg);
		}
		else if (fromDiscuss)
		{
			//发送讨论组消息
			CQ_sendDiscussMsg(ac, fromDiscuss, msg);
		}
		else
		{
			//发送私聊消息
			CQ_sendPrivateMsg(ac, fromQQ, msg);
		}
	}
}

// 全角转半角
void Process::sbc_to_dbc(char *sbc, char *dbc)
{
	for (; *sbc; ++sbc)
	{
		if ((*sbc & 0xff) == 0xA1 && (*(sbc + 1) & 0xff) == 0xA1)        //全角空格
		{
			*dbc++ = 0x20;
			++sbc;
		}
		else if ((*sbc & 0xff) == 0xA3 && (*(sbc + 1) & 0xff) >= 0xA1 && (*(sbc + 1) & 0xff) <= 0xFE)    //ASCII码中其它可显示字符
			*dbc++ = *++sbc - 0x80;
		else
		{
			if (*sbc < 0)    //如果是中文字符，则拷贝两个字节
				*dbc++ = *sbc++;
			*dbc++ = *sbc;
		}
	}
	*dbc = 0;
}