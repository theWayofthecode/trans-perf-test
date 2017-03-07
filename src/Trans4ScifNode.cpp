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
  int node_id = args.getNode_id();
  int port = args.getPort();
  if (node_id == -1) {
    sock.reset(t4s::listeningSocket(port));
  } else {
    sock.reset(t4s::connectingSocket(node_id, port));
  }
}
