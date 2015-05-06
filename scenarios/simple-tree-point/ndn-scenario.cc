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

#define NUM_CLIENTS 5
#define NUM_ROUTERS 1
#define NUM_SERVERS 1
#define ROUTER_NODE 5
#define SERVER_NODE 6
#define CACHE_SIZE  8 * 1024 * 1024
#define SIMULATION_TIME 600

namespace ns3 {


Ptr<Node> router;
FILE* pit_fp;
void printPIT();
int chunk_size = -1;

int get_cache_size() {
  int chunk_size;
  FILE * fp = fopen("/home/harshad/projects/icn-video-chunking/apps/chunk.conf", "r");

  fscanf(fp, "%d", &chunk_size);

  fclose(fp);

  return CACHE_SIZE / chunk_size;
}

/**
 * This scenario simulates a one server, one router, 5 client node tree.
 */
int
main(int argc, char* argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // read nodes from topology file
  //AnnotatedTopologyReader topologyReader("", 1);
  //topologyReader.SetFileName("topologies/simple-tree.txt");
  //topologyReader.Read();

  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("10Mbps"));
  Config::SetDefault("ns3::PointToPointNetDevice::Mtu", UintegerValue (0xFFFF));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  // Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

  // Create all nodes
  NodeContainer nodes;
  nodes.Create(7);

  PointToPointHelper p2p;
  for (int i = 0; i < NUM_CLIENTS; i++)
      p2p.Install(nodes.Get(i), nodes.Get(ROUTER_NODE));

  p2p.Install(nodes.Get(ROUTER_NODE), nodes.Get(SERVER_NODE));

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru");
  //  ndnHelper.SetPit ("ns3::ndn::pit::Persistent::AggregateStats");
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  /* Consumer */
  ndn::AppHelper clientApp("icnVideoChunkingClient");

  // No caching on clients
  for (int i = 0; i < NUM_CLIENTS; i++) {
    Ptr<Node> client = nodes.Get(i);
    clientApp.SetAttribute("ClientId", IntegerValue(client->GetId()));
    clientApp.Install(client);

    // Set cache size to 1 (disabled)
    char configstr[100];
    sprintf(configstr, "/NodeList/%d/$ns3::ndn::ContentStore/MaxSize", client->GetId());
    Config::Set (configstr, UintegerValue (1));
  }

  std::string prefix = "/prefix/sub";

  /* Producer */
  ndn::AppHelper serverApp("icnVideoChunkingServer");

  Ptr<Node> server = nodes.Get(SERVER_NODE);
  serverApp.Install(server);

  // Set cache size to 1 (disabled)
  char configstr[100];
  sprintf(configstr, "/NodeList/%d/$ns3::ndn::ContentStore/MaxSize", server->GetId());
  Config::Set (configstr, UintegerValue (1));

  // Set as origin of data for clients
  ndnGlobalRoutingHelper.AddOrigins(prefix, server);

  /* Routers */
  router = nodes.Get(ROUTER_NODE);

  // Set cache size to defined size
  char configstr2[100];
  sprintf(configstr2, "/NodeList/%d/$ns3::ndn::ContentStore/MaxSize", router->GetId());

  Config::Set (configstr2, UintegerValue (get_cache_size()));

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Simulator::Stop(Seconds(SIMULATION_TIME));

  // Create traces for each router
  char buffer[30];
  sprintf(buffer, "cs-trace-router.txt");
  ndn::CsTracer::Install(router, buffer, Seconds(1));

  // Print out pit state every second
  pit_fp = fopen("pit-log.txt", "w");
  for (int i = 0; i < SIMULATION_TIME; i++) {
    Simulator::Schedule(Seconds(i), printPIT);
  }

  Simulator::Run();
  Simulator::Destroy();
  fclose(pit_fp);

  return 0;
}

// PIT tracker
void printPIT()
{
  Time simTime = Simulator::Now();
  auto pitSize = router->GetObject<ndn::L3Protocol>()->getForwarder()->getPit().size();

  fprintf(pit_fp, "%f, %d\n", simTime.GetSeconds(), pitSize);
  fflush(pit_fp);
}



} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
