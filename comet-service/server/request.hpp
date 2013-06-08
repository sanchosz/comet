//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef COMET_SERVER_REQUEST_HPP
#define COMET_SERVER_REQUEST_HPP

#include <string>
#include <vector>
#include "header.hpp"

namespace comet {
namespace server {

/// A request received from a client.
struct request
{
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;
};

} // namespace server
} // namespace comet

#endif // HTTP_SERVER3_REQUEST_HPP
