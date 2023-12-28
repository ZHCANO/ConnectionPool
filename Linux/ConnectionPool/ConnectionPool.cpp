#include "ConnectionPool.h"
// 获取类的唯一实例对象的接口方法
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool; // 编译器自动lock和unlock
	return &pool;
}
// 从配置文件加载配置项
bool ConnectionPool::loadConfigFile() {
	// 以只读方式打开mysql.ini
	ifstream ifs;
	ifs.open("mysql.ini", ios::in);
	
	// 判断配置文件是否存在
	if (!ifs.is_open()) {
		LOG("mysql.ini file is not exist!");
		return false;
	}

	// 循环按行读取记录，按=和\n分割提取key和value
	string line;
	while (std::getline(ifs, line)) {
		string str;

		// 去除空格和'\r'
		for(int i=0; i < line.size(); i++) {
			if(line[i]==' ' || line[i]=='\r') continue;
			str += line[i];
		}

		// 判断是否为空行
        if (str.empty()) {
            continue;
        }

		int idx = str.find('=', 0);
		if (idx == -1) { // 无效的配置项
			continue;
		}
		int endIdx = str.find('\n', idx);
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endIdx - idx - 1);
		if (key == "ip") {
			ip_ = value;
		}
		else if (key == "port") {
			port_ = stoi(value);
		}
		else if (key == "username") {
			username_ = value;
		}
		else if (key == "passward") {
			passward_ = value;
		}
		else if (key == "dbname") {
			dbname_ = value;
		}
		else if (key == "initSize") {
			initSize_ = stoi(value);
		}
		else if (key == "maxSize") {
			maxSize_ = stoi(value);
		}
		else if (key == "maxIdleTime") {
			maxIdleTime_ = stoi(value);
		}
		else if (key == "connectionTimeout") {
			connectionTimeout_ = stoi(value);
		}
	}
	ifs.close();

	return true;
}

// 连接池的构造
ConnectionPool::ConnectionPool() {
	if (!loadConfigFile()) {
		return;
	}

	// 创建初始数量的连接
	for (int i = 0; i < initSize_; i++) {
		Connection* p = new Connection();
		p->connect(ip_, port_, username_, passward_, dbname_);
		p->refreshAlivetime();
		connectionQue_.push(p);
		connectionCnt_++;
	}

	// 启动连接池的生产者线程
	thread produce(bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	// 启动连接池定时清理连接线程
	thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

// 生产者线程
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(queueMutex_);

		// 连接池队列不空，生产者线程进入等待状态
		while (!connectionQue_.empty())
			cv.wait(lock);

		// 连接数量没有达到上限，继续生产新的连接
		if (connectionCnt_ < maxSize_) {
			Connection* p = new Connection();
			p->connect(ip_, port_, username_, passward_, dbname_);
			p->refreshAlivetime();
			connectionQue_.push(p);
			connectionCnt_++;
		}

		// 通知消费者线程获取连接
		cv.notify_all();
	}
}
// 消费者线程，提供给外部的接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(queueMutex_);

	// 连接池队列为空，消费者线程进入等待状态
	while (connectionQue_.empty()) {
		cv_status status = cv.wait_for(lock, chrono::milliseconds(connectionTimeout_)); // 线程池有连接唤醒/超时唤醒
		if (status == cv_status::timeout && connectionQue_.empty()) { // 超时唤醒
			LOG("获取空闲连接超时...获取连接失败！");
			return nullptr;
		}
	}

	/*
	* shared_ptr智能指针析构时，会把connection的资源delete，相当于调用connection的析构函数，connection就被close了
	* 这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到队列中
	*/

	// 从连接池队列取出一个连接
	shared_ptr<Connection> sp(connectionQue_.front(),
		[&](Connection* pcon) {
			// 这是在服务器应用线程中调用的
			unique_lock<mutex> lock(queueMutex_);
			pcon->refreshAlivetime();
			connectionQue_.push(pcon);
		});
	connectionQue_.pop();

	// 通知生产者线程生产
	if (connectionQue_.empty()) {
		cv.notify_all();
	}
	return sp;
}

// 定时清理连接线程
void ConnectionPool::scannerConnectionTask() {
	for (;;) {
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(maxIdleTime_));

		// 扫描整个队列，释放多余连接
		unique_lock<mutex> lock(queueMutex_);
		while (!connectionQue_.empty()) {
			Connection* p = connectionQue_.front();
			if (connectionCnt_ > initSize_ && p->getAlivetime() >= maxIdleTime_ * 1000) {
				connectionQue_.pop();
				delete p;
				connectionCnt_--;
			}
			else { // 队头连接没有超过最大空闲时间，其他连接肯定也没有
				break;
			}
		}
	}
}