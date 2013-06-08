#pragma once
#include <boost/noncopyable.hpp>
#include "request_handler.hpp"
#include "request.hpp"
#include "reply.hpp"

namespace comet {
namespace server {

class RequestProcessor
	: public boost::noncopyable
{
	private:
		request_handler& request_handler_;
	public:
		RequestProcessor(request_handler& request_handler);
		void handleRequest(const request& req, reply& rep);
};

} // namespace server
} // namespace comet