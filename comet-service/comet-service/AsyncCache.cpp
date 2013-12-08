#include "stdafx.h"

#include <boost\asio.hpp>
#include <boost\thread.hpp>

#include <functional>
#include <algorithm>
#include <future>

#include "AsyncCache.h"

template<typename T>
struct Lazy {
private:
	bool isEvaluated;
	T value;
	std::function<T()> evaluationFunction;
public:
	Lazy(std::function<T()> evaluation):isEvaluated(false), evaluationFunction(evaluation) {}
	T& operator() () { 
		return isEvaluated 
			? value 
			: (value = evaluationFunction());
	}
};



AsyncCache::AsyncCache(std::size_t numThreads)
	:_work(_io_service),
	handlerId(0)
{	
	_threads.reserve(numThreads);
	for (std::size_t i = 0; i < numThreads; ++i)
	{
		std::shared_ptr<std::thread> thread(new std::thread(
			boost::bind(&boost::asio::io_service::run, &_io_service)));
		_threads.push_back(thread);
	}
}

AsyncCache::~AsyncCache(void) {
	_io_service.stop();
	for (std::size_t i = 0; i < _threads.size(); ++i)
		_threads[i]->join();
}

void AsyncCache::publishRecord(std::string topic, Record record) {
	_io_service.post([this, topic, record] () {
		_recordCache[topic] = record;
		fireTopicChanged(topic);
	});	
}

unsigned int AsyncCache::getNextHandlerId() {
	return handlerId++;
}

AsyncCache::Handler AsyncCache::subscribeOnTopic(std::string topic, callbackFunction callback) {
	auto c  = std::make_pair(callback, getNextHandlerId());
	_io_service.post([topic, c, this] () { 
		if (_subscriptions.find(topic) == _subscriptions.end())
			_subscriptions[topic] = std::list<std::pair<callbackFunction, int>>(1,  c);
		else 
			_subscriptions[topic].push_back(c);
	});
	auto h = Handler(this, topic, c.first, c.second);
	return h;
}

void AsyncCache::unsubscribe(std::string topic, unsigned int id) {
	_io_service.post([topic, id, this] () { 
		if (_subscriptions.find(topic) != _subscriptions.end()) {
			_subscriptions[topic].remove_if([id] (std::pair<callbackFunction, int> p) { 
				return p.second == id;
			});
		}
	});
}

void AsyncCache::fireTopicChanged(std::string topic) {
	Lazy<Record> record ([topic, this](){ return getRecord(topic);});
	auto callbacks = getSubscriptions(topic);

	for(auto cb = callbacks.begin(); cb != callbacks.end(); ++cb)
		_io_service.post(std::bind(cb->first, topic, record()));
}

std::vector<std::pair<AsyncCache::callbackFunction,int>> AsyncCache::getSubscriptions(std::string topic) const {
	auto sub = _subscriptions.find(topic);
	auto callbacks = sub != _subscriptions.end()
		? std::vector<std::pair<AsyncCache::callbackFunction,int>>(sub->second.begin(), sub->second.end())
		: std::vector<std::pair<AsyncCache::callbackFunction,int>>();
	return callbacks;
}

Record AsyncCache::getRecord(std::string topic) const {
	std::promise<Record> recordPromise;

	_io_service.dispatch([topic, &recordPromise, this] () { 
		auto ri = _recordCache.find(topic);
		auto r =  ri != _recordCache.end()	? Record(ri->second): Record();
		recordPromise.set_value(r);
	});

	auto f = recordPromise.get_future().get();
	return f;
}

void AsyncCache::getRecord(std::string topic, std::function<void (Record)> getter) const {
	_io_service.post([topic, getter, this] () { 
		auto ri = _recordCache.find(topic);
		auto r =  ri != _recordCache.end()	? Record(ri->second): Record();
		getter(r);
	});
}