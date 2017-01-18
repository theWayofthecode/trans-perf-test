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
#include <iostream>
#include "common.h"
#include "CmdArg.h"

template<typename TransType>
class Node {
 private:
  TransType *this_;
  std::string ex_type_;
  int reps_;
  std::size_t chunk_size;

  void mem_reg() {
    auto timing = this_->mem_reg(chunk_size);
    if (timing == inval_dur)
      throw std::runtime_error("mem_reg: returned invalid duration for the timing\n");
    std::cout << timing.count() << " ";
    timing = this_->mem_unreg();
    if (timing == inval_dur)
      throw std::runtime_error("mem_unreg: returned invalid duration for the timing\n");
  }

  void mem_onlyreg() {
    auto timing = this_->mem_reg(chunk_size);
    if (timing == inval_dur)
      throw std::runtime_error("mem_reg: returned invalid duration for the timing\n");
    std::cout << timing.count() << " ";
  }

  void mem_unreg() {
    auto timing = this_->mem_reg(chunk_size);
    if (timing == inval_dur)
      throw std::runtime_error("mem_reg: returned invalid duration for the timing\n");
    timing = this_->mem_unreg();
    if (timing == inval_dur)
      throw std::runtime_error("mem_unreg: returned invalid duration for the timing\n");
    std::cout << timing.count() << " ";
  }

 public:

  Node(CmdArg args) {
    this_ = static_cast<TransType *> (this);
    reps_ = args.getReps();
    chunk_size = args.getChunk_size();
    ex_type_ = args.getExperiment();
  }

  bool experiment() {
    static int i = 1;
    if (i++ > reps_)
      return false;
    if (ex_type_ == "mem_reg") {
      mem_reg();
    } else if (ex_type_ == "mem_onlyreg") {
      mem_onlyreg();
    } else if (ex_type_ == "mem_unreg") {
      mem_unreg();
    } else {
      throw std::runtime_error("Unknown experiment type: " + ex_type_ + "\n");
    }
    return true;
  }
  
  //bool verify_experiment_data() = 0;
  void barrier() {
    TransType *this_ = static_cast<TransType *> (this);
    uint8_t sval = 0xaa, rval = 0;
    if_rerr_throw(1 != this_->send(&sval, 1), "Node: barrier(): send() error.");
    if_rerr_throw(1 != this_->recv(&rval, 1), "Node: barrier(): recv() error.");
    if_rerr_throw(sval != rval, "Node: barrier(): sval != rval.");
  }
};

#endif //TRANS_PERF_TEST_NODE_H
