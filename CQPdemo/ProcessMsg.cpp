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

// ������Ϣ
int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg, char* OutputMsg)
{
	char buf[2048];
	strcpy(buf, InputMsg);

	char *ptr = strtok(buf, " ");

	if (NULL == ptr)
	{
		return EVENT_IGNORE;
	}

	//.rd����
	if(0 == strcmp(ptr, rd))
	{
		ptr = strtok((char *)NULL, "");
		return RollDice(ac, fromQQ, fromGroup, fromDiscuss, 1, 100, ptr);
	}	
	
	//.r����
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

	//!coc����
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

	//!coc7����
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

	//.nn����
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
			// ������ʱ�ǳ�
			MapNickName_[fromQQ] = ptr;

			if (NULL != nick)
			{
				sprintf(str, " * %s �����ǳ��� %s", nick, ptr);
			}

			SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
		}
		else
		{
			// ȡ����ʱ�ǳ�
			MapNickName::iterator it = MapNickName_.find(fromQQ);
			if (it != MapNickName_.end())
			{
				MapNickName_.erase(it);
			}

			sprintf(str, " * %s ȡ�����Լ����ǳ�", nick);

			SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
		}	
	}

	return EVENT_IGNORE;
}

// Ͷ������
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
			sprintf(str, "* %s Ͷ�� %s : %dd%d+%d = ", nick, msg, throwtimes, sides, plus);
		}
		else if (mysubstr(msg, "-", tmp))
		{
			int minus = atoi(tmp);
			sprintf(str, "* %s Ͷ�� %s : %dd%d-%d = ", nick, msg, throwtimes, sides, minus);
			plus -= minus;
		}
		else
		{
			sprintf(str, "* %s Ͷ�� %s : %dd%d = ", nick, msg, throwtimes, sides);
		}
	}
	else
	{
		sprintf(str, "* %s Ͷ�� : %dd%d = ", nick, throwtimes, sides);
	}

	// ���������ܺ�
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

// ���COC6������
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
	sprintf(str, "* %s Ͷ��COC 6�� ���� :", nick);

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

	return EVENT_IGNORE;
}

// ���COC7������
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
	sprintf(str, "* %s Ͷ��COC 7�� ���� :", nick);

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

	return EVENT_IGNORE;
}

// Ͷ���������
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

// �ж�str1�Ƿ���str2��ͷ ����Ƿ���1 ���Ƿ���0 ������-1
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

// ���ַ���str�в���str0�����ƺ�������ݵ�str1����
int mysubstr(char *str, const char* str0, char str1[])
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
const char* GetNickName(int ac, int64_t fromQQ)
{
	// ������ʱ�ǳ��б�
	MapNickName::iterator it = MapNickName_.find(fromQQ);
	if (it != MapNickName_.end())
	{
		char* nick = (*it).second;
		return nick;
	}

	// ��ȡQQ�ǳ�
	if (QTool.GetStrangerInfo(ac, fromQQ, QInfo))
	{
//		char nick[100];
//		const char* tmp = QInfo.nick.c_str();
//		strcpy(nick, tmp);
//		return nick;
		return QInfo.nick.c_str();
	}

	// ��ȡʧ��
	return NULL;
}

//������Ϣ
void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg)
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