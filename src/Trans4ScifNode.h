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

#ifndef TRANS_PERF_TEST_TRANS4SCIFNODE_H
#define TRANS_PERF_TEST_TRANS4SCIFNODE_H

#include <scif.h>
#include <memory>
#include <algorithm>
#include <queue>
#include <trans4scif.h>
#include "Node.h"
#include "CmdArg.h"

class Trans4ScifNode : public Node<Trans4ScifNode> {
 private:
  std::unique_ptr<t4s::Socket> sock;
 public:
  Trans4ScifNode(CmdArg args);
  std::size_t send(uint8_t *data, std::size_t sz) { return sock->send(data, sz); }
  std::size_t recv(uint8_t *data, std::size_t sz) { return sock->recv(data, sz); }
  microseconds mem_reg(std::size_t sz) { return inval_dur; }
  microseconds mem_unreg() { return inval_dur; }
};


#endif //TRANS_PERF_TEST_SCIFNODE_H
