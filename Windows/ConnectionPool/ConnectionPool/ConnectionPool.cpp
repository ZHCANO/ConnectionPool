#include "ConnectionPool.h"
// ��ȡ���Ψһʵ������Ľӿڷ���
ConnectionPool* ConnectionPool::getConnectionPool() {
	static ConnectionPool pool; // �������Զ�lock��unlock
	return &pool;
}
// �������ļ�����������
bool ConnectionPool::loadConfigFile() {
	// ��ֻ����ʽ��mysql.ini
	ifstream ifs;
	ifs.open("mysql.ini", ios::in);
	
	// �ж������ļ��Ƿ����
	if (!ifs.is_open()) {
		LOG("mysql.ini file is not exist!");
		return false;
	}

	// ѭ�����ж�ȡ��¼����=��\n�ָ���ȡkey��value
	string line;
	while (std::getline(ifs, line)) {
		string str;

		// ȥ���ո��'\r'
		for (int i = 0; i < line.size(); i++) {
			if (line[i] == ' ' || line[i] == '\r') continue;
			str += line[i];
		}

		// �ж��Ƿ�Ϊ����
		if (str.empty()) {
			continue;
		}

		int idx = str.find('=', 0);
		if (idx == -1) { // ��Ч��������
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

// ���ӳصĹ���
ConnectionPool::ConnectionPool() {
	if (!loadConfigFile()) {
		return;
	}

	// ������ʼ����������
	for (int i = 0; i < initSize_; i++) {
		Connection* p = new Connection();
		p->connect(ip_, port_, username_, passward_, dbname_);
		p->refreshAlivetime();
		connectionQue_.push(p);
		connectionCnt_++;
	}

	// �������ӳص��������߳�
	thread produce(bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	// �������ӳض�ʱ���������߳�
	thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

// �������߳�
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(queueMutex_);

		// ���ӳض��в��գ��������߳̽���ȴ�״̬
		while (!connectionQue_.empty())
			cv.wait(lock);

		// ��������û�дﵽ���ޣ����������µ�����
		if (connectionCnt_ < maxSize_) {
			Connection* p = new Connection();
			p->connect(ip_, port_, username_, passward_, dbname_);
			p->refreshAlivetime();
			connectionQue_.push(p);
			connectionCnt_++;
		}

		// ֪ͨ�������̻߳�ȡ����
		cv.notify_all();
	}
}
// �������̣߳��ṩ���ⲿ�Ľӿڣ������ӳ��л�ȡһ�����õĿ�������
shared_ptr<Connection> ConnectionPool::getConnection() {
	unique_lock<mutex> lock(queueMutex_);

	// ���ӳض���Ϊ�գ��������߳̽���ȴ�״̬
	while (connectionQue_.empty()) {
		cv_status status = cv.wait_for(lock, chrono::milliseconds(connectionTimeout_)); // �̳߳������ӻ���/��ʱ����
		if (status == cv_status::timeout && connectionQue_.empty()) { // ��ʱ����
			LOG("��ȡ�������ӳ�ʱ...��ȡ����ʧ�ܣ�");
			return nullptr;
		}
	}

	/*
	* shared_ptr����ָ������ʱ�����connection����Դdelete���൱�ڵ���connection������������connection�ͱ�close��
	* ������Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ����connectionֱ�ӹ黹��������
	*/

	// �����ӳض���ȡ��һ������
	shared_ptr<Connection> sp(connectionQue_.front(),
		[&](Connection* pcon) {
			// �����ڷ�����Ӧ���߳��е��õ�
			unique_lock<mutex> lock(queueMutex_);
			pcon->refreshAlivetime();
			connectionQue_.push(pcon);
		});
	connectionQue_.pop();

	// ֪ͨ�������߳�����
	if (connectionQue_.empty()) {
		cv.notify_all();
	}
	return sp;
}

// ��ʱ���������߳�
void ConnectionPool::scannerConnectionTask() {
	for (;;) {
		// ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(maxIdleTime_));

		// ɨ���������У��ͷŶ�������
		unique_lock<mutex> lock(queueMutex_);
		while (!connectionQue_.empty()) {
			Connection* p = connectionQue_.front();
			if (connectionCnt_ > initSize_ && p->getAlivetime() >= maxIdleTime_ * 1000) {
				connectionQue_.pop();
				delete p;
				connectionCnt_--;
			}
			else { // ��ͷ����û�г���������ʱ�䣬�������ӿ϶�Ҳû��
				break;
			}
		}
	}
}