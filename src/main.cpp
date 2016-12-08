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
#include "ScifFenceNode.h"
#include "BlockingNode.h"
#include "ZeroMQNode.h"

enum Version {
  major = 4,
  minor = 0
};

using hrclock = std::chrono::high_resolution_clock;

template<typename DurationType>
inline std::chrono::microseconds cast_microseconds(DurationType d) {
  return std::chrono::duration_cast<std::chrono::microseconds>(d);
}

Node *build_node(std::string, int node_id, int port, std::size_t total_data_size);
void trans_perf(std::function<int(std::size_t)> transmission, int num_transfers, std::size_t chunk_size);
int run(int argc, char **argv);

int main(int argc, char **argv) {
  try {
    return run(argc, argv);
  } catch (std::exception &e) {
    std::cerr << "MAIN: Exception: " << e.what() << std::endl;
    return -1;
  }
}

int run(int argc, char **argv) {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
        std::cout << "=trans4scif=\n";
        std::cout << t4s::trans4scif_config() << std::endl;
        return 0;
      default:
        std::cerr << "Unrecognized option -" << c << std::endl;
        return -1;
    }
  }
  assert (port > 0);
  assert (chunk_size > 0);
  assert (num_transfers > 0);
  assert (trans_type != "");

  std::unique_ptr<Node> n;
  for (int i = 0; ; ++i) {
    try {
      //if we are building a connecting node, it may fail because the listening node is not ready yet
      //therefore we try for some time to reconnect
      n.reset(build_node(trans_type, node_id, port, total_data_size));
      break;
    } catch (std::system_error e) {
      if (e.code().value() != ECONNREFUSED || i == 2000)
        throw;
    }
  }
  n->barrier(); //we make sure the peers wait for each other
  if (node_id == -1) { //receiver side
    trans_perf([&n](std::size_t cz) { return n->recv(cz); }, num_transfers, chunk_size);
  } else { //sender side
    trans_perf([&n](std::size_t cz) { return n->send(cz); }, num_transfers, chunk_size);
  }
  //n->barrier(); //we make sure the peers wait for each other
  //assert(n->verify_transmission_data());
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
  } else if (trans_type == "sciffence") {
    return (node_id == -1) ?
           new ScifFenceNode(port) :
           new ScifFenceNode(node_id, port);
  } else if (trans_type == "blocking") {
    return (node_id == -1) ?
           new BlockingNode(port) :
           new BlockingNode(node_id, port);
  } else if (trans_type == "zeromqtcp") {
    return (node_id == -1) ?
           new ZeroMQNode("tcp://*:", port, total_data_size) :
           new ZeroMQNode("tcp://", "mic0", port, total_data_size);
  } else if (trans_type == "zeromqscif") {
    return (node_id == -1) ?
           new ZeroMQNode("scif://", port, total_data_size) :
           new ZeroMQNode("scif://", std::to_string(node_id), port, total_data_size);
  } else {
    throw std::invalid_argument("Unsupported transport type: " + trans_type);
  }
}

void trans_perf(std::function<int(std::size_t)> transmission, int num_transfers, std::size_t chunk_size) {
  for (int i = 0; i < num_transfers; ++i) {
    auto start = hrclock::now();
    assert (chunk_size == transmission(chunk_size));
    auto end = hrclock::now();
    std::cout << cast_microseconds(end - start).count() << " ";
  }
}
