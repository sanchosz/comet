#include "stdafx.h"
#include "RequestProcessor.h"
#include "mime_types.hpp"
#include <string>
#include <boost/lexical_cast.hpp>
#include <iostream>

namespace comet {
namespace server {

RequestProcessor::RequestProcessor(request_handler& request_handler)
	:request_handler_(request_handler)
{

}

void RequestProcessor::handleRequest(const request& req, reply& rep) {
	std::cout << "Handling request: " << req.uri << std::endl;
	if (req.uri == "/pull.js") {
		rep.status = reply::ok;
		rep.content = "{\"counter\":12}";
		rep.headers.resize(2);
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type("js");
	} else {
		request_handler_.handle_request(req, rep);
	}
}

}
}