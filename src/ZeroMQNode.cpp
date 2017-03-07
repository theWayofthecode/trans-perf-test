/*
    Copyright (c) 2015-2016 CERN

    This software is distributed under the terms of the
    GNU General Public Licence version 3 (GPLv3),
    copied verbatim in the file "LICENSE".
    In applying this licence, CERN does not waive
    the privileges and immunities granted to it by virtue of its status
    as an Intergovernmental Organization or submit itself to any jurisdiction.

    Author: Aram Santogidis <aram.santogidis@cern.ch>
*/

#include "ZeroMQNode.h"
#include <iostream>
#include <cstring>
#include <system_error>
#include <stdexcept>
#include <cassert>
#include <cerrno>
#include <zmq.h>

ZeroMQNode::ZeroMQNode(CmdArg args) :
    Node<ZeroMQNode>(args) {
  if (nullptr == (ctx_ = zmq_ctx_new()))
    throw std::runtime_error("zmq_ctx_new() failed.");
  if (nullptr == (zsock_ = zmq_socket(ctx_, ZMQ_PAIR)))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  int node_id = args.getNode_id();
  int port = args.getPort();
  std::string hostname = args.getHostname();

  if (hostname.empty()) { //SCIF transport selected
    if (node_id == -1) {
      std::string addr = "scif://" + std::to_string(port);
      if (-1 == zmq_bind(zsock_, addr.c_str()))
        throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    } else {
      std::string addr = "scif://" + std::to_string(node_id) + ":" + std::to_string(port);
      if (-1 == zmq_connect(zsock_, addr.c_str()))
        throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    }
  } else { //TCP transport selected
    std::string addr = "tcp://" + hostname + ":" + std::to_string(port);
    if (hostname == "*") {
      if (-1 == zmq_bind(zsock_, addr.c_str()))
        throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    } else {
      if (-1 == zmq_connect(zsock_, addr.c_str()))
        throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    }
  }
}

ZeroMQNode::~ZeroMQNode() {
  if (-1 == zmq_close(zsock_))
    std::cerr << __FILE__LINE__ << std::strerror(errno) << std::endl;
  if (-1 == zmq_ctx_term(ctx_))
    std::cerr << __FILE__LINE__ << std::strerror(errno) << std::endl;

}

std::size_t ZeroMQNode::send(uint8_t *data, std::size_t sz) {
  int rc = zmq_send_const(zsock_, data, sz, 0);
  if (rc == -1) {
    if (errno == EAGAIN)
      return 0;
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  }
  return static_cast<std::size_t>(rc);
}

std::size_t ZeroMQNode::recv(uint8_t *data, std::size_t sz) {
  int rc = zmq_recv(zsock_, data, sz, 0);
  if (rc == -1) {
    if (errno == EAGAIN)
      return 0;
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  }
  if (rc > sz)
    throw std::runtime_error("recv: " + std::to_string(rc) + " > " + std::to_string(sz));// See zmq_recv() manpage
  return static_cast<std::size_t>(rc);
}