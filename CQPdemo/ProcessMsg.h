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
	// ������Ϣ
	int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg);

	// Ͷ������
	int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg);

	// ���COC6������
	int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// ���COC7������
	int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);

	// Ͷ���������
	unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);

	// ��ȡ�ǳ�
	const char* GetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// �����ǳ�
	int SetNickName(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, char* nick);

	// �����ǳ�
	void ClearNickName();

	// Ͷ������
	int RollFortune(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss);

	// ��ȡ����
	unsigned int GetFortune(int64_t fromQQ);

	// ��������
	void SetFortune(int64_t fromQQ, int fortune);

	// ��������
	void ClearFortune();

	//������Ϣ
	void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);

	// �ж�str1�Ƿ���str2��ͷ ����Ƿ���1 ���Ƿ���0 ������-1
	int is_begin_with(const char * str1, char *str2);

	// ���ַ���str�в���str0�����ƺ�������ݵ�str1����
	int mysubstr(char *str, const char* str0, char str1[]);

	// ȫ��ת���
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

// // ������Ϣ
// int ProcessMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* InputMsg, char* OutputMsg);
// 
// // Ͷ������
// int RollDice(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int dicenum, unsigned int sides, char* msg);
// 
// // ���COC6������
// int RollAttributes(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);
// 
// // ���COC7������
// int RollAttributes7(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, unsigned int attributesnum);
// 
// // Ͷ���������
// unsigned int MultiDiceSum(unsigned int dicenum, unsigned int sides);
// 
// // �ж�str1�Ƿ���str2��ͷ ����Ƿ���1 ���Ƿ���0 ������-1
// int is_begin_with(const char * str1, char *str2);
// 
// // ���ַ���str�в���str0�����ƺ�������ݵ�str1����
// int mysubstr(char *str, const char* str0, char str1[]);
// 
// // ��ȡ�ǳ�
// const char* GetNickName(int ac, int64_t fromQQ);
// 
// //������Ϣ
// void SendMsg(int ac, int64_t fromQQ, int64_t fromGroup, int64_t fromDiscuss, const char* msg);