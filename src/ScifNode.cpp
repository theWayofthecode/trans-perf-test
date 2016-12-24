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
#include "ScifNode.h"
#include "common.h"

ScifNode::ScifNode(int node_id, int port, std::size_t total_data_size) {
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  if_err_throw(scif_connect(epd_.get(), &target_addr));

  alloc_init_data(total_data_size);
}

ScifNode::ScifNode(int port, std::size_t total_data_size) {
  ScifEpd l;

  if_err_throw(scif_bind(l.get(), port));

  // listen (backlog = 1)
  if_err_throw(scif_listen(l.get(), 1));

  // accept
  scif_epd_t acc_epd;
  struct scif_portID peer_addr;
  if_err_throw(scif_accept(l.get(), &peer_addr, &acc_epd, SCIF_ACCEPT_SYNC));
  epd_ = ScifEpd(acc_epd);

  alloc_init_data(total_data_size);
}

void ScifNode::alloc_init_data(std::size_t total_data_size) {
  data_.reset(new uint8_t[total_data_size]);
  std::fill_n(data_.get(), total_data_size, fill_value);
  d_idx_ = &data_[0];
  d_end_ = d_idx_ + total_data_size;
}

int ScifNode::transmission(int(*trans_prim)(scif_epd_t, void*, int, int), std::size_t sz ) {
  std::size_t total_trans_size = sz;
  while (total_trans_size) {
    std::size_t to_trans = std::min(total_trans_size, static_cast<std::size_t>(d_end_ - d_idx_));
    int rc = trans_prim(epd_.get(), d_idx_, to_trans, 0);
    if_err_throw(rc);
    d_idx_ += rc;
    if (d_idx_ == d_end_)
      d_idx_ = data_.get();
    total_trans_size -= rc;
  }
  return sz - total_trans_size;
}

void ScifNode::barrier() {
  char s = 'x', r = 0;
  if_err_throw(scif_send(epd_.get(), &s, 1, SCIF_SEND_BLOCK));
  if_err_throw(scif_recv(epd_.get(), &r, 1, SCIF_RECV_BLOCK));
  assert(r == 'x');
}
