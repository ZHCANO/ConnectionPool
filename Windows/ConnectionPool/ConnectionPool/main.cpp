#include "Connection.h"
#include "ConnectionPool.h"
#include <iostream>
using namespace std;

void connFunc() {
	string sql = "insert into user(name,age,sex) values('wang', 22, 'male')";
	for (int i = 0; i < 250; ++i){
		Connection conn;
		conn.connect("127.0.0.1", 3306, "root", "123456", "pool");
		conn.update(sql);
	}
}

void connPool() {
	ConnectionPool *cp = ConnectionPool::getConnectionPool();
	string sql = "insert into user(name,age,sex) values('wang', 22, 'male')";
	for(int i = 0; i < 250; ++i) {
		shared_ptr<Connection> sp = cp->getConnection();
		sp->update(sql);
	}
}

int main() {
	// 未使用数据库连接池
	// clock_t begin = clock();

	// string sql = "insert into user(name,age,sex) values('li', 21, 'female')";
	// for(int i=0;i<1000;i++) {
	// 	Connection conn;
	// 	conn.connect("127.0.0.1", 3306, "root", "123456", "pool");
	// 	conn.update(sql);
	// }

	// clock_t end = clock();

	// cout << "未使用数据库连接池的花费时间：" << ((double) (end - begin)) / CLOCKS_PER_SEC << endl;

	// 数据库连接池使用示例
	// clock_t begin = clock();

	// ConnectionPool* cp = ConnectionPool::getConnectionPool();
	// string sql = "insert into user(name,age,sex) values('wang', 22, 'male')";
	// for (int i = 0; i < 10000; i++) {
	// 	shared_ptr<Connection> sp = cp->getConnection();
	// 	sp->update(sql);
	// }

	// clock_t end = clock();

	// cout << "未使用数据库连接池的花费时间：" << ((double) (end - begin)) / CLOCKS_PER_SEC << endl;


	// 多线程
	clock_t begin = clock();

	thread t1(connPool);
	thread t2(connPool);
	thread t3(connPool);
	thread t4(connPool);

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	clock_t end = clock();

	cout << "使用数据库连接池的花费时间：" << ((double) (end - begin)) / CLOCKS_PER_SEC << endl;

	return 0;
}
