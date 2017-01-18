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
#include <system_error>
#include <iostream>
#include "common.h"

void if_serr_throw(int rc) {
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

void if_rerr_throw(bool err, std::string what) {
  if (err)
    throw std::runtime_error(what);
}

void log_msg(std::string msg) {
  static std::string program_name(msg);
  std::cerr << program_name << ": " << msg << std::endl;
}
