## ConnectionPool

### 连接池功能

- 初始连接量（initSize）
  - 连接池事先和MySQLServer创建initSize个连接
  - 当应用发起MySQL访问时，直接从连接池中获取一个可用的连接，使用完成后归还连接池
- 最大连接量（maxSize）
  - 当并发访问MySQL请求增多至超过初始连接量时，连接池会根据数量请求去创建更多的连接。
  - 但是连接的数量上限是maxSize，因为连接占用socket资源，如果连接池占用过多的socket资源，那么服务器就不能接收太多的客户端请求了。（连接池和服务器部署在同一台主机上）
- 最大空闲时间（maxIdleTime）
  - 如果新增加的连接在最大空闲时间内都没有被使用，那么这些连接就会被回收
- 连接超时时间（connectionTimeout）
  - 如果在connectionTimeout内都没有空闲的连接可以使用，那么连接就会失败，无法访问数据库

### 环境要求

* C++ 11
* MySQL

### 项目启动

**创建user表：**

```sql
create databases pool;
use pool;
CREATE TABLE user(id INT(11) PRIMARY KEY NOT NULL AUTO_INCREMENT,
    name VARCHAR(50) DEFAULT NULL,
    age INT(11) DEFAULT NULL,
    sex ENUM('male','female') DEFAULT NULL)ENGINE=INNODB DEFAULT CHARSET=utf8;
```

**配置文件**

打开mysql.ini配置数据库信息

```
# 数据库的配置文件（注意：等号左右不要有空格）
ip=127.0.0.1
port=3306
username=root
passward=123456
dbname=pool
initSize=10
maxSize=1024
# 最大空闲时间，默认单位：秒
maxIdleTime=60
# 连接超时时间，默认单位：毫秒
connectionTimeout=100
```

**windows:**

由于需要调用MySQL API来完成数据库的连接、查询和更新等操作，需要在vs2019中导入MySQL的头文件、库文件。

1. VS2019选择X64，因为安装的MySQL是64位的。
2. 右键项目 - 属性 - C/C++ - 常规 - 附加包含目录，填写mysql.h头文件的路径（例如：D:\developer_tools\MySQL\Files\MySQL Server 5.7\include）。注意：先在源文件下创建一个.cpp文件后，下图中C/C++的选项才会出现。
3. 右键项目 - 属性 - 链接器 - 常规 - 附加库目录，填写libmysql.lib的路径（D:\developer_tools\MySQL\Files\MySQL Server 5.7\lib）
4. 右键项目 - 属性 - 链接器 - 输入 - 附加依赖项，填写libmysql.lib库的名字（libmysql.lib）（静态库）
5. 把libmysql.dll动态链接库（Linux下后缀名是.so库）放在工程目录下（动态库）

按F5运行

**Linux:**

将源文件和头文件放在一个文件夹下，如下：
```
.
├── Connection.cpp
├── Connection.h
├── ConnectionPool.cpp
├── ConnectionPool.h
├── main.cpp
├── mysql.ini
└── public.h
```

在终端执行g++指令：

```
# 进入ConnectionPool目录
cd ConnectionPool
# g++ -o 可执行文件名 源文件1/2/3... 
# 指定头文件的位置（-I）、库文件的目录（-L）、库名（-l）
# mysql头文件路径（-I/usr/include/mysql） mysql库文件路径（-L/usr/lib/x86_64-linux-gnu） 库名（-lmysqlclient）
g++ -o ConnectionPool Connection.cpp ConnectionPool.cpp main.cpp -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread
```
执行：

```
./ConnectionPool
```

### Linux动态库

生成.o的二进制目标文件，得到与位置无关的代码：

```
# -fPIC 选项表示生成位置无关
g++ -c -fPIC Connection.cpp ConnectionPool.cpp
```

生成动态库：

```
g++ -shared Connection.o ConnectionPool.o -o libconpool.so
```

配置动态载入器加载路径：

```
// 修改/etc/ld.so.conf 
sudo vim /etc/ld.so.conf
// 添加路径
/home/ubuntu/workspace/lib
// 使其生效
sudo ldconfig
```

编译链接程序：

```
g++ -o conn main.cpp -L/home/ubuntu/workspace/lib -lconpool -pthread -I/usr/include/mysql -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread
```

运行（可以通过ldd （list dynamic dependencies）命令检查动态库依赖关系）：

```
./conn
```
