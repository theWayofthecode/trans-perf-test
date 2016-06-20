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

class ScifNode : public Node {
 private:

  std::unique_ptr<uint8_t[]> data;
  uint8_t *d_idx;
  uint8_t *d_end;
  scif_epd_t epd;

 public:

  ScifNode(int node_id, int port, std::size_t total_data_size);
  ScifNode(int port, std::size_t total_data_size);

  ~ScifNode() override {
    scif_close(epd);
  }

  int send(std::size_t sz) override;
  int recv(std::size_t sz) override;
};


#endif //TRANS_PERF_TEST_SCIFNODE_H
