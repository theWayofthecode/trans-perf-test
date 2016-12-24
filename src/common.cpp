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
#include "common.h"

void if_err_throw(int rc)
{
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}
