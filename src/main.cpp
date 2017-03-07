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

#include <memory>
#include <thread>
#include <stdexcept>
#include <system_error>
#include <cstdlib>
#include <errno.h>
#include <trans4scif.h>
#include "CmdArg.h"
#include "Node.h"
#include "ScifNode.h"
#include "Trans4ScifNode.h"
#include "ZeroMQNode.h"
#include "common.h"


enum Version {
  major = 6,
  minor = 4
};

template<typename NodeType>
void run(NodeType &n, int reps) {
  n.barrier();

  log_msg("Starting experiments.");
  for (int i = 0; i < reps; i++)
    n.experiment();
  log_msg("Completed experiments.");

  if (n.data_is_valid())
    log_msg("Data is valid.");
  else
    log_msg("Data is invalid.");

  n.barrier();
}

int main(int argc, char **argv) {
  log_msg(argv[0]);
  try {

    // Parse the command line arguments
    CmdArg args(argc, argv);
    if (args.getVersion()) {
      std::cout << Version::major << "." << Version::minor << std::endl;
      std::cout << "=trans4scif=\n";
      std::cout << t4s::trans4scif_config() << std::endl;
      return 0;
    }

    //Build the node and run the experiment
    auto node_type = args.getNode_type();
    if (node_type == "scif") {
      ScifNode node(args);
      run<ScifNode>(node, args.getReps());
    } else if (node_type == "trans4scif") {
      Trans4ScifNode node(args);
      run<Trans4ScifNode>(node, args.getReps());
    } else if (node_type == "zeromq") {
      ZeroMQNode node(args);
      run<ZeroMQNode>(node, args.getReps());
    }  else {
      std::cerr << "Unsupported Node type: " << node_type << std::endl;
      return EXIT_FAILURE;
    }

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  log_msg("EXIT");
  return EXIT_SUCCESS;
}
