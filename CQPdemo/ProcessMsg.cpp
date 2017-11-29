#include "stdafx.h"
#include "ProcessMsg.h"
#include "appmain.h"
#include "cqp.h"
#include "mtrand.hpp"
//#include <map>

// typedef std::map<int64_t, char*>    MapNickName;
// static MapNickName MapNickName_;
// 
// static CQTool QTool;
// static CQ_TYPE_QQ QInfo;
// 
// static char* help = ".help";
// static char* r = ".r";
// static char* rd = ".rd";
// static char* coc = "!coc";
// static char* coc7 = "!coc7";
// static char* nn = ".nn";

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
		ptr = strtok((char *)NULL, "");

		return SetNickName(ac, fromQQ, fromGroup, fromDiscuss, ptr);
	}

	//.jrrp����
	if (0 == strcmp(ptr, jrrp))
	{
		return RollFortune(ac, fromQQ, fromGroup, fromDiscuss);
	}

	return EVENT_IGNORE;
}

// Ͷ������
int Process::RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg)
{
	unsigned int sum = 0;
	unsigned int throwtimes = dicenum;
	mtsrand((unsigned)time(NULL));

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
int Process::RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum)
{
	unsigned int sum = 0;
	mtsrand((unsigned)time(NULL));
	
	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
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

	return EVENT_BLOCK;
}

// ���COC7������
int Process::RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum)
{
	unsigned int sum = 0;
	mtsrand((unsigned)time(NULL));
	
	const char* nick = GetNickName(ac, fromQQ, fromGroup, fromDiscuss);
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
	// ������ʱ�ǳ��б�
	MapNickName::iterator it = MapNickName_.find(fromQQ);
	if (it != MapNickName_.end())
	{
// 		char* nick = (*it).second;
// 		return nick;
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
			//		char nick[100];
			//		const char* tmp = QInfo.nick.c_str();
			//		strcpy(nick, tmp);
			//		return nick;
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

		sprintf(str, " * %s �����ǳ��� %s", oldNick, newNick);

		SendMsg(ac, fromQQ, fromGroup, fromDiscuss, str);
	}
	else
	{
		sprintf(str, " * %s ȡ�����Լ����ǳ�", oldNick);

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
	struct tm *t;
	time_t tt;
	time(&tt);
	t = localtime(&tt);

	if (lastday != t->tm_yday)
	{
		ClearFortune();
		lastday = t->tm_yday;
	}

	mtsrand((unsigned)time(NULL));

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
		fortune = mtirand() % 100 + 1;
		SetFortune(fromQQ, fortune);
	}
	
	sprintf(str, "* %s ���������ָ���� %u%% ! ", nick, fortune);

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
	MapFortune_[fromQQ] = fortune;
}

// ��������
void Process::ClearFortune()
{
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