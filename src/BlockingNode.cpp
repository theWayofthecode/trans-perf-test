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
#include <chrono>
#include <thread>
#include <pthread.h>
#include <mutex>
#include "BlockingNode.h"
#include "common.h"

BlockingNode::BlockingNode(int node_id, int port) {
  connector = true;
  struct scif_portID target_addr;
  target_addr.node = node_id;
  target_addr.port = port;
  if (scif_connect(epd_.get(), &target_addr) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  int rc = scif_recv(epd_.get(), &off_, sizeof(off_), SCIF_RECV_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  //mmap
  if ((mem_ = scif_mmap(nullptr, PAGE_SIZE, SCIF_PROT_WRITE | SCIF_PROT_READ, 0, epd_.get(), off_)) == SCIF_MMAP_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

BlockingNode::BlockingNode(int port) {
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
  off_ = scif_register(acc_epd, mem_, PAGE_SIZE, 0, SCIF_PROT_WRITE | SCIF_PROT_READ, 0);
  if (off_ == SCIF_REGISTER_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  int rc = scif_send(epd_.get(), &off_, sizeof(off_), SCIF_SEND_BLOCK);
  if (rc == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

BlockingNode::~BlockingNode() {
  if (connector) {
    if (scif_munmap(mem_, PAGE_SIZE) == -1) {
      std::system_error e(errno, std::system_category(), __FILE__LINE__);
      std::cerr << "Warning: scif_munmap: " << e.what() << __FILE__LINE__ << std::endl;
    }
  } else {
    scif_unregister(epd_.get(), off_, PAGE_SIZE);
    free(mem_);
  }
}

int BlockingNode::send(std::size_t sz) {
  int err;
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));
  pthread_cond_t * pcond = static_cast<pthread_cond_t *>(mem_);
  pthread_mutex_t * pmutex = static_cast<pthread_mutex_t *>(mem_+sizeof(pthread_cond_t));

  std::cerr << "Waiting\n";
  err = pthread_cond_wait(pcond, pmutex);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);

  return sz;
}

int BlockingNode::recv(std::size_t sz) {
  int err;
  pthread_cond_t * pcond = static_cast<pthread_cond_t *>(mem_);
  pthread_condattr_t attrcond;

/* Initialise attribute to cond. */
  err = pthread_condattr_init(&attrcond);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  err = pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);

/* Initialise cond. */
  err = pthread_cond_init(pcond, &attrcond);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  // MUTEX
  pthread_mutex_t * pmutex = static_cast<pthread_mutex_t *>(mem_+sizeof(pthread_cond_t));
  pthread_mutexattr_t attrmutex;

/* Initialise attribute to mutex. */
  err = pthread_mutexattr_init(&attrmutex);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  err = pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);

/* Initialise mutex. */
  err = pthread_mutex_init(pmutex, &attrmutex);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  //END MUTEX init

  std::this_thread::sleep_for(std::chrono::milliseconds(7000));

  std::cerr << "Signaling\n";
  err = pthread_cond_signal(pcond);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
/* Clean up. */
  std::cerr << "cond_destroy\n";
  err = pthread_cond_destroy(pcond);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  err = pthread_condattr_destroy(&attrcond);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  err = pthread_mutex_destroy(pmutex);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  err = pthread_mutexattr_destroy(&attrmutex);
  if (err)
    throw std::system_error(err, std::system_category(), __FILE__LINE__);
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  return sz;
}

void BlockingNode::barrier() {
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
