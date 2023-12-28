#ifndef PUBLIC_H
#define PUBLIC_H
#include <iostream>
using namespace std;

// 文件名:行号 时间戳: 日志信息
#define LOG(str)\
cout << __FILE__ << ":" << __LINE__ << " " << __TIMESTAMP__ << ":" << str <<endl;

#endif