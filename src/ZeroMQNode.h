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

#ifndef TRANS_PERF_TEST_ZEROMQNODE_H
#define TRANS_PERF_TEST_ZEROMQNODE_H

#include <zmq.h>
#include <cstdint>
#include <memory>
#include "Node.h"
#include "common.h"

class ZeroMQNode : public Node  {
 private:
  void *ctx_;
  void *s_;
  std::unique_ptr<uint8_t[]> data_;
  uint8_t *d_idx_;
  uint8_t *d_end_;
  void alloc_init_data(std::size_t total_data_size);
  bool listener;

 public:
  // Connect
  explicit ZeroMQNode(std::string trans_prefix, std::string target_node_id, uint16_t target_port, std::size_t total_data_size);

  // Listen
  explicit ZeroMQNode(std::string trans_prefix, uint16_t listening_port, std::size_t total_data_size);

  ~ZeroMQNode();

  void barrier() override;

  int send(std::size_t sz) override;

  int recv(std::size_t sz) override;

  bool verify_transmission_data() override {
    return std::all_of(data_.get(), d_end_, [](uint8_t v){return v == fill_value;});
  }
};


#endif //TRANS_PERF_TEST_ZEROMQNODE_H
