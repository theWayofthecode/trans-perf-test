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

#include <iostream>
#include <string>
#include <cassert>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <memory>
#include <thread>
#include <stdexcept>
#include <system_error>
#include <errno.h>
#include "Node.h"
#include "ScifNode.h"
#include "Trans4ScifNode.h"

enum Version {
  major = 0,
  minor = 2
};

using hrclock = std::chrono::high_resolution_clock;

template<typename DurationType>
inline std::chrono::microseconds cast_microseconds(DurationType d) {
  return std::chrono::duration_cast<std::chrono::microseconds>(d);
}

Node *build_node(std::string, int node_id, int port, std::size_t total_data_size);
void trans_perf(std::function<int(std::size_t)> transmission, int num_transfers, std::size_t chunk_size);


int main(int argc, char **argv) {
  std::string options("t:n:p:s:c:u:v");
  int node_id = -1;
  int port = -1;
  int num_transfers = 0;
  std::size_t chunk_size = 0;
  std::size_t total_data_size = 0;
  std::string trans_type("");
  int c;

  opterr = 0;
  while ((c = getopt(argc, argv, options.c_str())) != -1) {
    switch (c) {
      case 't':
        trans_type = optarg;
        break;
      case 'n':
        node_id = std::stoi(optarg);
        break;
      case 'p':
        port = std::stoi(optarg);
        break;
      case 's': {
        int sz = std::stoi(optarg);
        assert(sz > 0);
        total_data_size = static_cast<std::size_t> (sz);
      }
        break;
      case 'c': {
        int sz = std::stoi(optarg);
        assert(sz > 0);
        chunk_size = static_cast<std::size_t> (sz);
      }
        break;
      case 'u':
        num_transfers = std::stoi(optarg);
        break;
      case 'v':
        std::cout << Version::major << "." << Version::minor << std::endl;
        return 0;
      default:
        std::cerr << "Unrecognized option -" << c << std::endl;
        return -1;
    }
  }
  assert (port > 0);
  assert (chunk_size > 0);
  assert (total_data_size >= chunk_size);
  assert (num_transfers > 0);
  assert (trans_type != "");

  std::unique_ptr<Node> n;
  for (int i = 0; ; ++i) {
    try {
      //if we are building a connecting node, it may fail because the listening node is not ready yet
      //therefore we try for some time to reconnect
      n.reset(build_node(trans_type, node_id, port, total_data_size));
      break;
    } catch (std::system_error se) {
      if (se.code().value() != ECONNREFUSED || i == 1000)
        throw;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  n->barrier(); //we make sure the peers wait for each other
  if (node_id == -1) { //receiver side
    trans_perf([&n](std::size_t cz) { return n->recv(cz); }, num_transfers, chunk_size);
  } else { //sender side
    trans_perf([&n](std::size_t cz) { return n->send(cz); }, num_transfers, chunk_size);
  }
  assert(n->verify_transmission_data());
  return 0;
}

Node *build_node(std::string trans_type, int node_id, int port, std::size_t total_data_size) {
  if (trans_type == "scif") {
    return (node_id == -1) ?
           new ScifNode(port, total_data_size) :
           new ScifNode(node_id, port, total_data_size);
  } else if (trans_type == "trans4scif") {
    return (node_id == -1) ?
           new Trans4ScifNode(port, total_data_size) :
           new Trans4ScifNode(node_id, port, total_data_size);
  } else {
    throw std::invalid_argument("Unsupported transport type: " + trans_type);
  }
}

void trans_perf(std::function<int(std::size_t)> transmission, int num_transfers, std::size_t chunk_size) {
  for (int i = 0; i < num_transfers; ++i) {
    auto start = hrclock::now();
    transmission(chunk_size);
    auto end = hrclock::now();
    std::cout << cast_microseconds(end - start).count() << " ";
  }
}