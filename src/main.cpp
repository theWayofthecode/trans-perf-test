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

int main(int argc, char **argv) {
  std::string options("t:a:s:c:v");
  std::string address;
  std::size_t chunk_size = 0;
  int count = -1;

  opterr = 0;
  char c;
  while ((c = getopt(argc, argv, options.c_str())) != -1)
    if (c == 'a')
      address = optarg;
  opterr = 0;
  optind = 1;
  while ((c = getopt(argc, argv, options.c_str())) != -1) {
    switch (c) {
      case 't':
        std::cout << optarg << std::endl;
        break;
      case 'a':
        //was handled in the beginning
        break;
      case 's': {
        int sz = std::stoi(optarg);
        assert(sz > 0);
        chunk_size = static_cast<std::size_t> (sz);
      }
        break;
      case 'c':
        count = std::stoi(optarg);
        break;
      case 'v':
        std::cout << "version" << std::endl;
        break;
      default:
        std::cerr << "Unrecognized option -" << c << std::endl;
        return -1;
    }
  }
  assert (chunk_size > 0);
  assert (count > 0);
  std::cout << address << std::endl;
  std::cout << chunk_size << std::endl;
  std::cout << count << std::endl;
  return 0;
}