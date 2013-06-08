#include "stdafx.h"

#include <boost\asio.hpp>
#include <boost\thread.hpp>

#include <functional>
#include <algorithm>

#include "AsyncCache.h"

AsyncCache::AsyncCache(std::size_t numThreads)
	:_work(_io_service)
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
	// TODO: add lock
	_recordCache[topic] = record;
	fireTopicChanged(topic);
}

void AsyncCache::subscribeOnTopic(std::string topic, callbackFunction callback) {
	// TODO: add lock
	if (_subscriptions.find(topic) == _subscriptions.end())
		_subscriptions[topic] = std::list<callbackFunction>(1, callback);
	else 
		_subscriptions[topic].push_back(callback);
}

void AsyncCache::fireTopicChanged(std::string topic) {
	// TODO: add thread safity
	Record record = getRecord(topic);
	auto callbacks = getSubscriptions(topic);

	for(auto cb = callbacks.begin(); cb != callbacks.end(); ++cb)
		_io_service.post(std::bind(*cb, topic, record));
}

std::vector<AsyncCache::callbackFunction> AsyncCache::getSubscriptions(std::string topic) const {
	std::unique_lock<std::mutex> lock(_subscribtionMutex);
	auto sub = _subscriptions.find(topic);
	return sub != _subscriptions.end()
		? std::vector<callbackFunction>(sub->second.begin(), sub->second.end())
		: std::vector<callbackFunction>();
}

Record AsyncCache::getRecord(std::string topic) const {
	std::unique_lock<std::mutex> lock(_recordMutex);

	auto ri = _recordCache.find(topic);
	return ri != _recordCache.end()
		? Record(ri->second)
		: Record();
}