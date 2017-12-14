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

// ������Ϣ
int Process::ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg)
{
	if (NULL == InputMsg)
	{
		return EVENT_IGNORE;
	}

	char buf[2048];
	char dbc[2048];
	strcpy(buf, InputMsg);

	//ȫ��ת���
	sbc_to_dbc(buf, dbc);
	strcpy(buf, dbc);

	//����ͷ�����ľ��
	if (0 == strncmp(buf, "��", strlen("��")))
	{
		char tmp[2048];
		strcpy(tmp, ".");
		strcat(tmp, buf+2);
		strcpy(buf,tmp);
	}

	//.r����
	if (0 == strnicmp(buf, r, strlen(r)))
	{
		return RollSegmentation(ac, fromQQ, fromGroup, fromDiscuss, buf);
	}

	//.help����
	if (0 == strnicmp(buf, help, strlen(help)))
	{
		char helpmsg[] = "<FhatgnDice��version:2.0.0 by ����>\n\
< ָ��֧�� >\n\
- ����							.r 1d100+3\n\
- ����˽�Ž��						.rh 1d20\n\
- Ͷ�� coc����						!coc 5\n\
- Ͷ�� coc 7������						!coc7 5\n\
- Ͷ�� ��������						.jrrp\n\
- �����ǳ�							.nn ���ǳ�";
		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, helpmsg);
		return EVENT_BLOCK;
	}

	/************************************************************************
	* old 1.0.0
	* ���޸ģ�!coc����ϲ�
	*         ָ���ж�
	************************************************************************/

	char *ptr = strtok(buf, " ");

	if (NULL == ptr)
	{
		return EVENT_IGNORE;
	}

	//!coc����
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

	//!coc7����
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

	//.nn����
	if (0 == stricmp(ptr, nn))
	{
		ptr = strtok((char *)NULL, "");

		return SetNickName(ac, fromQQ, fromGroup, fromDiscuss, ptr);
	}

	//.jrrp����
	if (0 == stricmp(ptr, jrrp))
	{
		return RollFortune(ac, fromQQ, fromGroup, fromDiscuss);
	}

	return EVENT_IGNORE;
}

