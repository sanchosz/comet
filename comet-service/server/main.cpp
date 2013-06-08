//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "server.hpp"

int main(int argc, char* argv[])
{
  try
  {
    using boost::property_tree::ptree;
	using std::string;
	using std::size_t;
	ptree properties;

	boost::property_tree::read_json("config.json", properties);


	// Initialise the server.
	string host = properties.get("server.host", "0.0.0.0");
	string port = properties.get("server.port", "80");
	string rootDir = properties.get("processors.file_processor.root", ".");
	size_t num_threads = boost::lexical_cast<size_t>(properties.get("server.threads.number", "1"));
	
    comet::server::server s(host, port, rootDir, num_threads);

	std::cout << "Starting "<< num_threads << "-threaded server on " << host << ":" << port <<std::endl;
    // Run the server until stopped.
    s.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
