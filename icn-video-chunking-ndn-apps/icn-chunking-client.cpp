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

// custom-app.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ns3/integer.h"
#include "icn-short-names.h"
#include "icn-chunking-client.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include <time.h>
#include <ndn-cxx/encoding/block.hpp>


#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE("icnVideoChunkingClient");

namespace ns3 {
	static uint64_t get_time() {
		struct timeval tv;

		gettimeofday(&tv, NULL);
		return ((uint64_t)(tv.tv_sec * 1000000) + (uint64_t)tv.tv_usec);
	}

	NS_OBJECT_ENSURE_REGISTERED(icnVideoChunkingClient);

	// register NS-3 type
	TypeId
	icnVideoChunkingClient::GetTypeId()
	{
		static TypeId tid = TypeId("icnVideoChunkingClient")
			.SetParent<ndn::App>()
			.AddConstructor<icnVideoChunkingClient>()
			.AddAttribute("ClientId","ClientId",
						  IntegerValue(100),
						  MakeIntegerAccessor(&icnVideoChunkingClient::client_id),
						  MakeIntegerChecker<uint32_t>());
		return tid;
	}

	// Processing upon start of the application
	void
	icnVideoChunkingClient::StartApplication()
	{
		struct video *v;

		memset(&this->helper.stats, 0, sizeof(this->helper.stats));
		this->helper.video_access_dist = UNI;
		this->helper.video_size_dist = UNI;
		sprintf(this->log_file, "client-%d-logs.txt", this->client_id);
		this->log_fp = fopen(this->log_file, "w");

		printf("ClientID = %d\n", this->client_id);
		// initialize ndn::App
		ndn::App::StartApplication();

		// Add entry to FIB for `/prefix/sub`
		ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);

		this->helper.read_video_file();
		v = this->helper.get_next_video();
		// Schedule send of first interest

		Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::SendInterest, this);
	}

	// Processing when application is stopped
	void
	icnVideoChunkingClient::StopApplication()
	{
		// cleanup ndn::App
		ndn::App::StopApplication();
	}

	void
	icnVideoChunkingClient::SendInterest()
	{
		struct video *video;
		char interest_name[50];
		UniformVariable rand(0, std::numeric_limits<uint32_t>::max());

		if(this->helper.video_state.active == 1) {
			sprintf(interest_name, "/prefix/sub/video_%d/%d",
					this->helper.video_state.video->index, this->helper.video_state.current_offset);
		} else {
			video = this->helper.get_next_video();
			this->helper.new_video_started();
			this->helper.video_state.video = video;
			sprintf(interest_name, "/prefix/sub/video_%d/%d",
					this->helper.video_state.video->index,
					this->helper.video_state.current_offset);
			this->helper.video_state.active = 1;
#if 0
			fprintf(this->log_fp, "Watching video %d ", video->index);
#endif
		}

		auto interest = std::make_shared<ndn::Interest>(interest_name);
		interest->setNonce(rand.GetValue());
		interest->setInterestLifetime(ndn::time::seconds(10));

		THIS_CHUNK_REQ_TIME = get_time();

		m_transmittedInterests(interest, this, m_face);

		m_face->onReceiveInterest(*interest);
		fflush(this->log_fp);
	}


	/* This function would never be called */
	void icnVideoChunkingClient::OnInterest(std::shared_ptr<const ndn::Interest> interest) {
	}

	/*
	 * When Data arrives: Lot of stuff to do!
	 */
	void
	icnVideoChunkingClient::OnData(std::shared_ptr<const ndn::Data> data)
	{
		return;
		uint32_t buffering_time;

		ndn::Block block = data->getContent();
		// std::cout << "C:[DATA]\t<--" << data->getName() << " Size:" 
		// 		  << block.value_size() << std::endl;

		NCHUNKS++;
		fprintf(this->log_fp, "%lu, %lu, %lu, %lu\n",
				this->helper.video_state.video->index,
				TOTAL_START_TIME, TOTAL_VIEW_TIME,
				TOTAL_BUFFERING_TIME);
//			fprintf(this->log_fp, ".");

		/* Buffering time for this chunk */
		buffering_time = get_time() - THIS_CHUNK_REQ_TIME;

		if(LAST_CHUNK_VIEW_TIME == 0) {
			/* This is the first chunk */
			TOTAL_START_TIME += buffering_time;
		}

		if(buffering_time > LAST_CHUNK_VIEW_TIME) {
			TOTAL_BUFFERING_TIME += buffering_time - LAST_CHUNK_VIEW_TIME;
		}
		LAST_CHUNK_VIEW_TIME = block.value_size();
		/* ^^ Assumes constant 360p bitrate */
		/* Approx 5 minute ==> 10 Mb */
		/* 2 * 1024 * 1024 bytes ==> 60 seconds */

		TOTAL_VIEW_TIME += LAST_CHUNK_VIEW_TIME;

		CURRENT_OFFSET += block.value_size();

		if(CURRENT_OFFSET >= this->helper.video_state.video->size) {
			/* New video starts here: TODO Wait time distribution? */
#if 0
			fprintf(this->log_fp, "[DONE]\n");
#endif
			fprintf(this->log_fp, "%d, %Lu, %Lu, %Lu\n",
					this->helper.video_state.video->index,
					TOTAL_START_TIME, TOTAL_VIEW_TIME,
					TOTAL_BUFFERING_TIME);

			this->helper.video_state.active = 0;
			Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::SendInterest, this);
		} else {
			Simulator::Schedule(Seconds(0.0), &icnVideoChunkingClient::SendInterest, this);
		}
		fflush(this->log_fp);
	}
}
