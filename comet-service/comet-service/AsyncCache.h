#pragma once

#include <boost/asio.hpp>

#include <map>
#include <string>
#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

typedef std::pair<std::string, std::string> Field;

class Record {
	std::vector<Field> _fields;
public:
	void add(Field field) {
		_fields.push_back(field);
	}
	bool operator == (const Record& rhs) {
		return _fields == rhs._fields;
	}
};


class Container {
	std::list<std::string> _symbols;
public:
	void addSymbol(std::string symbol) {
		_symbols.push_back(symbol);
	}
};

class AsyncCache
{
public:
	typedef std::function<void (std::string, Record)> callbackFunction;
private:
	// recordCache: pair<path, reecord>
	std::map<std::string, Record> _recordCache;
	mutable std::mutex _recordMutex;
	// directoryCache: pair<path, directory>
	std::map<std::string, std::list<std::string> > _directoryCache;
	// containerCache: pair<path, container>
	std::map<std::string, Container> _containerCache;
	// subscriptions
	std::map<std::string, std::list<callbackFunction> > _subscriptions;
	mutable std::mutex _subscribtionMutex;
	boost::asio::io_service _io_service;
	boost::asio::io_service::work _work;
	std::vector<std::shared_ptr<std::thread> > _threads;
public:
	AsyncCache(std::size_t numThreads = 1);
	~AsyncCache(void);

	void publishRecord(std::string topic, Record record);
	void subscribeOnTopic(std::string topic, callbackFunction callback);
	Record getRecord(std::string topic) const;
private:
	void fireTopicChanged(std::string topic);
	std::vector<callbackFunction> getSubscriptions(std::string topic) const; 
};

static void getRecordAndSubscribe(AsyncCache* ac, std::string topic, AsyncCache::callbackFunction callback) {
	callback(topic, ac->getRecord(topic));
	ac->subscribeOnTopic(topic, callback);
}