#include "stdafx.h"
#include "ProcessMsg.h"
#include "appmain.h"
#include "cqp.h"
#include "mtrand.hpp"
#include <string.h>
#include <timeapi.h>
#include <shlwapi.h>

#pragma comment(lib,"shlwapi.lib")
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

	//.r命令
	if (0 == strnicmp(buf, r, strlen(r)))
	{
		return RollSegmentation(ac, fromQQ, fromGroup, fromDiscuss, buf);
	}

	//.help命令
	if (0 == strnicmp(buf, help, strlen(help)))
	{
		char helpmsg[] = "<FhatgnDice　version:2.0.0 by 腰子>\n\
< 指令支持 >\n\
- 骰点							.r 1d100+3\n\
- 暗骰私信结果						.rh 1d20\n\
- 投掷 coc属性						!coc 5\n\
- 投掷 coc 7版属性						!coc7 5\n\
- 投掷 本日运势						.jrrp\n\
- 设置昵称							.nn 新昵称";
		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, helpmsg);
		return EVENT_BLOCK;
	}

	/************************************************************************
	* old 1.0.0
	* 待修改：!coc命令合并
	*         指令判断
	************************************************************************/

	char *ptr = strtok(buf, " ");

	if (NULL == ptr)
	{
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

// 普通骰点消息处理
int Process::RollSegmentation(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg)
{
	XAutoLock l(csRollDice);
	char texttmp[2048];
	char text[2048];
	int plus = 0;
	bool secretly = false;
	char *tmp;

	strcpy(texttmp, msg);
	strlwr(msg);

	tmp = strtok(msg, ".r");

	if (NULL == tmp)
	{
		SendMsg(ac, fromQQ, fromGroup, fromGroup, "您的输入格式有误,请参考帮助指令 .help");
		return EVENT_BLOCK;
	}

	if (NULL == strstr(tmp, "d"))
	{
		SendMsg(ac, fromQQ, fromGroup, fromGroup, "您的输入格式有误,请参考帮助指令 .help");
		return EVENT_BLOCK;
	}

	VS_StrLTrim(tmp);//清除字符串前的空格

	if ('h' == tmp[0])
	{
		secretly = true;
		tmp++;
	}

	VS_StrLTrim(tmp);//清除字符串前的空格

	while (1)
	{
		//没有后续字符串需要处理时直接输出
		if (NULL == tmp)
		{
			return RollDice(ac, fromQQ, fromGroup, fromDiscuss, NULL, plus, secretly);
		}

		//判断是否含roll点指令
		if (NULL != strstr(tmp, "d"))
		{
			char *cmdL1 = NULL;
			if ('d' == tmp[0])//strtok不识别字符串首部的分隔符
			{
				cmdL1 = tmp + 1;
			}
			else
			{
				cmdL1 = strtok(tmp, "d");//以d为标签做分割
			}

			//骰子数默认为1
			if (NULL == cmdL1)
			{
				int dicenum = 1;
				//获取骰子面数
				char *cmdL2 = strtok((char*)NULL, "d");

				if (NULL == cmdL2)
				{
					diceVect.push_back(std::make_pair(1, 100));
					tmp = NULL;
					continue;
				}

				//分离数字和文字
				int i = 0;
				while (1)
				{
					if (cmdL2[i] >= '0'&&cmdL2[i] <= '9') //如果数组元素是数字
					{
						i++;
						continue;
					}
					else
					{
						char side[8];
						unsigned int sides = 0;

						if (i == 0)
						{
							sides = 100;
						}
						else
						{
							strncpy(side, cmdL2, i);
							strcpy(side + i, "\0");
							sides = atoi(side);
						}

						strcpy(text, cmdL2 + i);

						diceVect.push_back(std::make_pair(dicenum, sides));

						strcpy(tmp, text);

						break;
					}
				}
			}
			else
			{
				//判断首字符
				if (isdigit(cmdL1[0]))//是数字
				{
					if (strspn(cmdL1, "0123456789") == strlen(cmdL1))
					{
						int dicenum = atoi(cmdL1);

						//获取骰子面数
						char *cmdL2 = strtok((char*)NULL, "");

						if (NULL == cmdL2)
						{
							diceVect.push_back(std::make_pair(dicenum, 100));
							tmp = NULL;
							continue;
						}

						//分离数字和文字
						int i = 0;
						while (1)
						{
							if (cmdL2[i] >= '0'&&cmdL2[i] <= '9') //如果数组元素是数字
							{
								i++;
								continue;
							}
							else
							{
								char side[8];
								unsigned int sides = 0;

								if (i == 0)
								{
									sides = 100;
								}
								else
								{
									strncpy(side, cmdL2, i);
									strcpy(side + i, "\0");
									sides = atoi(side);
								}

								strcpy(text, cmdL2 + i);
								strcpy(tmp, text);

								diceVect.push_back(std::make_pair(dicenum, sides));
								break;
							}
						}
					}
					else if (0 == stricmp(cmdL1, tmp + 1))//字符串为"d...."格式
					{
						int dicenum = 1;
						//分离数字和文字
						int i = 0;
						while (1)
						{
							if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //如果数组元素是数字
							{
								i++;
								continue;
							}
							else
							{
								char side[8];
								unsigned int sides = 0;

								if (i == 0)
								{
									sides = 100;
								}
								else
								{
									strncpy(side, cmdL1, i);
									strcpy(side + i, "\0");
									sides = atoi(side);
								}

								strcpy(text, cmdL1 + i);
								strcpy(tmp, text);

								diceVect.push_back(std::make_pair(dicenum, sides));
								break;
							}
						}
					}
					else
					{
						//指令错误
						diceVect.clear();
						SendMsg(ac, fromQQ, fromGroup, fromGroup,"您的输入格式有误,请参考帮助指令 .help");
						return EVENT_BLOCK;
					}
				}
				else if (0 == strncmp(cmdL1, "+", strlen("+")) || 0 == strncmp(cmdL1, "-", strlen("-")))//加减法
				{
					if (strspn(cmdL1 + 1, "0123456789") == (strlen(cmdL1) - 1))
					{
						int dicenum = 1;
						if (strlen(cmdL1) > 1)//排除+d或-d格式
						{
							dicenum = atoi(cmdL1);
						}
						else if (0 == strncmp(cmdL1, "-", strlen("-")))
						{
							dicenum = -1;
						}

						//获取骰子面数
						char *cmdL2 = strtok((char*)NULL, "");

						if (NULL == cmdL2)
						{
							diceVect.push_back(std::make_pair(dicenum, 100));
							tmp = NULL;
							continue;
						}

						//分离数字和文字
						int i = 0;
						while (1)
						{
							if (cmdL2[i] >= '0'&&cmdL2[i] <= '9') //如果数组元素是数字
							{
								i++;
								continue;
							}
							else
							{
								char side[8];
								unsigned int sides = 0;

								if (i == 0)
								{
									sides = 100;
								}
								else
								{
									strncpy(side, cmdL2, i);
									strcpy(side + i, "\0");
									sides = atoi(side);
								}

								strcpy(text, cmdL2 + i);
								strcpy(tmp, text);

								diceVect.push_back(std::make_pair(dicenum, sides));
								break;
							}
						}
					}
					else if (0 == stricmp(cmdL1, tmp + 1))//字符串为"d...."格式
					{
						int dicenum = 1;
						//分离数字和文字
						int i = 0;
						while (1)
						{
							if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //如果数组元素是数字
							{
								i++;
								continue;
							}
							else
							{
								char side[8];
								unsigned int sides = 0;

								if (i == 0)
								{
									sides = 100;
								}
								else
								{
									strncpy(side, cmdL1, i);
									strcpy(side + i, "\0");
									sides = atoi(side);
								}

								strcpy(text, cmdL1 + i);
								strcpy(tmp, text);

								diceVect.push_back(std::make_pair(dicenum, sides));
								break;
							}
						}
					}
					else if (isdigit(cmdL1[1]))//判断首字符是数字
					{
						//分离数字和文字
						int i = 1;
						while (1)
						{
							if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //如果数组元素是数字
							{
								i++;
								continue;
							}
							else
							{
								char cplus[8];
								strncpy(cplus, cmdL1, i);
								strcpy(cplus + i, "\0");
								plus = atoi(cplus);

								char* msgtmp = StrStrI(texttmp, text + i);
								return RollDice(ac, fromQQ, fromGroup, fromDiscuss, msgtmp, plus, secretly);
							}
						}
					}
					else
					{
						char* msgtmp = StrStrI(texttmp, text);
						return RollDice(ac, fromQQ, fromGroup, fromDiscuss, msgtmp, plus, secretly);
					}
				}
				else if (0 == stricmp(cmdL1, tmp + 1))//字符串为"d...."格式
				{
					int dicenum = 1;
					//分离数字和文字
					int i = 0;
					while (1)
					{
						if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //如果数组元素是数字
						{
							i++;
							continue;
						}
						else
						{
							char side[8];
							unsigned int sides = 0;

							if (i == 0)
							{
								sides = 100;
							}
							else
							{
								strncpy(side, cmdL1, i);
								strcpy(side + i, "\0");
								sides = atoi(side);
							}

							strcpy(text, cmdL1 + i);
							strcpy(tmp, text);

							diceVect.push_back(std::make_pair(dicenum, sides));
							break;
						}
					}
				}
				else
				{
					char* msgtmp = StrStrI(texttmp, text);
					return RollDice(ac, fromQQ, fromGroup, fromDiscuss, msgtmp, plus, secretly);
				}
			}
		}
		else
		{
			//判断首字符
			if (0 == strncmp(tmp, "+", strlen("+")) || 0 == strncmp(tmp, "-", strlen("-")))//加减法
			{
				if (isdigit(tmp[1]))//是数字
				{
					//分离数字和文字
					int i = 1;
					while (1)
					{
						if (tmp[i] >= '0'&&tmp[i] <= '9') //如果数组元素是数字
						{
							i++;
							continue;
						}
						else
						{
							char cplus[8];
							strncpy(cplus, tmp, i);
							strcpy(cplus + i, "\0");
							plus = atoi(cplus);

							char* msgtmp = StrStrI(texttmp, text + i);
							return RollDice(ac, fromQQ, fromGroup, fromDiscuss, msgtmp, plus, secretly);
						}
					}
				}
				else
				{
					char* msgtmp = StrStrI(texttmp, text);
					return RollDice(ac, fromQQ, fromGroup, fromDiscuss, msgtmp, plus, secretly);
				}
			}
			else
			{
				char* msgtmp = StrStrI(texttmp, text);
				return RollDice(ac, fromQQ, fromGroup, fromDiscuss, msgtmp, plus, secretly);
			}
		}
	}

	diceVect.clear();

	SendMsg(ac, fromQQ, fromGroup, fromGroup, "您的输入格式有误,请参考帮助指令 .help");
	return EVENT_BLOCK;
}

// 投掷骰子
int Process::RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg, int plus, bool secretly)
{
	int sum = 0;
	char formula[1024] = "";
	char result[1024] = "";
	char outputfmt[2048];
	char output[2048];
	sprintf_s(outputfmt, "* %%s 投掷 %%s : %%s = %%s");

	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == nick)
	{
		diceVect.clear();
		return EVENT_IGNORE;
	}

	//生成随机数种子
	mtsrand((unsigned)(::timeGetTime() + fromQQ));

	//获取队列长度
	size_t len = diceVect.size();
	if (len <= 0)
	{
		//没有投掷骰子
		return EVENT_IGNORE;
	}

	if (NULL == msg)
	{
		//标注为空
		msg = "";
	}
	else
	{
		VS_StrLTrim(msg);//清除字符串前的空格
	}

	//组成算式
	for (size_t i = 0; i < len; i++)
	{
		if (0 == diceVect[i].first)
		{
			char outputerr[256];
			sprintf_s(outputerr, "* %s 丢了一团空气到了桌子上。", nick);
			SendMsg(ac, fromQQ, fromGroup, fromGroup, outputerr);
			diceVect.clear();
			return EVENT_BLOCK;
		}
		else if (0 == diceVect[i].second)
		{
			char outputerr[256];
			sprintf_s(outputerr, "* %s 拿出了一个异次元骰子 没有人看得到它的结果。", nick);
			SendMsg(ac, fromQQ, fromGroup, fromGroup, outputerr);
			diceVect.clear();
			return EVENT_BLOCK;
		}

		char tmp[10];
		if (i != 0 && diceVect[i].first > 0)
		{
			strcat_s(formula, 512, "+");
		}

		sprintf_s(tmp, "%dd%d", diceVect[i].first, diceVect[i].second);
		strcat_s(formula, 512, tmp);
	}

	//组成结果
	for (size_t i = 0; i < len; i++)
	{
		unsigned int dicenum = 0;
		int rollret = 1;
		char resulttmp[512] = "";
		char tmp[10] = "";
		char rolltmp[512] = "";
		char fmt[512] = "";//fmt格式控制


		if (diceVect[i].first > 0)//做加法
		{
			if (i != 0)
			{
				strcat_s(fmt, 512, "+");
			}
			dicenum = diceVect[i].first;
		}
		else if (diceVect[i].first < 0)//做减法
		{
			strcat_s(fmt, 512, "-");
			dicenum -= diceVect[i].first;
		}

		//一次性投掷多个骰子的结果用括号标识
		if (dicenum > 1)
		{
			strcat_s(fmt, 512, "(%s)");
		}
		else
		{
			strcat_s(fmt, 512, "%s");
		}

		//投掷骰子
		for (int j = 0; j < dicenum; j++)
		{
			int sides = diceVect[i].second;
			if (j != 0)
			{
				strcat_s(rolltmp, 512, "、");
			}

			//生成随机数
			rollret = mtirand() % sides + 1;
			if (diceVect[i].first > 0)
			{
				sum += rollret;
			}
			else
			{
				sum -= rollret;
			}

			itoa(rollret, tmp, 10);
			strcat_s(rolltmp, 512, tmp);

			Sleep(1);
		}

		//将此次投掷结果输入到结果字符串中
		sprintf_s(resulttmp, fmt, rolltmp);
		strcat_s(result, resulttmp);
	}

	if (0 != plus)
	{
		char tmp[10] = "";
		itoa(plus, tmp, 10);

		if (plus > 0)
		{
			strcat_s(formula, 512, "+");
			strcat_s(result, 512, "+");
		}

		strcat_s(formula, 512, tmp);
		strcat_s(result, 512, tmp);
		sum += plus;
	}

	if (len > 1 || 0 != plus || diceVect[0].first > 1)
	{
		char tmp[10] = "";
		itoa(sum, tmp, 10);
		strcat_s(result, 512, " = ");
		strcat_s(result, 512, tmp);
	}

	sprintf_s(output, outputfmt, nick, msg, formula, result);

	if (secretly)
	{
		char outputh[512];
		sprintf_s(outputh, "* %s 投了一把隐形骰子 你们这些笨蛋是看不见的", nick);
		SendMsg(ac, fromQQ, fromGroup, fromGroup, outputh);
		SendMsg(ac, fromQQ, 0, 0, output);
	}
	else
	{
		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, output);
	}

	diceVect.clear();
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
	sprintf_s(str, "* %s 投掷COC 6版 属性 :", nick);

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
	sprintf_s(str, "* %s 投掷COC 7版 属性 :", nick);

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

		sprintf_s(str, " * %s 的新昵称是 %s", oldNick, newNick);

		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
	}
	else
	{
		sprintf_s(str, " * %s 取消了自己的昵称", oldNick);

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
	
	sprintf_s(str, "* %s 今天的运势指数是 %u%% ! ", nick, fortune);

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

// 清除字符串前的空格
void Process::VS_StrLTrim(char *pStr)
{
	char *pTmp = pStr;

	while (*pTmp == ' ')
	{
		pTmp++;
	}
	while (*pTmp != '\0')
	{
		*pStr = *pTmp;
		pStr++;
		pTmp++;
	}
	*pStr = '\0';
}