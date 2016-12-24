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

#ifndef TRANS_PERF_TEST_SCIFMMAPNODE_H
#define TRANS_PERF_TEST_SCIFMMAPNODE_H

#include <scif.h>
#include <memory>
#include <algorithm>
#include "Node.h"
#include "scifepd.h"


class ScifmmapNode : public Node {
 private:
  ScifEpd epd_;
  void *mem_ = nullptr;
  void *rmem_ = nullptr;
  void *mem_end_ = nullptr;
  void *rmem_end_ = nullptr;
  off_t off_;
  off_t roff_;
  std::size_t total_size_;
  static constexpr uint64_t val_ = 0xAA;
  static constexpr int PAGE_SIZE=0x1000;

  void alloc_init_data(std::size_t total_data_size);

 public:

  ScifmmapNode(int node_id, int port, std::size_t total_data_size);
  ScifmmapNode(int port, std::size_t total_data_size);
  ~ScifmmapNode();

  void barrier() override;

  int send(std::size_t sz) override;

  int recv(std::size_t sz) override;

  //TODO: implement a proper verification
  bool verify_transmission_data() override {
    return true;
  }
};


#endif
