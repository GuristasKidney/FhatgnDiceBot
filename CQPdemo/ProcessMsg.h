#pragma once
#include <time.h>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include "QTool.h"
#include "XAutoLock.h"

typedef std::map<int64_t, char*>         MapNickName;//Map������ŵ�ǰ���õ��ǳ���Ϣ��keyΪQQ�ţ�valueΪ��ʱ�ǳ�
//typedef std::map<int64_t, unsigned int>  MapFortune;//Map������ű������ƣ�keyΪQQ�ţ�valueΪ����ָ��
typedef std::multimap<int64_t, std::pair<int64_t, unsigned int>>  MapFortune;//Map������ű������ƣ�keyΪQQ�ţ�value��first��Ⱥ�ţ�secondΪ����ָ��
typedef std::map<int64_t, std::pair<int64_t, int>>       Mapluckiest;//Map�������ÿ��Ⱥ����������ߵ��ˣ�keyΪȺ�ţ�valueΪQQ��
typedef std::vector<std::pair<int, int>> DiceVector;//vector���д�ŵ�ǰ�����ָ�ǰ��Ϊ������������Ϊ������������Ϊ����ʱ����˴μ���Ϊ����

class Process
{
public:
	Process();
	~Process();
public:
	static Process&Instance(void);
public:
	// ������Ϣ
	int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg);

	// ��ͨ������Ϣ����
	int RollSegmentation(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg);

	// Ͷ������
	int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* msg, int plus, bool secretly);

	// ���COC6������
	int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// ���COC7������
	int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// Ͷ���������
	unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);

	// ��ȡ�ǳ�
	const char* GetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, int mode=0);

	// �����ǳ�
	int SetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* nick);

	// �����ǳ�
	void ClearNickName();

	// Ͷ������
	int RollFortune(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// ��ȡ����
	unsigned int GetFortune(int64_t fromQQ);

	// ��ȡȺ�����б�
	int GetFortuneList(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// ��������
	void SetFortune(int64_t fromQQ, int64_t fromGroup, unsigned int fortune);

	// ��������
	void ClearFortune();

	// Luckiest��Ϣ����
	int ProcessLuckiest(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// ��ȡLuckiest
	int GetLuckiest(int64_t fromGroup, int GetType);

	// ����Luckiest
	void SetLuckiest(int64_t fromQQ, int64_t fromGroup, int forturn);

	// ����Luckiest
	void ClearLuckiest();

	//������Ϣ
	void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);

	// �ж�str1�Ƿ���str2��ͷ ����Ƿ���1 ���Ƿ���0 ������-1
	int is_begin_with(const char * str1, char *str2);

	// ���ַ���str�в���str0�����ƺ�������ݵ�str1����
	int mysubstr(char *str, const char* str0, char str1[]);

	// ȫ��ת���
	void sbc_to_dbc(char *sbc, char *dbc);

	// ����ַ���ǰ�Ŀո�
	void VS_StrLTrim(char *pStr);

private:

	XCritSec    csMapNickName_;
	MapNickName MapNickName_;//��ʱ�ǳ�����

	XCritSec    csMapFortune_;
	MapFortune  MapFortune_;//����ָ��������ÿ��12��������
	int lastday;//����������Ӧ������

	XCritSec    csLuckiest_;
	Mapluckiest Mapluckiest_;//

	XCritSec    csRollDice_;
	DiceVector	DiceVect_;//���Ӷ���

	CQTool QTool;
	CQ_TYPE_QQ QInfo;
	CQ_Type_GroupMember QGroupInfo;

// 	unsigned int topfortune;//������ߵ�����ֵ
// 	int64_t topQQ;//����������ߵ�QQ��

	//ָ�
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