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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "CannotResolve"
#ifndef TRANS_PERF_TEST_TRANS4SCIFNODE_H
#define TRANS_PERF_TEST_TRANS4SCIFNODE_H

#include <trans4scif.h>
#include "Node.h"
#include "common.h"

class Trans4ScifNode : public Node  {
 private:
  std::unique_ptr<t4s::Socket> t4ss_;
  std::unique_ptr<uint8_t[]> data_;
  uint8_t *d_idx_;
  uint8_t *d_end_;
  void alloc_init_data(std::size_t total_data_size);

 public:
  // Connect
  Trans4ScifNode(int16_t target_node_id, uint16_t target_port, std::size_t total_data_size) :
      t4ss_(t4s::Connect(target_node_id, target_port))
  { alloc_init_data(total_data_size); }

  // Listen
  Trans4ScifNode(uint16_t listening_port, std::size_t total_data_size) :
      t4ss_(t4s::Listen(listening_port))
  { alloc_init_data(total_data_size); }

  //~Trans4ScifNode() override { std::cerr << "Trans4ScifNode destroyed!\n"; }

  void barrier() override;

  int send(std::size_t sz) override;

  int recv(std::size_t sz) override;

  bool verify_transmission_data() override {
    return std::all_of(data_.get(), d_end_, [](uint8_t v){return v == fill_value;});
  }
};


#endif //TRANS_PERF_TEST_TRANS4SCIFNODE_H