// ��ͨ������Ϣ����
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
		SendMsg(ac, fromQQ, fromGroup, fromGroup, "���������ʽ����,��ο�����ָ�� .help");
		return EVENT_BLOCK;
	}

	if (NULL == strstr(tmp, "d"))
	{
		SendMsg(ac, fromQQ, fromGroup, fromGroup, "���������ʽ����,��ο�����ָ�� .help");
		return EVENT_BLOCK;
	}

	VS_StrLTrim(tmp);//����ַ���ǰ�Ŀո�

	if ('h' == tmp[0])
	{
		secretly = true;
		tmp++;
	}

	VS_StrLTrim(tmp);//����ַ���ǰ�Ŀո�

	while (1)
	{
		//û�к����ַ�����Ҫ����ʱֱ�����
		if (NULL == tmp)
		{
			return RollDice(ac, fromQQ, fromGroup, fromDiscuss, NULL, plus, secretly);
		}

		//�ж��Ƿ�roll��ָ��
		if (NULL != strstr(tmp, "d"))
		{
			char *cmdL1 = NULL;
			if ('d' == tmp[0])//strtok��ʶ���ַ����ײ��ķָ���
			{
				cmdL1 = tmp + 1;
			}
			else
			{
				cmdL1 = strtok(tmp, "d");//��dΪ��ǩ���ָ�
			}

			//������Ĭ��Ϊ1
			if (NULL == cmdL1)
			{
				int dicenum = 1;
				//��ȡ��������
				char *cmdL2 = strtok((char*)NULL, "d");

				if (NULL == cmdL2)
				{
					diceVect.push_back(std::make_pair(1, 100));
					tmp = NULL;
					continue;
				}

				//�������ֺ�����
				int i = 0;
				while (1)
				{
					if (cmdL2[i] >= '0'&&cmdL2[i] <= '9') //�������Ԫ��������
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
				//�ж����ַ�
				if (isdigit(cmdL1[0]))//������
				{
					if (strspn(cmdL1, "0123456789") == strlen(cmdL1))
					{
						int dicenum = atoi(cmdL1);

						//��ȡ��������
						char *cmdL2 = strtok((char*)NULL, "");

						if (NULL == cmdL2)
						{
							diceVect.push_back(std::make_pair(dicenum, 100));
							tmp = NULL;
							continue;
						}

						//�������ֺ�����
						int i = 0;
						while (1)
						{
							if (cmdL2[i] >= '0'&&cmdL2[i] <= '9') //�������Ԫ��������
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
					else if (0 == stricmp(cmdL1, tmp + 1))//�ַ���Ϊ"d...."��ʽ
					{
						int dicenum = 1;
						//�������ֺ�����
						int i = 0;
						while (1)
						{
							if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //�������Ԫ��������
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
						//ָ�����
						diceVect.clear();
						SendMsg(ac, fromQQ, fromGroup, fromGroup,"���������ʽ����,��ο�����ָ�� .help");
						return EVENT_BLOCK;
					}
				}
				else if (0 == strncmp(cmdL1, "+", strlen("+")) || 0 == strncmp(cmdL1, "-", strlen("-")))//�Ӽ���
				{
					if (strspn(cmdL1 + 1, "0123456789") == (strlen(cmdL1) - 1))
					{
						int dicenum = 1;
						if (strlen(cmdL1) > 1)//�ų�+d��-d��ʽ
						{
							dicenum = atoi(cmdL1);
						}
						else if (0 == strncmp(cmdL1, "-", strlen("-")))
						{
							dicenum = -1;
						}

						//��ȡ��������
						char *cmdL2 = strtok((char*)NULL, "");

						if (NULL == cmdL2)
						{
							diceVect.push_back(std::make_pair(dicenum, 100));
							tmp = NULL;
							continue;
						}

						//�������ֺ�����
						int i = 0;
						while (1)
						{
							if (cmdL2[i] >= '0'&&cmdL2[i] <= '9') //�������Ԫ��������
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
					else if (0 == stricmp(cmdL1, tmp + 1))//�ַ���Ϊ"d...."��ʽ
					{
						int dicenum = 1;
						//�������ֺ�����
						int i = 0;
						while (1)
						{
							if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //�������Ԫ��������
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
					else if (isdigit(cmdL1[1]))//�ж����ַ�������
					{
						//�������ֺ�����
						int i = 1;
						while (1)
						{
							if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //�������Ԫ��������
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
				else if (0 == stricmp(cmdL1, tmp + 1))//�ַ���Ϊ"d...."��ʽ
				{
					int dicenum = 1;
					//�������ֺ�����
					int i = 0;
					while (1)
					{
						if (cmdL1[i] >= '0'&&cmdL1[i] <= '9') //�������Ԫ��������
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
			//�ж����ַ�
			if (0 == strncmp(tmp, "+", strlen("+")) || 0 == strncmp(tmp, "-", strlen("-")))//�Ӽ���
			{
				if (isdigit(tmp[1]))//������
				{
					//�������ֺ�����
					int i = 1;
					while (1)
					{
						if (tmp[i] >= '0'&&tmp[i] <= '9') //�������Ԫ��������
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

	SendMsg(ac, fromQQ, fromGroup, fromGroup, "���������ʽ����,��ο�����ָ�� .help");
	return EVENT_BLOCK;
}

// Ͷ������
int Process::RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg, int plus, bool secretly)
{
	int sum = 0;
	char formula[1024] = "";
	char result[1024] = "";
	char outputfmt[2048];
	char output[2048];
	sprintf_s(outputfmt, "* %%s Ͷ�� %%s : %%s = %%s");

	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
	if (NULL == nick)
	{
		diceVect.clear();
		return EVENT_IGNORE;
	}

	//�������������
	mtsrand((unsigned)(::timeGetTime() + fromQQ));

	//��ȡ���г���
	size_t len = diceVect.size();
	if (len <= 0)
	{
		//û��Ͷ������
		return EVENT_IGNORE;
	}

	if (NULL == msg)
	{
		//��עΪ��
		msg = "";
	}
	else
	{
		VS_StrLTrim(msg);//����ַ���ǰ�Ŀո�
	}

	//�����ʽ
	for (size_t i = 0; i < len; i++)
	{
		if (0 == diceVect[i].first)
		{
			char outputerr[256];
			sprintf_s(outputerr, "* %s ����һ�ſ������������ϡ�", nick);
			SendMsg(ac, fromQQ, fromGroup, fromGroup, outputerr);
			diceVect.clear();
			return EVENT_BLOCK;
		}
		else if (0 == diceVect[i].second)
		{
			char outputerr[256];
			sprintf_s(outputerr, "* %s �ó���һ�����Ԫ���� û���˿��õ����Ľ����", nick);
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

	//��ɽ��
	for (size_t i = 0; i < len; i++)
	{
		unsigned int dicenum = 0;
		int rollret = 1;
		char resulttmp[512] = "";
		char tmp[10] = "";
		char rolltmp[512] = "";
		char fmt[512] = "";//fmt��ʽ����


		if (diceVect[i].first > 0)//���ӷ�
		{
			if (i != 0)
			{
				strcat_s(fmt, 512, "+");
			}
			dicenum = diceVect[i].first;
		}
		else if (diceVect[i].first < 0)//������
		{
			strcat_s(fmt, 512, "-");
			dicenum -= diceVect[i].first;
		}

		//һ����Ͷ��������ӵĽ�������ű�ʶ
		if (dicenum > 1)
		{
			strcat_s(fmt, 512, "(%s)");
		}
		else
		{
			strcat_s(fmt, 512, "%s");
		}

		//Ͷ������
		for (int j = 0; j < dicenum; j++)
		{
			int sides = diceVect[i].second;
			if (j != 0)
			{
				strcat_s(rolltmp, 512, "��");
			}

			//���������
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

		//���˴�Ͷ��������뵽����ַ�����
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
		sprintf_s(outputh, "* %s Ͷ��һ���������� ������Щ�����ǿ�������", nick);
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

// ���COC6������
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
	sprintf_s(str, "* %s Ͷ��COC 6�� ���� :", nick);

	for (size_t i = 0; i < attributesnum; i++)
	{
		char string[100] = "";
		char attributes[10];
		unsigned int Str = MultiDiceSum(3, 6);    //����

		strcat(string, "\n���� ");
		if (Str < 10)
		{
			strcat(string, "  ");
		}
		itoa(Str, attributes, 10);
		strcat(string, attributes);

		unsigned int Dex = MultiDiceSum(3, 6);    //����

		strcat(string, " ���� ");
		if (Dex < 10)
		{
			strcat(string, "  ");
		}
		itoa(Dex, attributes, 10);
		strcat(string, attributes);

		unsigned int Con = MultiDiceSum(3, 6);    //����

		strcat(string, " ���� ");
		if (Con < 10)
		{
			strcat(string, "  ");
		}
		itoa(Con, attributes, 10);
		strcat(string, attributes);

		unsigned int App = MultiDiceSum(3, 6);    //��ò

		strcat(string, " ��ò ");
		if (App < 10)
		{
			strcat(string, "  ");
		}
		itoa(App, attributes, 10);
		strcat(string, attributes);

		unsigned int Pow = MultiDiceSum(3, 6);    //��־

		strcat(string, " ��־ ");
		if (Pow < 10)
		{
			strcat(string, "  ");
		}
		itoa(Pow, attributes, 10);
		strcat(string, attributes);

		unsigned int Int = MultiDiceSum(2, 6) + 6;//����

		strcat(string, " ���� ");
		if (Int < 10)
		{
			strcat(string, "  ");
		}
		itoa(Int, attributes, 10);
		strcat(string, attributes);

		unsigned int Siz = MultiDiceSum(2, 6) + 6;//����

		strcat(string, " ���� ");
		if (Siz < 10)
		{
			strcat(string, "  ");
		}
		itoa(Siz, attributes, 10);
		strcat(string, attributes);

		unsigned int Edu = MultiDiceSum(3, 6) + 3;//����

		strcat(string, " ���� ");
		if (Edu < 10)
		{
			strcat(string, "  ");
		}
		itoa(Edu, attributes, 10);
		strcat(string, attributes);

		unsigned int Assets = MultiDiceSum(1, 10);//�ʲ�

		strcat(string, " �ʲ� ");
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

// ���COC7������
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
	sprintf_s(str, "* %s Ͷ��COC 7�� ���� :", nick);

	for (size_t i = 0; i < attributesnum; i++)
	{
		char string[100] = "";
		char attributes[10];
		unsigned int Str = MultiDiceSum(3, 6) * 5;    //����

		strcat(string, "\n���� ");
		if (Str < 10)
		{
			strcat(string, "  ");
		}
		itoa(Str, attributes, 10);
		strcat(string, attributes);

		unsigned int Dex = MultiDiceSum(3, 6) * 5;    //����

		strcat(string, " ���� ");
		if (Dex < 10)
		{
			strcat(string, "  ");
		}
		itoa(Dex, attributes, 10);
		strcat(string, attributes);

		unsigned int Con = MultiDiceSum(3, 6) * 5;    //����

		strcat(string, " ���� ");
		if (Con < 10)
		{
			strcat(string, "  ");
		}
		itoa(Con, attributes, 10);
		strcat(string, attributes);

		unsigned int App = MultiDiceSum(3, 6) * 5;    //��ò

		strcat(string, " ��ò ");
		if (App < 10)
		{
			strcat(string, "  ");
		}
		itoa(App, attributes, 10);
		strcat(string, attributes);

		unsigned int Pow = MultiDiceSum(3, 6) * 5;    //��־

		strcat(string, " ��־ ");
		if (Pow < 10)
		{
			strcat(string, "  ");
		}
		itoa(Pow, attributes, 10);
		strcat(string, attributes);

		unsigned int Int = (MultiDiceSum(2, 6) + 6) * 5;//����

		strcat(string, " ���� ");
		if (Int < 10)
		{
			strcat(string, "  ");
		}
		itoa(Int, attributes, 10);
		strcat(string, attributes);

		unsigned int Siz = (MultiDiceSum(2, 6) + 6) * 5;//����

		strcat(string, " ���� ");
		if (Siz < 10)
		{
			strcat(string, "  ");
		}
		itoa(Siz, attributes, 10);
		strcat(string, attributes);

		unsigned int Edu = (MultiDiceSum(2, 6) + 6) * 5;//����

		strcat(string, " ���� ");
		if (Edu < 10)
		{
			strcat(string, "  ");
		}
		itoa(Edu, attributes, 10);
		strcat(string, attributes);

		unsigned int Luc = MultiDiceSum(3, 6) * 5;//����

		strcat(string, " ���� ");
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

// Ͷ���������
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

// �ж�str1�Ƿ���str2��ͷ ����Ƿ���1 ���Ƿ���0 ������-1
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

// ���ַ���str�в���str0�����ƺ�������ݵ�str1����
int Process::mysubstr(char *str, const char* str0, char str1[])
{
	str1[0] = 0;
	// ���ַ���str�в���str0��������ڣ�strp����str0�Ŀ�ʼλ��
	char* strp = strstr(str, str0); 
	if (strp == NULL)
	{
		// û���ҵ�
		return 0; 
	}

	// ������ַ������ƺ�����strp+strlen(str0)����str0����ĵ�һ���ַ���λ�ã������λ�ÿ�ʼ�����ƺ�������ݵ�str1����
	strcpy(str1, strp + strlen(str0)); 

	return 1;
}

// ��ȡ�ǳ�
const char* Process::GetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss)
{
	XAutoLock l(csMapNickName_);
	// ������ʱ�ǳ��б�
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
		// ��ȡQQ�ǳ�
		if (QTool.GetStrangerInfo(ac, fromQQ, QInfo))
		{
			return QInfo.nick.c_str();
		}
	}

	// ��ȡʧ��
	return NULL;
}

// �����ǳ�
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
	// ȡ����ʱ�ǳ�
	MapNickName::iterator it = MapNickName_.find(fromQQ);
	if (it != MapNickName_.end())
	{
		deleteNick = (*it).second;
		MapNickName_.erase(it);
	}

	if (NULL != nick)
	{
		// ������ʱ�ǳ�
		int len = strlen(nick);
		char* newNick = new char[len+1];
		if (NULL == newNick)
		{
			return EVENT_IGNORE;
		}

		strcpy(newNick, nick);

		MapNickName_[fromQQ] = newNick;

		sprintf_s(str, " * %s �����ǳ��� %s", oldNick, newNick);

		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
	}
	else
	{
		sprintf_s(str, " * %s ȡ�����Լ����ǳ�", oldNick);

		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
	}

	if (NULL != deleteNick)
	{
		delete[] deleteNick;
		deleteNick = NULL;
	}

	return EVENT_BLOCK;
}

// �����ǳ�
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

// Ͷ������
int Process::RollFortune(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss)
{
	XAutoLock l(csRollDice);

	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);

	//���ڱ䶯�����������¼
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
	
	sprintf_s(str, "* %s ���������ָ���� %u%% ! ", nick, fortune);

	for (int i = 0; i < fortune; i++)
	{
		strcat(str, "|");
	}

	SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);

	return EVENT_BLOCK;

}

// ��ȡ����
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

// ��������
void Process::SetFortune(int64_t fromQQ, int fortune)
{
	XAutoLock l(csMapFortune_);
	MapFortune_[fromQQ] = fortune;
}

// ��������
void Process::ClearFortune()
{
	XAutoLock l(csMapFortune_);
	MapFortune_.clear();
}

//������Ϣ
void Process::SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg)
{
	if (fromQQ)
	{
		if (fromGroup)
		{
			//����Ⱥ��Ϣ
			CQ_sendGroupMsg(ac, fromGroup, msg);
		}
		else if (fromDiscuss)
		{
			//������������Ϣ
			CQ_sendDiscussMsg(ac, fromDiscuss, msg);
		}
		else
		{
			//����˽����Ϣ
			CQ_sendPrivateMsg(ac, fromQQ, msg);
		}
	}
}

// ȫ��ת���
void Process::sbc_to_dbc(char *sbc, char *dbc)
{
	for (; *sbc; ++sbc)
	{
		if ((*sbc & 0xff) == 0xA1 && (*(sbc + 1) & 0xff) == 0xA1)        //ȫ�ǿո�
		{
			*dbc++ = 0x20;
			++sbc;
		}
		else if ((*sbc & 0xff) == 0xA3 && (*(sbc + 1) & 0xff) >= 0xA1 && (*(sbc + 1) & 0xff) <= 0xFE)    //ASCII������������ʾ�ַ�
			*dbc++ = *++sbc - 0x80;
		else
		{
			if (*sbc < 0)    //����������ַ����򿽱������ֽ�
				*dbc++ = *sbc++;
			*dbc++ = *sbc;
		}
	}
	*dbc = 0;
}

// ����ַ���ǰ�Ŀո�
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