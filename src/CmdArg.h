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

#ifndef TRANS_PERF_TEST_CMDARG_H
#define TRANS_PERF_TEST_CMDARG_H

#include <string>

class CmdArg {
 private:
  int node_id_ = -1;
  int port_ = -1;
  int num_transfers_ = 0;
  bool RTT_perf_ = false;
  std::size_t chunk_size_ = 0;
  std::size_t total_data_size_ = 0;
  std::string trans_type_;
  bool version_ = false;

 public:
  CmdArg(int argc, char **argv);

  int getNode_id() const {
    return node_id_;
  }

  int getPort() const {
    return port_;
  }

  int getNum_transfers() const {
    return num_transfers_;
  }

  bool getRTT_perf() const {
    return RTT_perf_;
  }

  std::size_t getChunk_size() const {
    return chunk_size_;
  }

  const std::size_t &getTotal_data_size() const {
    return total_data_size_;
  }

  bool getVersion() const {
    return version_;
  }

  std::string getTrans_type() const {
    return trans_type_;
  }
};

#endif //TRANS_PERF_TEST_CMDARG_H
