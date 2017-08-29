#include "stdafx.h"
#include "ProcessMsg.h"
#include "appmain.h"
#include "QTool.h"
#include "cqp.h"
#include "mtrand.hpp"

int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg, char* OutputMsg)
{
//	CQTool QTool;
//	CQ_TYPE_QQ QInfo;
	char* r = ".r";
	char* rd = ".rd";
	char buf[2048];
	strcpy(buf, InputMsg);
	char *ptr = strtok(buf, " ");
//	if (1 == is_begin_with(InputMsg, rd))
	if(0 == strcmp(ptr, rd))
	{
		ptr = strtok((char *)NULL, "");
		return RollDice(ac, fromQQ, fromGroup, fromDiscuss, 1, 100, ptr);
		/*
		if (QTool.GetStrangerInfo(ac, fromQQ, QInfo))
		{
			srand((unsigned)time(0));
			int randnum = rand() % 100 + 1;
			char str[100];
			if (ptr)
			{
				sprintf(str, "* %s 投掷 %s : 1d100 = %d", QInfo.nick.c_str(), ptr, randnum);
			}
			else
			{
				sprintf(str, "* %s 投掷 : 1d100 = %d", QInfo.nick.c_str(), randnum);
			}
			
			CQ_sendPrivateMsg(ac, fromQQ, str);
			return EVENT_BLOCK;
		}
		*/
	}

	if (0 == strcmp(ptr, r))
	{
		ptr = strtok((char *)NULL, "d");
		unsigned int dicenum = atoi(ptr);
		if (1<= dicenum && dicenum <=100)
		{
			ptr = strtok((char *)NULL, "d");
			int sides = atoi(ptr);
			if (2 <= sides && sides <= 100)
			{
				return RollDice(ac, fromQQ, fromGroup, fromDiscuss, dicenum, sides, NULL);
			}
			
			return RollDice(ac, fromQQ, fromGroup, fromDiscuss, dicenum, 100, NULL);
		}
	}

	return EVENT_IGNORE;
}

int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg)
{
	CQTool QTool;
	CQ_TYPE_QQ QInfo;
	unsigned int sum = 0;
	unsigned int throwtimes = dicenum;
//	srand((unsigned)time(0));
	mtsrand((unsigned)time(NULL));
	if (QTool.GetStrangerInfo(ac, fromQQ, QInfo))
	{
		char str[2048];
		if (msg)
		{
			sprintf(str, "* %s 投掷 %s : %dd%d = ", QInfo.nick.c_str(), msg, throwtimes, sides);
		}
		else
		{
			sprintf(str, "* %s 投掷 : %dd%d = ", QInfo.nick.c_str(), throwtimes, sides);
		}

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
		}

		if (fromGroup)
		{
			CQ_sendGroupMsg(ac, fromGroup, str);
		}
		else if (fromDiscuss)
		{
			CQ_sendDiscussMsg(ac, fromDiscuss, str);
		}
		else
		{
			CQ_sendPrivateMsg(ac, fromQQ, str);
		}

		return EVENT_BLOCK;
	}
	return EVENT_IGNORE;
}

/**判断str1是否以str2开头
* 如果是返回1
* 不是返回0
* 出错返回-1
* */
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