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

#ifndef TRANS_PERF_TEST_NODE_H
#define TRANS_PERF_TEST_NODE_H

#include <chrono>
#include <ctime>
#include <stdexcept>
#include <functional>
#include <iostream>
#include <algorithm>
#include "common.h"
#include "CmdArg.h"

static constexpr uint8_t fill_value = 0xAB;

template<typename TransType>
class Node {
 private:
  TransType *this_;
  std::string ex_type_;
  int reps_;
  std::size_t chunk_size_;
  std::unique_ptr<uint8_t> data_;
  uint8_t *data_end_;

  /* Experiments */
  void mem_reg() {
    auto timing = this_->mem_reg(chunk_size_);
    if (timing == inval_dur)
      throw std::runtime_error("mem_reg: returned invalid duration for the timing\n");
    std::cout << timing.count() << " ";
    timing = this_->mem_unreg();
    if (timing == inval_dur)
      throw std::runtime_error("mem_unreg: returned invalid duration for the timing\n");
  }

  void mem_onlyreg() {
    auto timing = this_->mem_reg(chunk_size_);
    if (timing == inval_dur)
      throw std::runtime_error("mem_reg: returned invalid duration for the timing\n");
    std::cout << timing.count() << " ";
  }

  void mem_unreg() {
    auto timing = this_->mem_reg(chunk_size_);
    if (timing == inval_dur)
      throw std::runtime_error("mem_reg: returned invalid duration for the timing\n");
    timing = this_->mem_unreg();
    if (timing == inval_dur)
      throw std::runtime_error("mem_unreg: returned invalid duration for the timing\n");
    std::cout << timing.count() << " ";
  }

  void send_thr() {
    static uint8_t *dp = data_.get();
    std::size_t bytes_sent = 0;
    auto start = high_resolution_clock::now();

    while (bytes_sent < chunk_size_) {
      auto b = this_->send(dp, chunk_size_ - bytes_sent);
      bytes_sent += b;
      dp += b;
      if (dp >= data_end_)
        dp = data_.get();
    }
    auto end = high_resolution_clock::now();
    std::cout << (duration_cast<microseconds>(end - start)).count() << " ";
  }

  void recv_thr() {
    static uint8_t *dp = data_.get();
    static int i = 0;

    std::size_t bytes_recv = 0;
    auto start = high_resolution_clock::now();
    while (bytes_recv < chunk_size_) {
      auto b = this_->recv(dp, chunk_size_ - bytes_recv);
      bytes_recv += b;
      dp += b;
      if (dp >= data_end_)
        dp = data_.get();
    }
    auto end = high_resolution_clock::now();
    std::cout << (duration_cast<microseconds>(end - start)).count() << " ";
  }
  
 //////////////////////////////////////////////////////////////////////////
 public:
  std::function<void()> experiment;

  Node(CmdArg args) :
    this_(static_cast<TransType *> (this)),
    reps_(args.getReps()),
    chunk_size_(args.getChunk_size()),
    ex_type_(args.getExperiment()),
    data_(new uint8_t[args.getTotal_data_size()]),
    data_end_(data_.get() + args.getTotal_data_size()) {

    if (ex_type_ == "mem_reg") {
      experiment = std::bind(&Node::mem_reg, this);
    } else if (ex_type_ == "mem_onlyreg") {
      experiment = std::bind(&Node::mem_onlyreg, this);
    } else if (ex_type_ == "mem_unreg") {
      experiment = std::bind(&Node::mem_unreg, this);
    } else if (ex_type_ == "send_thr") {
      experiment = std::bind(&Node::send_thr, this);
      std::fill(data_.get(), data_end_, fill_value);
    } else if (ex_type_ == "recv_thr") {
      experiment = std::bind(&Node::recv_thr, this);
      std::fill(data_.get(), data_end_, 0);
    }  else {
      throw std::runtime_error("Unknown experiment type: " + ex_type_ + "\n");
    }
  }

  bool experiment_tmp() {
    static int i = 1;
    if (i++ > reps_)
      return false;
    experiment();
    return true;
  }

  bool data_is_valid() {
    // The data buffer may have not changed completely
    // so we determin till what point it must have changed values
    uint8_t *end = data_.get()+(chunk_size_*reps_);
    if (end > data_end_)
      end = data_end_;
    return std::all_of(data_.get(), end, [](uint8_t v) { return v == fill_value; });
  }

  void barrier() {
    static uint8_t sval = 0xbb;
    uint8_t rval = 0;
    int rc = 0;
    do {
      rc = this_->send(&sval, 1);
      if (rc == -1)
        throw std::runtime_error("Node: barrier(): send() error.");
    } while (1 != rc);
    do {
      rc = this_->recv(&rval, 1);
      if (rc == -1)
        throw std::runtime_error("Node: barrier(): recv() error.");
    } while (1 != rc);

    if (sval != rval)
      throw std::runtime_error("Node: barrier(): sval != rval.");
  }
};

#endif //TRANS_PERF_TEST_NODE_H
