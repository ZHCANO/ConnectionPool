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
	// 获取类的唯一实例对象的接口方法
	static ConnectionPool* getConnectionPool();

	// 消费者线程，提供给外部的接口，从连接池中获取一个可用的空闲连接
	shared_ptr<Connection> getConnection();
private:
	// 私有化构造函数
	ConnectionPool(); 

	// 从配置文件加载配置项
	bool loadConfigFile();

	// 生产者线程
	void produceConnectionTask();

	// 定时清理连接线程
	void scannerConnectionTask();

	string ip_; // ip地址
	unsigned int port_; // 端口号
	string username_; // 用户名
	string passward_; // 密码
	string dbname_; // 数据库名
	int initSize_; // 初始连接量
	int maxSize_; // 最大连接量
	int maxIdleTime_; // 最大空闲时间
	int connectionTimeout_; // 连接超时时间

	queue<Connection*> connectionQue_; // 存储 mysql 连接的队列
	mutex queueMutex_; // 维护连接队列线程安全的互斥锁
	atomic_int connectionCnt_; // 记录创建的连接的总数量
	condition_variable cv; // 设置条件变量，用于连接生产者线程和消费者线程的通信
};
#endif