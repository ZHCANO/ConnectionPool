#ifndef CONNECTION_H
#define CONNECTION_H
#include <mysql.h>
#include <string>
#include <ctime>
#include "public.h"
using namespace std;
class Connection {
public:
	// 初始化数据库连接
	Connection();

	// 释放数据库连接
	~Connection();

	// 连接数据库
	bool connect(string ip, unsigned short port, string user, string passward, string dbname);

	// 更新操作insert/delete/update
	bool update(string sql);

	// 查询操作select
	MYSQL_RES* query(string sql);

	// 刷新一下连接的起始空闲时间点
	void refreshAlivetime() { alivetime_ = clock(); } // 单位：毫秒

	// 返回存活时间（在队列中待的时间）
	clock_t getAlivetime() { return clock() - alivetime_; }
private:
	MYSQL* conn_; // 表示和MySQL Server的一条连接
	clock_t alivetime_; // 记录进入空闲状态后的起始存活时间
};
#endif
