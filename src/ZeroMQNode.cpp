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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <system_error>
#include "common.h"
#include "ZeroMQNode.h"


// Connect
ZeroMQNode::ZeroMQNode(std::string trans_prefix,
                       std::string target_node_id,
                       uint16_t target_port,
                       std::size_t total_data_size) {
  ctx_ = zmq_init(1);
  if (!ctx_)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  s_ = zmq_socket(ctx_, ZMQ_PUSH);
  if (!s_)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  std::string addr = trans_prefix + target_node_id + ":" + std::to_string(target_port);
  if (0 != zmq_connect(s_, addr.c_str()))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  alloc_init_data(total_data_size);
  listener = false;
}

// Listen
ZeroMQNode::ZeroMQNode(std::string trans_prefix,
                       uint16_t listening_port,
                       std::size_t total_data_size) {
  ctx_ = zmq_init(1);
  if (!ctx_)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  s_ = zmq_socket(ctx_, ZMQ_PULL);
  if (!s_)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  std::string addr = trans_prefix + std::to_string(listening_port);
  if (0 != zmq_bind(s_, addr.c_str()))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  alloc_init_data(total_data_size);
  listener = true;
}

ZeroMQNode::~ZeroMQNode() {
  if (0 != zmq_close (s_))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  if (0 != zmq_ctx_term (ctx_))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

int ZeroMQNode::send(std::size_t sz) {
  int rc = zmq_send(s_, d_idx_, sz, 0);
  if (rc == -1)
    return 0;
  d_idx_ += rc;
  if (d_idx_ == d_end_)
    d_idx_ = data_.get();
  return rc;
}

int ZeroMQNode::recv(std::size_t sz) {
  int rc = zmq_recv(s_, d_idx_, sz, 0);
  if (rc == -1)
    return 0;
  d_idx_ += rc;
  if (d_idx_ == d_end_)
    d_idx_ = data_.get();
  return rc;
}

void ZeroMQNode::barrier() {
  if (!listener) {
    uint8_t s = 'x';
    if (1 != zmq_send(s_, &s, 1, 0))
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  } else {
    uint8_t r = 0;
    if (1 != zmq_recv(s_, &r, 1, 0))
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    assert(r == 'x');
  }
}

void ZeroMQNode::alloc_init_data(std::size_t total_data_size) {
  data_.reset(new uint8_t[total_data_size]);
  std::fill_n(data_.get(), total_data_size, fill_value);
  d_idx_ = &data_[0];
  d_end_ = d_idx_ + total_data_size;
}
