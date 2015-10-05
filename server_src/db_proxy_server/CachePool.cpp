/*
 * CachePool.cpp
 *
 *  Created on: 2014年7月24日
 *      Author: ziteng
 *  Modify By ZhangYuanhao
 *  2015-01-13
 *  Add some redis command
 * Modify By ZhangYuanhao
 * 2015-01-14
 * Add comment for function
 *
 */

#include "CachePool.h"
#include "ConfigFileReader.h"

#define MIN_CACHE_CONN_CNT	2

CacheManager* CacheManager::s_cache_manager = NULL;

CacheConn::CacheConn(CachePool* pCachePool)
{
	m_pCachePool = pCachePool;
	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::~CacheConn()
{
	if (m_pContext) {
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

/*
 * redis初始化连接和重连操作，类似mysql_ping()
 */
int CacheConn::Init()
{
	if (m_pContext) {
		redisReply* reply = (redisReply *)redisCommand(m_pContext, "SELECT %d", m_pCachePool->GetDBNum());
		if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0)) {
			freeReplyObject(reply); //释放redisCommand执行后返回的的redisReply所占用的内存
			return 0;
		} else {
			redisFree(m_pContext); 
			m_pContext = NULL;
			log("select cache db failed, reconnect!!!");			
		}
	}

	// 4s 尝试重连一次
	uint64_t cur_time = (uint64_t)time(NULL);
	if (cur_time < m_last_connect_time + 4) {
		log("cur_time < m_last_connect_time + 4");
		return 1;
	}

	m_last_connect_time = cur_time;

	// 200ms超时
	struct timeval timeout = {0, 200000};
	//该函数用来连接redis数据库， 两个参数分别是redis数据库的ip和端口，端口号一般为6379,供连接超时限定
	m_pContext = redisConnectWithTimeout(m_pCachePool->GetServerIP(), m_pCachePool->GetServerPort(), timeout);
	if (!m_pContext || m_pContext->err) {
		if (m_pContext) {
			log("redisConnect failed: %s", m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
		} else {
			log("redisConnect failed");
		}

		return 1;
	}

	if(m_pCachePool->IsUsePassword())
	{
		redisReply* reply = (redisReply *)redisCommand(m_pContext, "AUTH %s", m_pCachePool->GetPassword());
		if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0)) {
			freeReplyObject(reply);
			
		} else {
			log("auth cache db failed");
			return 3;
		}
	}

	//该函数用于执行redis数据库中的命令，第一个参数为连接数据库返回的redisContext，剩下的参数为变参，如同C语言中的prinf()函数。此函数的返回值为void*，但是一般会强制转换为redisReply类型，以便做进一步的处理
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SELECT %d", m_pCachePool->GetDBNum());
	if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0)) {
		freeReplyObject(reply); //释放redisCommand执行后返回的的redisReply所占用的内存
		return 0;
	} else {
		log("select cache db failed");
		return 2;
	}
}


const char* CacheConn::GetPoolName()
{
	return m_pCachePool->GetPoolName();
}

long CacheConn::del(string key )
{
	if (Init()) {
		return 0;
	}
	//删除 key ，不存在的key将被忽略
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "DEL %s ", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return 0;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}


