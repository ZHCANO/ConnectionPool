#include "Connection.h"
/*
* ʵ��MySQL���ݿ����ɾ�Ĳ����
*/
// ��ʼ�����ݿ�����
Connection::Connection() {
	conn_ = mysql_init(nullptr);
}

// �ͷ����ݿ�����
Connection::~Connection() {
	if (conn_ != nullptr)
		mysql_close(conn_);
}

// �������ݿ�
bool Connection::connect(string ip, unsigned short port, string user, string password, string dbname) {
	MYSQL* p = mysql_real_connect(conn_, ip.c_str(), user.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	// �������ʧ��ԭ��
	if (p == nullptr) {
		cerr << mysql_error(conn_) << " - " << mysql_error(conn_) << std::endl;
	}
	return p != nullptr;
}

// ���²���insert/delete/update
bool Connection::update(string sql) {
	if (mysql_query(conn_, sql.c_str())) {
		LOG("Update Failed:" + sql);
		return false;
	}
	return true;
}

// ��ѯ����select
MYSQL_RES* Connection::query(string sql) {
	if (mysql_query(conn_, sql.c_str())){
		LOG("Query Failed:" + sql);
		return nullptr;
	}
	return mysql_use_result(conn_);
}