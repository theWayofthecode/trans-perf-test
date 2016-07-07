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

#include <system_error>
#include <cassert>
#include <algorithm>
#include "ScifNode.h"
#include "common.h"

ScifNode::ScifNode(int node_id, int port, std::size_t total_data_size) {
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  if (scif_connect(epd_.get(), &target_addr) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  //data_ allocation
  data_.reset(new uint8_t[total_data_size]);
  std::fill_n(data_.get(), total_data_size, fill_value);
  d_idx_ = &data_[0];
  d_end_ = d_idx_ + total_data_size;
}

ScifNode::ScifNode(int port, std::size_t total_data_size) {
  ScifEpd l;

  if (scif_bind(l.get(), port) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // listen (backlog = 1)
  if (scif_listen(l.get(), 1) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // accept
  scif_epd_t acc_epd;
  struct scif_portID peer_addr;
  if (scif_accept(l.get(), &peer_addr, &acc_epd, SCIF_ACCEPT_SYNC) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  epd_ = ScifEpd(acc_epd);

  //data_ allocation
  data_.reset(new uint8_t[total_data_size]);
  std::fill_n(data_.get(), total_data_size, 0);
  d_idx_ = &data_[0];
  d_end_ = d_idx_ + total_data_size;
}

int ScifNode::send(std::size_t sz) {
  std::size_t total_to_send = sz;
  std::size_t to_send = std::min(total_to_send, static_cast<std::size_t>(d_end_ - d_idx_));
  int rc = scif_send(epd_.get(), d_idx_, to_send, SCIF_SEND_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  d_idx_ += rc;
  total_to_send -= rc;
  if (d_idx_ == d_end_) {
    d_idx_ = data_.get();
    int rc = scif_send(epd_.get(), d_idx_, total_to_send, SCIF_SEND_BLOCK);
    if (rc == -1)
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    d_idx_ += rc;
    total_to_send -= rc;
  }
  assert(total_to_send == 0);
  return sz - total_to_send;
}

int ScifNode::recv(std::size_t sz) {
  std::size_t total_to_recv = sz;
  std::size_t to_recv = std::min(total_to_recv, static_cast<std::size_t>(d_end_ - d_idx_));
  int rc = scif_recv(epd_.get(), d_idx_, total_to_recv, SCIF_RECV_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  d_idx_ += rc;
  total_to_recv -= rc;
  if (d_idx_ == d_end_) {
    d_idx_ = data_.get();
    int rc = scif_recv(epd_.get(), d_idx_, total_to_recv, SCIF_SEND_BLOCK);
    if (rc == -1)
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    d_idx_ += rc;
    total_to_recv -= rc;
  }
  assert(total_to_recv == 0);
  return sz - total_to_recv;
}

bool ScifNode::verify_transmission_data() {
  return std::all_of(data_.get(), d_end_, [](uint8_t v){return v == fill_value;});
}