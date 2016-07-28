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
#include "Trans4ScifNode.h"
#include "common.h"


void Trans4ScifNode::alloc_init_data(std::size_t total_data_size) {
  data_.reset(new uint8_t[total_data_size]);
  std::fill_n(data_.get(), total_data_size, fill_value);
  d_idx_ = &data_[0];
  d_end_ = d_idx_ + total_data_size;
}

int Trans4ScifNode::send(std::size_t sz) {
  std::size_t total_trans_size = sz;
  while (total_trans_size) {
    std::size_t to_trans = std::min(total_trans_size, static_cast<std::size_t>(d_end_ - d_idx_));
    std::size_t rc = t4ss_->Send(d_idx_, to_trans);
    d_idx_ += rc;
    if (d_idx_ == d_end_)
      d_idx_ = data_.get();
    total_trans_size -= rc;
  }
  return sz - total_trans_size;
}

int Trans4ScifNode::recv(std::size_t sz) {
  std::size_t total_trans_size = sz;
  while (total_trans_size) {
    std::size_t to_trans = std::min(total_trans_size, static_cast<std::size_t>(d_end_ - d_idx_));
    std::size_t rc = t4ss_->Recv(d_idx_, to_trans);
    d_idx_ += rc;
    if (d_idx_ == d_end_)
      d_idx_ = data_.get();
    total_trans_size -= rc;
  }
  return sz - total_trans_size;
}

void Trans4ScifNode::barrier() {
  uint8_t s = 'x';
  assert(t4ss_->Send(&s, 1) == 1);
  uint8_t r = 0;
  for (int i = 0; t4ss_->Recv(&r, 1) < 1; ++i) {
    if (i == 10000)
      throw std::runtime_error("Timeout in barrier");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  assert(r == 'x');
}