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
#define CACHE_SIZE  (1024 * 1024 * 1024)
namespace ns3 {

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
		AnnotatedTopologyReader topologyReader("", 1);
		topologyReader.SetFileName("topologies/simple-tree.txt");
		topologyReader.Read();

		// Install NDN stack on all nodes
		ndn::StackHelper ndnHelper;
		ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru");
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
			char buffer[10];
			sprintf(buffer, "client%d", i);
			Ptr<Node> client = Names::Find<Node>(buffer);

			// Set cache size to 1 (disabled)
			char configstr[100];
			sprintf(configstr, "/NodeList/%d/$ns3::ndn::ContentStore/MaxSize", client->GetId());
			printf("i = %d, cleint-Id = %d\n", i, client->GetId());
			clientApp.SetAttribute("ClientId", IntegerValue(client->GetId()));
			Config::Set (configstr, UintegerValue(1));
			clientApp.Install(client);
		}

		std::string prefix = "/prefix/sub";

		/* Producer */
		ndn::AppHelper serverApp("icnVideoChunkingServer");

		for (int i = 0; i < NUM_SERVERS; i++) {
			char buffer[10];
			sprintf(buffer, "server%d", i);
			Ptr<Node> server = Names::Find<Node>(buffer);
			serverApp.Install(server);

			// Set cache size to 1 (disabled)
			char configstr[100];
			sprintf(configstr, "/NodeList/%d/$ns3::ndn::ContentStore/MaxSize", server->GetId());
			Config::Set (configstr, UintegerValue (1));

			// Set as origin of data for clients
			ndnGlobalRoutingHelper.AddOrigins(prefix, server);
		}

		/* Routers */
		Ptr<Node> routers[NUM_ROUTERS];
		for (int i = 0; i < NUM_ROUTERS; i++) {
			char buffer[10];
			sprintf(buffer, "router%d", i);
			Ptr<Node> router = Names::Find<Node>(buffer);

			// Set cache size to defined size
			char configstr[100];
			sprintf(configstr, "/NodeList/%d/$ns3::ndn::ContentStore/MaxSize", router->GetId());
			Config::Set (configstr, UintegerValue (CACHE_SIZE));
			routers[i] = router;
		}

		// Calculate and install FIBs
		ndn::GlobalRoutingHelper::CalculateRoutes();

		Simulator::Stop(Seconds(600));

		// Create traces for each router
		for (int i = 0; i < NUM_ROUTERS; i++) {
			char buffer[30];
			sprintf(buffer, "cs-trace-router-%d.txt", i);
			ndn::CsTracer::Install(routers[i], buffer, Seconds(1));
		}

		Simulator::Run();
		Simulator::Destroy();

		return 0;
	}

} // namespace ns3

int
main(int argc, char* argv[])
{
	return ns3::main(argc, argv);
}
