#include "Connection.h"
/*
* 实现MySQL数据库的增删改查操作
*/
// 初始化数据库连接
Connection::Connection() {
	conn_ = mysql_init(nullptr);
}

// 释放数据库连接
Connection::~Connection() {
	if (conn_ != nullptr)
		mysql_close(conn_);
}

// 连接数据库
bool Connection::connect(string ip, unsigned short port, string user, string password, string dbname) {
	MYSQL* p = mysql_real_connect(conn_, ip.c_str(), user.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	// 输出连接失败原因
	if (p == nullptr) {
		cerr << mysql_error(conn_) << " - " << mysql_error(conn_) << std::endl;
	}
	return p != nullptr;
}

// 更新操作insert/delete/update
bool Connection::update(string sql) {
	if (mysql_query(conn_, sql.c_str())) {
		LOG("Update Failed:" + sql);
		return false;
	}
	return true;
}

// 查询操作select
MYSQL_RES* Connection::query(string sql) {
	if (mysql_query(conn_, sql.c_str())){
		LOG("Query Failed:" + sql);
		return nullptr;
	}
	return mysql_use_result(conn_);
}