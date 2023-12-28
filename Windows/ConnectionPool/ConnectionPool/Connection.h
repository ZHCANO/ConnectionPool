#ifndef CONNECTION_H
#define CONNECTION_H
#include <mysql.h>
#include <string>
#include <ctime>
#include "public.h"
using namespace std;
class Connection {
public:
	// ��ʼ�����ݿ�����
	Connection();

	// �ͷ����ݿ�����
	~Connection();

	// �������ݿ�
	bool connect(string ip, unsigned short port, string user, string passward, string dbname);

	// ���²���insert/delete/update
	bool update(string sql);

	// ��ѯ����select
	MYSQL_RES* query(string sql);

	// ˢ��һ�����ӵ���ʼ����ʱ���
	void refreshAlivetime() { alivetime_ = clock(); } // ��λ������

	// ���ش��ʱ�䣨�ڶ����д���ʱ�䣩
	clock_t getAlivetime() { return clock() - alivetime_; }
private:
	MYSQL* conn_; // ��ʾ��MySQL Server��һ������
	clock_t alivetime_; // ��¼�������״̬�����ʼ���ʱ��
};
#endif
