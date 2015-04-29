/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-custom-apps.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3 {

/**
 * This scenario simulates a one-node two-custom-app scenario:
 *
 *   +------+ <-----> (CustomApp)
 *   | Node |
 *   +------+ <-----> (Hijacker)
 *
 *     NS_LOG=CustomApp ./waf --run=ndn-custom-apps
 * https://github.com/cawka/ndnSIM-examples/blob/master/examples/ndn-simple-with-cs-lfu.cc
 */
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);
  NodeContainer nodes;
  PointToPointHelper p2p;

  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

  // Creating nodes
  nodes.Create(3);
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));

  // Install NDN stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.SetDefaultRoutes (true);
  //  ccnxHelper.SetOldContentStore ("ns3::ndn::cs::Freshness::Lfu"); // don't set up max size here, will use default value = 100
  ccnxHelper.InstallAll ();

  // ndn::StackHelper ndnHelper;
  // ndnHelper.SetDefaultRoutes(true);
  // ndnHelper.SetContentStore("ns3::ndn::cs::Freshness::Lfu");
  // ndnHelper.InstallAll();

  // Config::Set("/NodeList/0/$ns3::ndn::ContentStore/MaxSize", UintegerValue(1));
  // Config::Set("/NodeList/1/$ns3::ndn::ContentStore/MaxSize", UintegerValue(2));
  // Config::Set("/NodeList/2/$ns3::ndn::ContentStore/MaxSize", UintegerValue(100000));


  /* Consumer */
  // ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  // Consumer will request /prefix/0, /prefix/1, ...
  // consumerHelper.SetPrefix ("/prefix");
  // consumerHelper.SetAttribute ("Frequency", StringValue ("10")); // 10 interests a second
  // consumerHelper.Install (nodes.Get (0)); // first node
  ndn::AppHelper app1("icnVideoChunkingClient");
  app1.Install(nodes.Get(0));
  app1.SetAttribute("ClientId", IntegerValue(0));

  /* Producer */
  // ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  // Producer will reply to all requests starting with /prefix
  // producerHelper.SetPrefix ("/prefix");
  // producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  // producerHelper.Install (nodes.Get (2)); // last node
  ndn::AppHelper app2("icnVideoChunkingServer");
  app2.Install(nodes.Get(1)); // last node
  ndn::FibHelper::AddRoute(nodes.Get(0), "/prefix/sub", nodes.Get(1), 0);

  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  //  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
