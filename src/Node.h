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

#ifndef TRANS_PERF_TEST_NODE_H
#define TRANS_PERF_TEST_NODE_H

class Node {
 public:
  virtual ~Node() {}
  virtual int send(std::size_t sz) = 0;
  virtual int recv(std::size_t sz) = 0;
  virtual bool verify_transmission_data() = 0;
};

#endif //TRANS_PERF_TEST_NODE_H
