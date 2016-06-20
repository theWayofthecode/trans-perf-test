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
#include <unistd.h>
#include <memory>
#include <stdexcept>
#include "Node.h"
#include "ScifNode.h"

//enum the version

Node *build_node(std::string, int node_id, int port, std::size_t total_data_size);
//void benchmark(Node *n, std::size_t total_data_size, std::size_t chunk_size);

int main(int argc, char **argv) {
  std::string options("t:n:p:s:c:v");
  int node_id = -1;
  int port = -1;
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
      case 'v':
        std::cout << "version" << std::endl;
        break;
      default:
        std::cerr << "Unrecognized option -" << c << std::endl;
        return -1;
    }
  }
  assert (port > 0);
  assert (chunk_size > 0);
  assert (total_data_size > 0);
  assert (trans_type != "");

  std::unique_ptr<Node> n(build_node(trans_type, node_id, port, total_data_size));
  return 0;
}

Node *build_node(std::string trans_type, int node_id, int port, std::size_t total_data_size) {
  if (trans_type == "scif") {
    return (node_id == -1) ? new ScifNode(port, total_data_size) : new ScifNode(node_id, port, total_data_size);
  } else {
    throw std::invalid_argument("Unsupported transport type: "+trans_type);
  }
}

//void benchmark(Node *n, std::size_t total_data_size, std::size_t chunk_size) {
  //
//}
