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
#include <memory>
#include <thread>
#include <stdexcept>
#include <system_error>
#include <errno.h>
#include <trans4scif.h>
#include "CmdArg.h"
//#include "Node.h"
//#include "ScifNode.h"
//#include "Trans4ScifNode.h"
//#include "ScifFenceNode.h"
//#include "ZeroMQNode.h"
//#include "ScifmmapNode.h"


//TODO: what is this? I have also versioning in CMakeList.txt
enum Version {
  major = 5,
  minor = 0
};

using hrclock = std::chrono::high_resolution_clock;

template<typename DurationType>
inline std::chrono::microseconds cast_microseconds(DurationType d) {
  return std::chrono::duration_cast<std::chrono::microseconds>(d);
}

//Node *build_node(std::string, int node_id, int port, std::size_t total_data_size);
//void trans_throughput_perf(std::function<int(std::size_t)> transmission, int num_transfers, std::size_t chunk_size);
//void trans_RTT_perf(std::shared_ptr<Node> &n, int node_id, int num_transfers, std::size_t chunk_size);
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
  CmdArg args(argc, argv);
  if (args.getVersion()) {
    std::cout << Version::major << "." << Version::minor << std::endl;
    std::cout << "=trans4scif=\n";
    std::cout << t4s::trans4scif_config() << std::endl;
    return 0;
  }
//
//  std::shared_ptr<Node> n;
//  for (int i = 0;; ++i) {
//    try {
//      //if we are building a connecting node, it may fail because the listening node is not ready yet
//      //therefore we try for some time to reconnect
//      n.reset(build_node(trans_type, node_id, port, total_data_size));
//      break;
//    } catch (std::system_error e) {
//      if (e.code().value() != ECONNREFUSED || i == 2000)
//        throw;
//    }
//  }
//
//  n->barrier(); //we make sure the peers wait for each other
//  if (RTT_perf) {
//    trans_RTT_perf(n, node_id, num_transfers, chunk_size);
//  } else if (node_id == -1) { // (assuming throughput test) receiver side
//    trans_throughput_perf([&n](std::size_t cz) { return n->recv(cz); }, num_transfers, chunk_size);
//  } else { //sender side
//    trans_throughput_perf([&n](std::size_t cz) { return n->send(cz); }, num_transfers, chunk_size);
//  }
//  assert(n->verify_transmission_data());
//  n->barrier(); //we make sure the peers wait for each other
  return 0;
}

//Node *build_node(std::string trans_type, int node_id, int port, std::size_t total_data_size) {
//  if (trans_type == "scif") {
//    return (node_id == -1) ?
//           new ScifNode(port, total_data_size) :
//           new ScifNode(node_id, port, total_data_size);
//  } else if (trans_type == "trans4scif") {
//    return (node_id == -1) ?
//           new Trans4ScifNode(port, total_data_size) :
//           new Trans4ScifNode(node_id, port, total_data_size);
//  } else if (trans_type == "sciffence") {
//    return (node_id == -1) ?
//           new ScifFenceNode(port) :
//           new ScifFenceNode(node_id, port);
//  } else if (trans_type == "scifmmap") {
//    return (node_id == -1) ?
//           new ScifmmapNode(port, total_data_size) :
//           new ScifmmapNode(node_id, port, total_data_size);
//  } else if (trans_type == "zeromqtcp") {
//    return (node_id == -1) ?
//           new ZeroMQNode("tcp://*:", port, total_data_size) :
//           new ZeroMQNode("tcp://", "mic0", port, total_data_size);
//  } else if (trans_type == "zeromqscif") {
//    return (node_id == -1) ?
//           new ZeroMQNode("scif://", port, total_data_size) :
//           new ZeroMQNode("scif://", std::to_string(node_id), port, total_data_size);
//  } else {
//    throw std::invalid_argument("Unsupported transport type: " + trans_type);
//  }
//}
//
//void trans_throughput_perf(std::function<int(std::size_t)> transmission, int num_transfers, std::size_t chunk_size) {
//  for (int i = 0; i < num_transfers; ++i) {
//    auto start = hrclock::now();
//    transmission(chunk_size);
//    auto end = hrclock::now();
//    std::cout << cast_microseconds(end - start).count() << " ";
//  }
//}
//
//void trans_RTT_perf(std::shared_ptr<Node> &n, int node_id, int num_transfers, std::size_t chunk_size) {
//  if (node_id == -1) { // Receiver
//    for (int i = 0; i < num_transfers; ++i) {
//      n->recv(chunk_size);
//      n->send(chunk_size);
//      std::cout << 0 << " ";
//    }
//  } else { // The node that measures RTT
//    for (int i = 0; i < num_transfers; ++i) {
//      auto start = hrclock::now();
//      n->send(chunk_size);
//      n->recv(chunk_size);
//      auto end = hrclock::now();
//      std::cout << cast_microseconds(end - start).count() << " ";
//    }
//  }
//}