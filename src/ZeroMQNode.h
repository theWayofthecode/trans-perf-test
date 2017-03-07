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

#include <scif.h>
#include <memory>
#include <algorithm>
#include <queue>
#include "Node.h"
#include "CmdArg.h"

class ZeroMQNode : public Node<ZeroMQNode> {
 private:
  void *ctx_;
  void *zsock_;
 public:
  ZeroMQNode(CmdArg args);
  ~ZeroMQNode();
  std::size_t send(uint8_t *data, std::size_t sz);
  std::size_t recv(uint8_t *data, std::size_t sz);
  microseconds mem_reg(std::size_t sz) {}
  microseconds mem_unreg() {}

};


#endif //TRANS_PERF_TEST_SCIFNODE_H
