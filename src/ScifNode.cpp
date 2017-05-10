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

#include "ScifNode.h"
#include <system_error>
#include <cassert>
#include <thread>
#include "common.h"

ScifNode::ScifNode(CmdArg args) : Node<ScifNode>(args) {
  auto node_id = args.getNode_id();
  auto port = args.getPort();
  if (node_id == -1) {
    listen(port);
  } else {
    connect(node_id, port);
  }
  alloc_init_data(args.getTotal_data_size());
}

ScifNode::~ScifNode() {
  while(!winQ.empty()) {
    RMAWindow win = winQ.front();
    scif_unregister(epd_.get(), win.off, win.sz);
    winQ.pop();
  }
}

void ScifNode::connect(int node_id, int port) {
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  //If scif_connect() failed because possibly the listener is not ready to accept, then retry
  int i = 0;
  while (-1 == scif_connect(epd_.get(), &target_addr)) {
    if (errno != ECONNREFUSED || i++ >= 100)
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    std::this_thread::sleep_for(milliseconds(50));
    log_msg("Retrying to connect.");
  }
}

void ScifNode::listen(int port) {
  t4s::ScifEpd l;

  if (-1 == scif_bind(l.get(), port))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // listen (backlog = 1)
  if (-1 == scif_listen(l.get(), 1))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // accept
  scif_epd_t acc_epd;
  struct scif_portID peer_addr;
  if (-1 == scif_accept(l.get(), &peer_addr, &acc_epd, SCIF_ACCEPT_SYNC))
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  epd_ = t4s::ScifEpd(acc_epd);
}

void ScifNode::alloc_init_data(std::size_t total_data_size) {
  total_size_ = (((total_data_size - 1)/ PAGE_SIZE) + 1) * PAGE_SIZE;
  //mem_ allocation
  void *m;
  if (0 != posix_memalign(&m, PAGE_SIZE, total_size_));
  mem_.reset(static_cast<uint8_t *>(m));
  mem_end_ = mem_.get() + total_size_;
  std::fill_n(mem_.get(), total_size_, 0);
}

std::size_t ScifNode::send(uint8_t *data, std::size_t sz) {
  int rc = scif_send(epd_.get(), data, sz, 0);
  if (-1 == rc);
  return rc;
}

std::size_t ScifNode::recv(uint8_t *data, std::size_t sz) {
  int rc = scif_recv(epd_.get(), data, sz, 0);
  if (-1 == rc);
  return rc;
}

microseconds ScifNode::mem_reg(std::size_t sz) {
  static uint8_t *d = mem_.get();
  if (d >= mem_end_)
    d = mem_.get();

  RMAWindow win;
  auto start = high_resolution_clock::now();
  win.off = scif_register(epd_.get(), d, sz, 0, SCIF_PROT_WRITE | SCIF_PROT_READ, 0);
  auto end = high_resolution_clock::now();
  if  (win.off == SCIF_REGISTER_FAILED);
  win.sz = sz;
  winQ.push(win);
  d += sz;

  return duration_cast<microseconds>(end - start);
}

microseconds ScifNode::mem_unreg() {
  if (winQ.empty())
    return inval_dur;
  RMAWindow win = winQ.front();
  auto start = high_resolution_clock::now();
  if (-1 == scif_unregister(epd_.get(), win.off, win.sz));
  auto end = high_resolution_clock::now();
  winQ.pop();
  return duration_cast<microseconds>(end - start);
}