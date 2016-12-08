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
#include <cstring>
#include <cstdlib>
#include "ScifFenceNode.h"
#include "common.h"

ScifFenceNode::ScifFenceNode(int node_id, int port) {
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  if (scif_connect(epd_.get(), &target_addr) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  int rc = scif_recv(epd_.get(), &off_, sizeof(off_), SCIF_RECV_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

ScifFenceNode::ScifFenceNode(int port) {
  ScifEpd l;

  if (scif_bind(l.get(), port) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // listen (backlog = 1)
  if (scif_listen(l.get(), 1) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // accept
  scif_epd_t acc_epd;
  struct scif_portID peer_addr;
  if (scif_accept(l.get(), &peer_addr, &acc_epd, SCIF_ACCEPT_SYNC) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  epd_ = ScifEpd(acc_epd);

  //mem_ allocation
  int err = posix_memalign(&mem_, PAGE_SIZE, PAGE_SIZE);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  std::memset(mem_, 0, PAGE_SIZE);
  off_ = scif_register(acc_epd, mem_, PAGE_SIZE, 0, SCIF_PROT_WRITE, 0);
  if (off_ == SCIF_REGISTER_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  int rc = scif_send(epd_.get(), &off_, sizeof(off_), SCIF_SEND_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

ScifFenceNode::~ScifFenceNode() {
  if (mem_) {
    scif_unregister(epd_.get(), off_, PAGE_SIZE);
    free(mem_);
  }
}

int ScifFenceNode::send(std::size_t sz) {
  if (scif_fence_signal(epd_.get(), 0, 0, off_, val_++, SCIF_FENCE_INIT_SELF | SCIF_SIGNAL_REMOTE) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  return sz;
}

int ScifFenceNode::recv(std::size_t sz) {
  uint64_t cur_val = val_;
  val_++;
  int tolerance = 1000000;
  int i;
  for (i = 0; i < tolerance; ++i) {
    if (cur_val == *static_cast<uint64_t *>(mem_))
      return sz;
  }
  if (i == tolerance)
    throw;
}

void ScifFenceNode::barrier() {
  char s = 'x';
  int rc = scif_send(epd_.get(), &s, 1, SCIF_SEND_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  char r = 0;
  rc = scif_recv(epd_.get(), &r, 1, SCIF_RECV_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  assert(r == 'x');
}
