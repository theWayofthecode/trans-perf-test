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

#include <unistd.h>
#include <stdexcept>
#include "CmdArg.h"

CmdArg::CmdArg(int argc, char **argv) {
  std::string options("t:n:p:s:c:u:e:v");
  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, options.c_str())) != -1) {
    switch (c) {
      case 't':
        node_type_ = optarg;
        break;
      case 'n':
        //TODO: std::to_string ?
        node_id_ = std::stoi(optarg);
        break;
      case 'p':
        port_ = std::stoi(optarg);
        break;
      case 's':
        total_data_size_ = static_cast<std::size_t> (std::stoi(optarg));
        break;
      case 'c':
        chunk_size_ = static_cast<std::size_t> (std::stoi(optarg));
        break;
      case 'u':
        reps_ = std::stoi(optarg);
        break;
      case 'v':
        version_ = true;
        break;
      case 'e':
        experiment_ = optarg;
        break;
      default:
        throw std::invalid_argument("Unrecognized option in argument list.");
    }
  }
}