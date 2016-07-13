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

#ifndef TRANS_PERF_TEST_SCIFNODE_H
#define TRANS_PERF_TEST_SCIFNODE_H

#include <scif.h>
#include <memory>
#include <algorithm>
#include "Node.h"
#include "scifepd.h"

class ScifNode : public Node {
 private:
  //The data_ can be transmitted multiple times
  std::unique_ptr<uint8_t[]> data_;
  uint8_t *d_idx_;
  uint8_t *d_end_;
  ScifEpd epd_;
  int transmission(int(*trans_prim)(scif_epd_t, void*, int, int), std::size_t sz );

 public:

  ScifNode(int node_id, int port, std::size_t total_data_size);
  ScifNode(int port, std::size_t total_data_size);

  void barrier() override;

  int send(std::size_t sz) override {
    return transmission(scif_send, sz);
  }

  int recv(std::size_t sz) override {
    return transmission(scif_recv, sz);
  }

  bool verify_transmission_data() override {
    return std::all_of(data_.get(), d_end_, [](uint8_t v){return v == fill_value;});
  }
};


#endif //TRANS_PERF_TEST_SCIFNODE_H
