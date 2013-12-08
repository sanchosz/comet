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

class AsyncCache: std::enable_shared_from_this<AsyncCache>
{
public:
	typedef std::function<void (std::string, Record)> callbackFunction;
	struct Handler {
		std::string _topic;
		callbackFunction _callback;
		AsyncCache* _cache;
		unsigned int _id;
		Handler(AsyncCache* cache, std::string topic, callbackFunction callback, unsigned int id) : 
			_topic(topic), 
			_callback(callback),  
			_cache(cache),
			_id(id)
		{}
		void unsubscribe() {
			//std::shared_ptr<AsyncCache> shared_cache(_cache.lock());
			//if (shared_cache) shared_cache->unsubscribe(_topic, _id);
			_cache->unsubscribe(_topic, _id);
		}
	};
private:
	// recordCache: pair<path, reecord>
	std::map<std::string, Record> _recordCache;
	// directoryCache: pair<path, directory>
	std::map<std::string, std::list<std::string> > _directoryCache;
	std::map<std::string, Container> _containerCache;
	// subscriptions
	std::map<std::string, std::list<std::pair<callbackFunction, int>>> _subscriptions;
	mutable boost::asio::io_service _io_service;
	boost::asio::io_service::work _work;
	std::vector<std::shared_ptr<std::thread> > _threads;
	unsigned int handlerId;
public:
	AsyncCache(std::size_t numThreads = 1);
	~AsyncCache(void);

	void publishRecord(std::string topic, Record record);
	Handler subscribeOnTopic(std::string topic, callbackFunction callback);
	void unsubscribe(std::string topic, unsigned int id);
	Record getRecord(std::string topic) const;
	void getRecord(std::string topic, std::function<void (Record)> getter) const;
private:
	void fireTopicChanged(std::string topic);
	std::vector<std::pair<callbackFunction,int>> getSubscriptions(std::string topic) const; 
	unsigned int getNextHandlerId();
};

static void getRecordAndSubscribe(AsyncCache* ac, std::string topic, AsyncCache::callbackFunction callback) {
	callback(topic, ac->getRecord(topic));
	ac->subscribeOnTopic(topic, callback);
}