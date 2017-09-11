#include "stdafx.h"
#include "ProcessMsg.h"
#include "appmain.h"
#include "QTool.h"
#include "cqp.h"
#include "mtrand.hpp"
#include <map>

typedef std::map<int64_t, char*>    MapNickName;
static MapNickName MapNickName_;

static CQTool QTool;
static CQ_TYPE_QQ QInfo;

static char* help = ".help";
static char* r = ".r";
static char* rd = ".rd";
static char* coc = "!coc";
static char* coc7 = "!coc7";
static char* nn = ".nn";

// 处理消息
int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg, char* OutputMsg)
{
	char buf[2048];
	strcpy(buf, InputMsg);

	char *ptr = strtok(buf, " ");

	if (NULL == ptr)
	{
		return EVENT_IGNORE;
	}

	//.rd命令
	if(0 == strcmp(ptr, rd))
	{
		ptr = strtok((char *)NULL, "");
		return RollDice(ac, fromQQ, fromGroup, fromDiscuss, 1, 100, ptr);
	}	
	
	//.r命令
	if (0 == strcmp(ptr, r))
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
	if (0 == strcmp(ptr, coc))
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
	if (0 == strcmp(ptr, coc7))
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
	if (0 == strcmp(ptr, nn))
	{
		char str[2048];
		const char* nick = GetNickName(ac, fromQQ);
		if (NULL == nick)
		{
			return EVENT_IGNORE;
		}

		ptr = strtok((char *)NULL, "");

		if (NULL != ptr)
		{
			// 设置临时昵称
			MapNickName_[fromQQ] = ptr;

			if (NULL != nick)
			{
				sprintf(str, " * %s 的新昵称是 %s", nick, ptr);
			}

			SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
		}
		else
		{
			// 取消临时昵称
			MapNickName::iterator it = MapNickName_.find(fromQQ);
			if (it != MapNickName_.end())
			{
				MapNickName_.erase(it);
			}

			sprintf(str, " * %s 取消了自己的昵称", nick);

			SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
		}	
	}

	return EVENT_IGNORE;
}

// 投掷骰子
int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg)
{
	unsigned int sum = 0;
	unsigned int throwtimes = dicenum;
	mtsrand((unsigned)time(NULL));

	char str[2048];
	int plus = 0;

	const char* nick = GetNickName(ac, fromQQ);
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
//			unsigned int randnum = rand() % sides + 1;
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
int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum)
{
	unsigned int sum = 0;
	mtsrand((unsigned)time(NULL));
	
	const char* nick = GetNickName(ac, fromQQ);
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

	return EVENT_IGNORE;
}

// 随机COC7版属性
int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum)
{
	unsigned int sum = 0;
	mtsrand((unsigned)time(NULL));
	
	const char* nick = GetNickName(ac, fromQQ);
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

	return EVENT_IGNORE;
}

// 投掷多个骰子
unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides)
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
int is_begin_with(const char * str1, char *str2)
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
int mysubstr(char *str, const char* str0, char str1[])
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
const char* GetNickName(int ac, int64_t fromQQ)
{
	// 查找临时昵称列表
	MapNickName::iterator it = MapNickName_.find(fromQQ);
	if (it != MapNickName_.end())
	{
		char* nick = (*it).second;
		return nick;
	}

	// 获取QQ昵称
	if (QTool.GetStrangerInfo(ac, fromQQ, QInfo))
	{
//		char nick[100];
//		const char* tmp = QInfo.nick.c_str();
//		strcpy(nick, tmp);
//		return nick;
		return QInfo.nick.c_str();
	}

	// 获取失败
	return NULL;
}

//发送消息
void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg)
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