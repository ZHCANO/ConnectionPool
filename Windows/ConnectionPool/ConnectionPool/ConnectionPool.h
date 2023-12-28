#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include "Connection.h"
#include <fstream>
#include <string>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "public.h"

class ConnectionPool {
public:
	// ��ȡ���Ψһʵ������Ľӿڷ���
	static ConnectionPool* getConnectionPool();

	// �������̣߳��ṩ���ⲿ�Ľӿڣ������ӳ��л�ȡһ�����õĿ�������
	shared_ptr<Connection> getConnection();
private:
	// ˽�л����캯��
	ConnectionPool(); 

	// �������ļ�����������
	bool loadConfigFile();

	// �������߳�
	void produceConnectionTask();

	// ��ʱ���������߳�
	void scannerConnectionTask();

	string ip_; // ip��ַ
	unsigned int port_; // �˿ں�
	string username_; // �û���
	string passward_; // ����
	string dbname_; // ���ݿ���
	int initSize_; // ��ʼ������
	int maxSize_; // ���������
	int maxIdleTime_; // ������ʱ��
	int connectionTimeout_; // ���ӳ�ʱʱ��

	queue<Connection*> connectionQue_; // �洢 mysql ���ӵĶ���
	mutex queueMutex_; // ά�����Ӷ����̰߳�ȫ�Ļ�����
	atomic_int connectionCnt_; // ��¼���������ӵ�������
	condition_variable cv; // �����������������������������̺߳��������̵߳�ͨ��
};
#endif