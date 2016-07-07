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
#include "Node.h"
#include "scifepd.h"

class ScifNode : public Node {
 private:
  //The data_ can be transmitted multiple times
  std::unique_ptr<uint8_t[]> data_;
  uint8_t *d_idx_;
  uint8_t *d_end_;
  ScifEpd epd_;

 public:

  ScifNode(int node_id, int port, std::size_t total_data_size);
  ScifNode(int port, std::size_t total_data_size);

  int send(std::size_t sz) override;
  int recv(std::size_t sz) override;
  bool verify_transmission_data() override;
};


#endif //TRANS_PERF_TEST_SCIFNODE_H
