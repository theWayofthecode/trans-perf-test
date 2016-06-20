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
#include <cassert>
#include "ScifNode.h"
#include "common.h"

ScifNode::ScifNode(int node_id, int port, std::size_t total_data_size) : data{new uint8_t[total_data_size]} {
  d_idx = &data[0];
  d_end = d_idx + total_data_size;
  epd = scif_open();
  if(epd == SCIF_OPEN_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  if (scif_connect(epd, &target_addr) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

ScifNode::ScifNode(int port, std::size_t total_data_size) : data{new uint8_t[total_data_size]} {
  d_idx = &data[0];
  d_end = d_idx + total_data_size;
  scif_epd_t l = scif_open();
  if(l == SCIF_OPEN_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  if (scif_bind(l, port) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

//   listen (backlog = 1)
  if (scif_listen(l, 1) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

//   accept
  struct scif_portID peer_addr;
  if (scif_accept(l, &peer_addr, &epd, SCIF_ACCEPT_SYNC) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  if (scif_close(l) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

int ScifNode::send(std::size_t sz) {
  assert(d_idx < d_end);
  int rc = scif_send(epd, d_idx, sz, 0);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  d_idx += rc;
  return rc;
}

int ScifNode::recv(std::size_t sz) {
  assert(d_idx < d_end);
  int rc = scif_recv(epd, d_idx, sz, 0);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  d_idx += rc;
  return rc;
}