string CacheConn::get(string key) //查询StrKey对应的String值
{
	string value;

	if (Init()) {
		return value;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "GET %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext); //释放redisConnect()所产生的连接
		m_pContext = NULL;
		return value;
	}

	if (reply->type == REDIS_REPLY_STRING) {
		value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return value;
}

string CacheConn::setex(string key, int timeout, string value)
{
	string ret_value;

	if (Init()) {
		return ret_value;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SETEX %s %d %s", key.c_str(), timeout, value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::set(string key, string &value)
{
    string ret_value;
    
    if (Init()) {
        return ret_value;
    }
    
    redisReply* reply = (redisReply *)redisCommand(m_pContext, "SET %s %s", key.c_str(), value.c_str());
    if (!reply) {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }
    
    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}

bool CacheConn::mget(const vector<string>& keys, map<string, string>& ret_value)
{
    if(Init())
    {
        return false;
    }
    if(keys.empty())
    {
        return false;
    }
    
    string strKey;
    bool bFirst = true;
    for (vector<string>::const_iterator it=keys.begin(); it!=keys.end(); ++it) {
        if(bFirst)
        {
            bFirst = false;
            strKey = *it;
        }
        else
        {
            strKey += " " + *it;
        }
    }
    
    if(strKey.empty())
    {
        return false;
    }
    strKey = "MGET " + strKey;
    redisReply* reply = (redisReply*) redisCommand(m_pContext, strKey.c_str());
    if (!reply) {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return false;
    }
    if(reply->type == REDIS_REPLY_ARRAY)
    {
        for(size_t i=0; i<reply->elements; ++i)
        {
            redisReply* child_reply = reply->element[i];
            if (child_reply->type == REDIS_REPLY_STRING) {
                ret_value[keys[i]] = child_reply->str;
            }
        }
    }
    freeReplyObject(reply);
    return true;
}

bool CacheConn::isExists(string &key)
{
    if (Init()) {
        return false;
    }
    
    redisReply* reply = (redisReply*) redisCommand(m_pContext, "EXISTS %s", key.c_str());
    if(!reply)
    {
        log("redisCommand failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        return false;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    if(0 == ret_value)
    {
        return false;
    }
    else
    {
        return true;
    }
}
long CacheConn::hdel(string key, string field)
{
	if (Init()) {
		return 0;
	}
	//删除哈希表 key 中的一个或多个指定域，不存在的域将被忽略
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HDEL %s %s", key.c_str(), field.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return 0;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::hget(string key, string field)
{
	string ret_value;
	if (Init()) {
		return ret_value;
	}
	//返回哈希表 key 中给定域 field 的值
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HGET %s %s", key.c_str(), field.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	if (reply->type == REDIS_REPLY_STRING) {
		ret_value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hgetAll(string key, map<string, string>& ret_value)
{
	if (Init()) {
		return false;
	}
	//在返回值里，紧跟每个域名(field name)之后是域的值(value)，所以返回值的长度是哈希表大小的两倍
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HGETALL %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if ( (reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0) ) {
		for (size_t i = 0; i < reply->elements; i += 2) {
			redisReply* field_reply = reply->element[i];
			redisReply* value_reply = reply->element[i + 1];

			string field(field_reply->str, field_reply->len);
			string value(value_reply->str, value_reply->len);
			ret_value.insert(make_pair(field, value));
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::hset(string key, string field, string value)
{
	if (Init()) {
		return -1;
	}
	//将哈希表 key 中的域 field 的值设为 value 。如果 key 不存在，一个新的哈希表被创建并进行 HSET 操作。如果域 field 已经存在于哈希表中，旧值将被覆盖
	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::hincrBy(string key, string field, long value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "HINCRBY %s %s %ld", key.c_str(), field.c_str(), value);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::incrBy(string key, long value)
{
    if(Init())
    {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCRBY %s %ld", key.c_str(), value);
    if(!reply)
    {
        log("redis Command failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::hmset(string key, map<string, string>& hash)
{
	string ret_value;

	if (Init()) {
		return ret_value;
	}

	int argc = hash.size() * 2 + 2;
	const char** argv = new const char* [argc];
	if (!argv) {
		return ret_value;
	}

	argv[0] = "HMSET";
	argv[1] = key.c_str();
	int i = 2;
	for (map<string, string>::iterator it = hash.begin(); it != hash.end(); it++) {
		argv[i++] = it->first.c_str();
		argv[i++] = it->second.c_str();
	}

	redisReply* reply = (redisReply *)redisCommandArgv(m_pContext, argc, argv, NULL);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		delete [] argv;

		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);

	delete [] argv;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hmget(string key, list<string>& fields, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	int argc = fields.size() + 2;
	const char** argv = new const char* [argc];
	if (!argv) {
		return false;
	}

	argv[0] = "HMGET";
	argv[1] = key.c_str();
	int i = 2;
	for (list<string>::iterator it = fields.begin(); it != fields.end(); it++) {
		argv[i++] = it->c_str();
	}

	redisReply* reply = (redisReply *)redisCommandArgv(m_pContext, argc, (const char**)argv, NULL);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		delete [] argv;

		redisFree(m_pContext);
		m_pContext = NULL;

		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	delete [] argv;
	freeReplyObject(reply);
	return true;
}

long CacheConn::incr(string key)
{
    if(Init())
    {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(m_pContext, "INCR %s", key.c_str());
    if(!reply)
    {
        log("redis Command failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::decr(string key)
{
    if(Init())
    {
        return -1;
    }
    
    redisReply* reply = (redisReply*)redisCommand(m_pContext, "DECR %s", key.c_str());
    if(!reply)
    {
        log("redis Command failed:%s", m_pContext->errstr);
        redisFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::lpush(string key, string value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LPUSH %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::rpush(string key, string value)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "RPUSH %s %s", key.c_str(), value.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::llen(string key)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LLEN %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::lrange(string key, long start, long end, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "LRANGE %s %d %d", key.c_str(), start, end);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::zadd(string key, long score, string member)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZADD %s %d %s", key.c_str(), score, member.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::zcard(string key)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZCARD %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::zrem(string key, string member)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZREM %s %s", key.c_str(), member.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::zrange(string key, long start, long stop, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZRANGE %s %d %d", key.c_str(), start, stop);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}


bool CacheConn::zrevrange(string key, long start, long stop, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZREVRANGE %s %d %d", key.c_str(), start, stop);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}

bool CacheConn::zrevrangebyscore(string key, string max, string min, long limit, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZREVRANGEBYSCORE %s %s %s LIMIT 0 %d", key.c_str(), max.c_str(), min.c_str(), limit);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}


long CacheConn::zremrangebyrank(string key, long start, long stop)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZREMRANGEBYRANK %s %d %d", key.c_str(), start, stop);
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::zremrangebyscore(string key, string min, string max)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "ZREMRANGEBYSCORE %s %s %s", key.c_str(), min.c_str(), max.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::sadd(string key, string member)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SADD %s %s", key.c_str(), member.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::scard(string key)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SCARD %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::sismember(string key, string member)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SISMEMBER %s %s", key.c_str(), member.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::smembers(string key, list<string>& ret_value)
{
	if (Init()) {
		return false;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SMEMBERS %s", key.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY) {
		for (size_t i = 0; i < reply->elements; i++) {
			redisReply* value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::srem(string key, string member)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SREM %s %s", key.c_str(), member.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::smove(string source, string destination, string member)
{
	if (Init()) {
		return -1;
	}

	redisReply* reply = (redisReply *)redisCommand(m_pContext, "SMOVE %s %s %s", source.c_str(), destination.c_str(), member.c_str());
	if (!reply) {
		log("redisCommand failed:%s", m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}



///////////////
CachePool::CachePool(const char* pool_name, const char* server_ip, int server_port, int db_num, int max_conn_cnt, const char* password)
{
	m_pool_name = pool_name;
	m_server_ip = server_ip;
	m_server_port = server_port;
	m_db_num = db_num;
	m_max_conn_cnt = max_conn_cnt;
	m_cur_conn_cnt = MIN_CACHE_CONN_CNT;
	m_password = password==NULL?"":password;
}

CachePool::~CachePool()
{
	m_free_notify.Lock();
	for (list<CacheConn*>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++) {
		CacheConn* pConn = *it;
		delete pConn;
	}

	m_free_list.clear();
	m_cur_conn_cnt = 0;
	m_free_notify.Unlock();
}

int CachePool::Init()
{
	for (int i = 0; i < m_cur_conn_cnt; i++) {
		CacheConn* pConn = new CacheConn(this);
		if (pConn->Init()) {
			delete pConn;
			return 1;
		}

		m_free_list.push_back(pConn);
	}

	log("cache pool: %s, list size: %u", m_pool_name.c_str(), m_free_list.size());
	return 0;
}

CacheConn* CachePool::GetCacheConn()
{
	m_free_notify.Lock();

	while (m_free_list.empty()) {
		if (m_cur_conn_cnt >= m_max_conn_cnt) {
			m_free_notify.Wait();
		} else {
			CacheConn* pCacheConn = new CacheConn(this);
			int ret = pCacheConn->Init();
			if (ret) {
				log("Init CacheConn failed");
				delete pCacheConn;
				m_free_notify.Unlock();
				return NULL;
			} else {
				m_free_list.push_back(pCacheConn);
				m_cur_conn_cnt++;
				log("new cache connection: %s, conn_cnt: %d", m_pool_name.c_str(), m_cur_conn_cnt);
			}
		}
	}

	CacheConn* pConn = m_free_list.front();
	m_free_list.pop_front();

	m_free_notify.Unlock();

	return pConn;
}

void CachePool::RelCacheConn(CacheConn* pCacheConn)
{
	m_free_notify.Lock();

	list<CacheConn*>::iterator it = m_free_list.begin();
	for (; it != m_free_list.end(); it++) {
		if (*it == pCacheConn) {
			break;
		}
	}

	if (it == m_free_list.end()) {
		m_free_list.push_back(pCacheConn);
	}

	m_free_notify.Signal();
	m_free_notify.Unlock();
}

///////////
CacheManager::CacheManager()
{

}

CacheManager::~CacheManager()
{

}

CacheManager* CacheManager::getInstance(const char* szConfigFile)
{
	if (!s_cache_manager) {
		s_cache_manager = new CacheManager();
		if (s_cache_manager->Init(szConfigFile)) {
			delete s_cache_manager;
			s_cache_manager = NULL;
		}
	}

	return s_cache_manager;
}

int CacheManager::Init(const char* szConfigFile)
{
	CConfigFileReader config_file(szConfigFile);

	char* cache_instances = config_file.GetConfigName("CacheInstances");
	if (!cache_instances) {
		log("not configure CacheIntance");
		return 1;
	}

	char host[64];
	char port[64];
	char password[64];
	char db[64];
    char maxconncnt[64];
	CStrExplode instances_name(cache_instances, ',');
	for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++) {
		char* pool_name = instances_name.GetItem(i);
		//printf("%s", pool_name);
		snprintf(host, 64, "%s_host", pool_name);
		snprintf(port, 64, "%s_port", pool_name);
		snprintf(password, 64, "%s_password", pool_name);
		snprintf(db, 64, "%s_db", pool_name);
        snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);

		char* cache_host = config_file.GetConfigName(host);
		char* str_cache_port = config_file.GetConfigName(port);
		char* str_cache_password = config_file.GetConfigName(password);
		char* str_cache_db = config_file.GetConfigName(db);
        char* str_max_conn_cnt = config_file.GetConfigName(maxconncnt);
		if (!cache_host || !str_cache_port || !str_cache_db || !str_max_conn_cnt) {
			log("not configure cache instance: %s", pool_name);
			return 2;
		}

		CachePool* pCachePool = new CachePool(pool_name, cache_host, atoi(str_cache_port),
				atoi(str_cache_db), atoi(str_max_conn_cnt), str_cache_password);
		if (pCachePool->Init()) {
			log("Init cache pool failed");
			return 3;
		}

		m_cache_pool_map.insert(make_pair(pool_name, pCachePool));
	}

	return 0;
}

CacheConn* CacheManager::GetCacheConn(const char* pool_name)
{
	map<string, CachePool*>::iterator it = m_cache_pool_map.find(pool_name);
	if (it != m_cache_pool_map.end()) {
		return it->second->GetCacheConn();
	} else {
		return NULL;
	}
}

void CacheManager::RelCacheConn(CacheConn* pCacheConn)
{
	if (!pCacheConn) {
		return;
	}

	map<string, CachePool*>::iterator it = m_cache_pool_map.find(pCacheConn->GetPoolName());
	if (it != m_cache_pool_map.end()) {
		return it->second->RelCacheConn(pCacheConn);
	}
}
