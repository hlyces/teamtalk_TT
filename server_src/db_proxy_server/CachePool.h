/*
 * CachePool.h
 *
 *  Created on: 2014年7月24日
 *      Author: ziteng
 *  Modify By ZhangYuanhao 
 *  2015-01-13
 *  Add some redis command
 */

#ifndef CACHEPOOL_H_
#define CACHEPOOL_H_

#include <vector>
#include "../base/util.h"
#include "ThreadPool.h"
#include "hiredis.h"

class CachePool;

class CacheConn {
public:
	CacheConn(CachePool* pCachePool);
	virtual ~CacheConn();

	int Init();
	const char* GetPoolName();

	long del(string key);
	string get(string key);
	string setex(string key, int timeout, string value);
    string set(string key, string& value);
    
    //批量获取
    bool mget(const vector<string>& keys, map<string, string>& ret_value);
    // 判断一个key是否存在
    bool isExists(string &key);

	// Redis hash structure
	long hdel(string key, string field);
	string hget(string key, string field);
	bool hgetAll(string key, map<string, string>& ret_value);
	long hset(string key, string field, string value);

	long hincrBy(string key, string field, long value);
    long incrBy(string key, long value);
	string hmset(string key, map<string, string>& hash);
	bool hmget(string key, list<string>& fields, list<string>& ret_value);
    
    //原子加减1
    long incr(string key);
    long decr(string key);

	// Redis list structure
	long lpush(string key, string value);
	long rpush(string key, string value);
	long llen(string key);
	bool lrange(string key, long start, long end, list<string>& ret_value);

	//sorted set
	long zadd(string key, long score, string member);
	long zcard(string key);
	long zrem(string key, string member);
	bool zrange(string key, long start, long stop, list<string>& ret_value);
	bool zrevrange(string key, long start, long stop, list<string>& ret_value);
	bool zrevrangebyscore(string key, string max, string min, long limit, list<string>& ret_value);
	long zremrangebyrank(string key, long start, long stop);
	long zremrangebyscore(string key, string min, string max);

	//set
	long sadd(string key, string member);
	long scard(string key);
	long sismember(string key, string member);
	bool smembers(string key, list<string>& ret_value);
	long srem(string key, string member);
	long smove(string source, string destination, string member);

private:
	CachePool* 		m_pCachePool;
	redisContext* 	m_pContext;
	uint64_t		m_last_connect_time;
};

class CachePool {
public:
	CachePool(const char* pool_name, const char* server_ip, int server_port, int db_num, int max_conn_cnt, const char* password);
	virtual ~CachePool();

	int Init();

	CacheConn* GetCacheConn();
	void RelCacheConn(CacheConn* pCacheConn);

	const char* GetPoolName() { return m_pool_name.c_str(); }
	const char* GetServerIP() { return m_server_ip.c_str(); }
	int GetServerPort() { return m_server_port; }
	int GetDBNum() { return m_db_num; }
	const char* GetPassword() { return m_password.c_str(); }
	bool IsUsePassword(){ return !m_password.empty();}
private:
	string 		m_pool_name;
	string		m_server_ip;
	int			m_server_port;
	int			m_db_num;
	string		m_password;

	int			m_cur_conn_cnt;
	int 		m_max_conn_cnt;
	list<CacheConn*>	m_free_list;
	CThreadNotify		m_free_notify;
};

class CacheManager {
public:
	virtual ~CacheManager();

	static CacheManager* getInstance(const char* szConfigFile=NULL);

	int Init(const char* szConfigFile);
	CacheConn* GetCacheConn(const char* pool_name);
	void RelCacheConn(CacheConn* pCacheConn);
private:
	CacheManager();

private:
	static CacheManager* 	s_cache_manager;
	map<string, CachePool*>	m_cache_pool_map;
};

#endif /* CACHEPOOL_H_ */
