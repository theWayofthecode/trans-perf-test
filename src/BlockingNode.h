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

#ifndef TRANS_PERF_TEST_BLOCKINGNODE_H
#define TRANS_PERF_TEST_BLOCKINGNODE_H

#include <scif.h>
#include <memory>
#include <algorithm>
#include <condition_variable>
#include "Node.h"
#include "scifepd.h"

class BlockingNode: public Node {
 private:
  struct sync {
    std::mutex m;
    std::condition_variable cv;
    bool ready = false;
  };
  bool connector = false;
  ScifEpd epd_;
  void *mem_ = nullptr;
  off_t off_;
  uint64_t val_ = 0;
  static constexpr int PAGE_SIZE = 0x1000;
 public:

  BlockingNode(int node_id, int port);
  BlockingNode(int port);
  ~BlockingNode();

  void barrier() override;

  int send(std::size_t sz) override;

  int recv(std::size_t sz) override;

  bool verify_transmission_data() override {
    return true;
  }
};


#endif
