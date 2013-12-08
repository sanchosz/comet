// test-async-cache.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>
#include <boost/thread/barrier.hpp>
using namespace boost::unit_test;

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "../comet-service/AsyncCache.h"
#include "Barrier.h"

using std::string;

BOOST_AUTO_TEST_SUITE( AsyncCache_Suite )

BOOST_AUTO_TEST_CASE( SubscribePublish )
{
     AsyncCache ac;
	 Record actRecord;
	 string actTopic = "/a/b/c";
	 Record expRecord;
	 string expTopic;
	
	 Barrier barrier(2);
	 std::unique_lock<std::mutex> testlock = barrier.makeLock();

	 auto h = ac.subscribeOnTopic(actTopic, [&] (string topic, Record record) {
		 std::unique_lock<std::mutex> lock = barrier.makeLock();
		 BOOST_MESSAGE("Callback is called");
		 expTopic= topic; 
		 expRecord = record;
		 barrier.notify_one();
	 });
	 actRecord.add(std::make_pair("Name", "Test field"));
	 ac.publishRecord(actTopic, actRecord);

	 barrier.wait(testlock);
	 BOOST_REQUIRE(expRecord == actRecord);
	 BOOST_REQUIRE(expTopic == actTopic);
}

BOOST_AUTO_TEST_CASE( TwoSubscribersPublish)
{
     AsyncCache ac;
	 Record actRecord;
	 string actTopic = "/FX/USD/EUR";
	 Record expRecord1, expRecord2;
	 string expTopic1, expTopic2;
	 std::mutex mutex;
	 std::condition_variable barier;
	 int counter = 0;

	 std::unique_lock<std::mutex> testlock(mutex);

	 ac.subscribeOnTopic(actTopic, [&] (string topic, Record record) {
		 std::unique_lock<std::mutex> lock(mutex);
		 BOOST_MESSAGE("1-st callback is called");
		 expTopic1= topic; 
		 expRecord1 = record;
		 counter++;
		 barier.notify_one();
	 });
	 ac.subscribeOnTopic(actTopic, [&] (string topic, Record record) {
		 std::unique_lock<std::mutex> lock(mutex);
		 BOOST_MESSAGE("2-st callback is called");
		 expTopic2 = topic; 
		 expRecord2 = record;
		 counter++;
		 barier.notify_one();
	 });
	 actRecord.add(std::make_pair("Name", "EUR"));
	 actRecord.add(std::make_pair("rate", "1.5"));
	 ac.publishRecord(actTopic, actRecord);

	 barier.wait(testlock, [&] () -> bool {return counter == 2;});
	 BOOST_REQUIRE(expRecord1 == actRecord);
	 BOOST_REQUIRE(expTopic1 == actTopic);
	 BOOST_REQUIRE(expRecord2 == actRecord);
	 BOOST_REQUIRE(expTopic2 == actTopic);
}

BOOST_AUTO_TEST_CASE( TwoSubscribersPublish2)
{
     AsyncCache ac;
	 Record actRecord;
	 string actTopic = "/FX/CHF/EUR";
	 Record expRecord1, expRecord2;
	 string expTopic1, expTopic2;
	 std::mutex mutex;
	 std::condition_variable barier;
	 int counter = 0;

	 std::unique_lock<std::mutex> testlock(mutex);

	 auto h1 = ac.subscribeOnTopic(actTopic, [&] (string topic, Record record) {
		 BOOST_MESSAGE("must not be called");
	 });
	 auto h2 = ac.subscribeOnTopic(actTopic, [&] (string topic, Record record) {
		 std::unique_lock<std::mutex> lock(mutex);
		 BOOST_MESSAGE("2-st callback is called");
		 expTopic2 = topic; 
		 expRecord2 = record;
		 counter++;
		 barier.notify_one();
	 });
	 h1.unsubscribe();
	 actRecord.add(std::make_pair("Name", "CHF"));
	 actRecord.add(std::make_pair("rate", "2.7"));
	 ac.publishRecord(actTopic, actRecord);

	 barier.wait(testlock, [&] () -> bool {return counter == 1;});
	 BOOST_REQUIRE(expRecord2 == actRecord);
	 BOOST_REQUIRE(expTopic2 == actTopic);
}


BOOST_AUTO_TEST_CASE(PublishGetRecord)
{
	 AsyncCache ac;
	 Record expRecord;
	 expRecord.add(std::make_pair("CCY", "GBP"));
	 string topic = "/fi/ccy/g";
	 	 
	 ac.publishRecord(topic, expRecord);
	 Record actRecord = ac.getRecord(topic);

	 BOOST_REQUIRE(expRecord == actRecord);
}

BOOST_AUTO_TEST_CASE(PublishGetRecordFunc)
{
	 AsyncCache ac;
	 Record expRecord, actRecord;
	 expRecord.add(std::make_pair("CCY", "UAH"));
	 string topic = "/fi/ccy/ua";
	 
	 std::mutex mutex;
	 std::condition_variable barier;
	 int counter = 0;
	 std::unique_lock<std::mutex> testlock(mutex);

	 std::atomic<int> fence = 0;

	 ac.publishRecord(topic, expRecord);
	 ac.getRecord(topic, [&](Record r) {
		 actRecord = r; 
		 counter++;
		 barier.notify_one();
	 });

	 barier.wait(testlock, [&] () -> bool {return counter == 1;});
	 BOOST_REQUIRE(expRecord == actRecord);
}

BOOST_AUTO_TEST_SUITE_END()
