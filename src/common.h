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

#ifndef TRANS_PERF_TEST_COMMON_H
#define TRANS_PERF_TEST_COMMON_H

#include <string>
#include <chrono>
#include <ctime>

#define INT_TO_STR_(i) #i
#define INT_TO_STR(i) INT_TO_STR_(i)
#define __FILE__LINE__ (__FILE__ + std::string(":") + INT_TO_STR(__LINE__))

// Used to fill the transmission data arrays
constexpr uint8_t fill_value = 0xAB;
constexpr std::size_t PAGE_SIZE = 0x1000;

void if_serr_throw(int rc);
void if_rerr_throw(bool err, std::string what);

using namespace std::chrono;

constexpr microseconds inval_dur = duration_values<microseconds>::max();

void log_msg(std::string msg);
#endif //TRANS_PERF_TEST_COMMON_H
