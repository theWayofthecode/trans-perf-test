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
#include <cassert>
#include "CmdArg.h"

CmdArg::CmdArg(int argc, char **argv) {
  std::string options("t:n:h:p:s:b:c:u:e:v");
  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, options.c_str())) != -1) {
    switch (c) {
      case 't':
        node_type_ = optarg;
        break;
      case 'n':
        node_id_ = std::stoi(optarg);
        break;
      case 'h':
        hostname_ = std::string(optarg);
        break;
      case 'p':
        port_ = std::stoi(optarg);
        break;
      case 's':
        total_data_size_ = static_cast<std::size_t> (std::stoi(optarg));
        assert(total_data_size_ > 0);
        break;
      case 'b':
        buf_size_ = static_cast<std::size_t> (std::stoi(optarg));
        assert(buf_size_ > 0);
        break;
      case 'c':
        chunk_size_ = static_cast<std::size_t> (std::stoi(optarg));
        assert(chunk_size_ > 0);
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