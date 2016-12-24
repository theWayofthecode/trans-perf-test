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
#include "ScifmmapNode.h"
#include "common.h"

ScifmmapNode::ScifmmapNode(int node_id, int port, std::size_t total_data_size) {
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  if_err_throw(scif_connect(epd_.get(), &target_addr));
  alloc_init_data(total_data_size);
}

ScifmmapNode::ScifmmapNode(int port, std::size_t total_data_size) {
  ScifEpd l;

  if_err_throw(scif_bind(l.get(), port));

  // listen (backlog = 1)
  if_err_throw(scif_listen(l.get(), 1));

  // accept
  scif_epd_t acc_epd;
  struct scif_portID peer_addr;
  if_err_throw(scif_accept(l.get(), &peer_addr, &acc_epd, SCIF_ACCEPT_SYNC));
  epd_ = ScifEpd(acc_epd);

  alloc_init_data(total_data_size);
}

void ScifmmapNode::alloc_init_data(std::size_t total_data_size) {
  total_size_ = ((total_data_size / PAGE_SIZE) + 1) * PAGE_SIZE;

  std::cerr << total_size_ << std::endl;
  //mem_ allocation
  int err = posix_memalign(&mem_, PAGE_SIZE, total_size_);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  std::memset(mem_, 0, PAGE_SIZE);

  off_ = scif_register(epd_.get(), mem_, total_size_, 0, SCIF_PROT_WRITE, 0);
  if (off_ == SCIF_REGISTER_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  if_err_throw(scif_send(epd_.get(), &off_, sizeof(off_), SCIF_SEND_BLOCK));
  if_err_throw(scif_recv(epd_.get(), &roff_, sizeof(roff_), SCIF_RECV_BLOCK));

  if ((rmem_ = scif_mmap(nullptr, total_size_, SCIF_PROT_WRITE, 0, epd_.get(), roff_)) == SCIF_MMAP_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  mem_end_ = static_cast<uint8_t *>(mem_) + total_size_;
  rmem_end_ = static_cast<uint8_t *>(rmem_) + total_size_;
}

ScifmmapNode::~ScifmmapNode() {
  scif_unregister(epd_.get(), off_, total_size_);
  free(mem_);

  if (scif_munmap(rmem_, total_size_) == -1) {
    std::system_error e(errno, std::system_category(), __FILE__LINE__);
    std::cerr << "Warning: scif_munmap: " << e.what() << __FILE__LINE__ << std::endl;
  }
}

int ScifmmapNode::send(std::size_t sz) {
  static volatile uint64_t *p = static_cast<volatile uint64_t *>(rmem_);
  for (int i = 0; i < sz; ++i) {
    *p = val_;
    p += 1;
    if (p >= rmem_end_)
      p = static_cast<volatile uint64_t *>(rmem_);
  }
  return sz;
}

int ScifmmapNode::recv(std::size_t sz) {
  static volatile uint64_t *p = static_cast<volatile uint64_t *>(mem_);
  for (int i = 0; i < sz; ++i) {
    while (*p != val_);
    p += 1;
    if (p >= mem_end_)
      p = static_cast<volatile uint64_t *>(mem_);
  }
  return sz;
}

void ScifmmapNode::barrier() {
  char s = 'x', r = 0;
  if_err_throw(scif_send(epd_.get(), &s, 1, SCIF_SEND_BLOCK));
  if_err_throw(scif_recv(epd_.get(), &r, 1, SCIF_RECV_BLOCK));
  assert(r == 'x');
}
