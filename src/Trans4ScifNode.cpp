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

#include "Trans4ScifNode.h"

Trans4ScifNode::Trans4ScifNode(CmdArg args) : Node<Trans4ScifNode>(args) {
  uint16_t node_id = args.getNode_id();
  if (node_id == static_cast<uint16_t>(-1)) {
    sock.reset(new t4s::Socket(args.getPort()));
  } else {
    sock.reset(new t4s::Socket(node_id, args.getPort()));
  }
}
