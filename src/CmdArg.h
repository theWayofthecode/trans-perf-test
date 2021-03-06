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
  uint16_t node_id_ = -1;
  uint16_t port_ = -1;
  std::string hostname_;
  int reps_ = 0;
  std::string experiment_;
  std::size_t chunk_size_ = 0;
  std::size_t total_data_size_ = 0;
  std::size_t buf_size_ = 16777280;
  std::string node_type_;
  bool version_ = false;

 public:
  CmdArg(int argc, char **argv);

  uint16_t getNode_id() const {
    return node_id_;
  }

  uint16_t getPort() const {
    return port_;
  }

  std::string getHostname() const {
    return hostname_;
  }

  int getReps() const {
    return reps_;
  }

  std::string getExperiment() const {
    return experiment_;
  }

  std::size_t getChunk_size() const {
    return chunk_size_;
  }

  std::size_t getTotal_data_size() const {
    return total_data_size_;
  }

  std::size_t getBuf_size() const {
    return buf_size_;
  }

  bool getVersion() const {
    return version_;
  }

  std::string getNode_type() const {
    return node_type_;
  }
};

#endif //TRANS_PERF_TEST_CMDARG_H
