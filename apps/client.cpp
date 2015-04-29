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
#include "short-names.h"
#include "client.hpp"

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
		Time current_time = Simulator::Now();
		uint64_t us = current_time.GetMicroSeconds();

		return us;
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
		memset(&this->helper.stats, 0, sizeof(this->helper.stats));
		this->helper.video_access_dist = UNI;
		this->helper.video_size_dist = UNI;
		sprintf(this->log_file, "client-%d-logs.txt", this->client_id);

//		printf("ClientID = %d\n", this->client_id);
		// initialize ndn::App
		ndn::App::StartApplication();

		// Add entry to FIB for `/prefix/sub`
		ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);

		this->helper.set_client_id(this->client_id);
		this->helper.read_video_file();
		// Schedule send of first interest

		Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::SendInterest, this);
		Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::dumpStats, this);
	}

	// Processing when application is stopped
	void
	icnVideoChunkingClient::StopApplication()
	{
		// cleanup ndn::App
		ndn::App::StopApplication();
	}


	void
	icnVideoChunkingClient::Retransmit() {
		uint64_t current_time = get_time();

		if(current_time - THIS_CHUNK_REQ_TIME > 4500000) {
//			fprintf(stderr, "TIMEDOUT :(((((((((((((((((((((((\n");
		}
	}

	void
	icnVideoChunkingClient::SendInterest()
	{
		struct video *video;
		char interest_name[50];
		UniformVariable rand(0, std::numeric_limits<uint32_t>::max());

		if(this->helper.video_state.active == 1) {
			sprintf(interest_name, "/prefix/sub/video_%d/%d/%d",
					this->helper.video_state.video->index,
					this->helper.video_state.current_chunk,
					this->helper.video_state.current_chunk_offset);
		} else {
			video = this->helper.get_next_video();
			this->helper.new_video_started(video);
			sprintf(interest_name, "/prefix/sub/video_%d/%d/%d",
					this->helper.video_state.video->index,
					this->helper.video_state.current_chunk,
					this->helper.video_state.current_chunk_offset);
			this->helper.video_state.active = 1;
			STATE_VAR.frac_video = this->helper.video_access[video->index].frac_video;
			TOTAL_VIEWS++;
//			fprintf(stderr, "Watching video %d ", video->index);
		}

		STATE_VAR.last_pkt_req_time = get_time();
//		fprintf(stderr, "Client sent interest %s\n", interest_name);
		auto interest = std::make_shared<ndn::Interest>(interest_name);
		interest->setNonce(rand.GetValue());
		interest->setInterestLifetime(ndn::time::seconds(1));

		if(STATE_VAR.current_chunk_offset == 0) {
			/* Only if this is a new _chunk_ */
			THIS_CHUNK_REQ_TIME = get_time();
//			printf("Setting THIS_CHUNK_REQ_TIME = %Lu\n", THIS_CHUNK_REQ_TIME);
		}

		m_transmittedInterests(interest, this, m_face);

		m_face->onReceiveInterest(*interest);
	//	Simulator::Schedule(Seconds(5.0), &icnVideoChunkingClient::Retransmit, this);
	}


	/* This function would never be called */
	void icnVideoChunkingClient::OnInterest(std::shared_ptr<const ndn::Interest> interest) {
	}


	void icnVideoChunkingClient::dumpStats(void)
	{
		this->log_fp = fopen(this->log_file, "w");

		fprintf(this->log_fp, "%u, %lu, %lu, %lu\n",
				TOTAL_VIEWS, TOTAL_START_TIME, TOTAL_VIEW_TIME,
				TOTAL_BUFFERING_TIME);
		fclose(this->log_fp);
		Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::dumpStats, this);
	}

	int icnVideoChunkingClient::shouldQuitEarly(void)
	{
		// UniformVariable coin_toss(0, 1.0);

		// if(coin_toss.GetValue() <
		//    ((double)STATE_VAR.current_chunk / STATE_VAR.video->n_chunks - 0.5)) {
		// 	printf("Quitting early.[%lf][%lf]\n",
		// 		   coin_toss.GetValue(),
		// 		   (double)STATE_VAR.current_chunk / STATE_VAR.video->n_chunks - 0.5);
		// 	return 1;
		// }

		return 0;
	}

	void icnVideoChunkingClient::onChunk(void)
	{
		uint64_t buffering_time, current_time, extra_time, avg_rtt;

		if(STATE_VAR.current_chunk % 10 == 0) {
			// printf("[Client %u]\t%d/%d\n",
			// 	   this->client_id,
			// 	   STATE_VAR.current_chunk,
			// 	   STATE_VAR.video->n_chunks);
		}

		avg_rtt = STATE_VAR.this_chunk_total_rtt / STATE_VAR.num_pkts_in_this_chunk;
		extra_time = (STATE_VAR.num_pkts_in_this_chunk - 1) * (avg_rtt / 2);
		/* Buffering time for this chunk */
		current_time = get_time() - extra_time;
		// printf("[CHK_RECV] avg_rtt = %Lu\textra_time = %Lu\n",
		// 	   avg_rtt, extra_time);

		STATE_VAR.this_chunk_total_rtt = 0;
		STATE_VAR.num_pkts_in_this_chunk = 0;

		buffering_time = current_time - THIS_CHUNK_REQ_TIME;

		/* Calculate video start time */
		if(LAST_CHUNK_VIEW_TIME == 0) {
			TOTAL_START_TIME += buffering_time;
		}

//		printf("buffering = %Lu, Last view = %Lu\n", buffering_time, LAST_CHUNK_VIEW_TIME);
		/* Calculate buffering time */
		if(buffering_time > LAST_CHUNK_VIEW_TIME) {
			TOTAL_BUFFERING_TIME += buffering_time - LAST_CHUNK_VIEW_TIME;
		}

		LAST_CHUNK_VIEW_TIME = STATE_VAR.video->chunk_size * 30;
		/* ^^ Assumes constant 360p bitrate */
		/* Approx 5 minute ==> 10 Mb */
		/* 2 * 1024 * 1024 bytes ==> 60 * 1000 * 1000 us */

		TOTAL_VIEW_TIME += LAST_CHUNK_VIEW_TIME;
		if(CURRENT_OFFSET >= this->helper.video_state.video->size) {
			/* New video starts here: TODO Wait time distribution? */
#if 0
			fprintf(this->log_fp, "[DONE]\n");

			fprintf(this->log_fp, "%d, %Lu, %Lu, %Lu\n",
					this->helper.video_state.video->index,
					TOTAL_START_TIME, TOTAL_VIEW_TIME,
					TOTAL_BUFFERING_TIME);
#endif
			this->helper.video_state.active = 0;
			Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::SendInterest, this);
		} else {
			icnVideoChunkingClient::SendInterest();
//			Simulator::Schedule(Seconds(0.0), &icnVideoChunkingClient::SendInterest, this);
		}
	}

	/*
	 * When Data arrives: Lot of stuff to do!
	 */
	void
	icnVideoChunkingClient::OnData(std::shared_ptr<const ndn::Data> data)
	{

		int should_stop = 0;
		ndn::Block block = data->getContent();
//		std::cout << "Received " << data->getName() << std::endl;

		STATE_VAR.num_pkts_in_this_chunk++;
		STATE_VAR.this_chunk_total_rtt += get_time() - STATE_VAR.last_pkt_req_time;
		STATE_VAR.current_chunk_offset += block.value_size();

		// printf("[PKT_RECV] total_rtt = %Lu\tnum_pkts = %Lu\n",
		// 	   STATE_VAR.this_chunk_total_rtt,
		// 	   STATE_VAR.num_pkts_in_this_chunk);

		CURRENT_OFFSET += block.value_size();
		if(((float)CURRENT_OFFSET / STATE_VAR.video->size) > STATE_VAR.frac_video) 
			should_stop = 1;
		// printf("%f, %f, %d\n", ((float)CURRENT_OFFSET / STATE_VAR.video->size),
		// 	   STATE_VAR.frac_video,
		// 	   should_stop);

//		std::cout << CURRENT_OFFSET << "/" << STATE_VAR.video->size << std::endl;
		if(should_stop) {
			this->helper.video_state.active = 0;
			Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::SendInterest, this);
		} if(STATE_VAR.current_chunk_offset >= STATE_VAR.video->chunk_size) {
			// printf("Received Chunk %Lu, %Lu\n", STATE_VAR.current_chunk_offset,
			// 	   STATE_VAR.video->chunk_size);
			STATE_VAR.current_chunk++;
			STATE_VAR.current_chunk_offset = 0;
			icnVideoChunkingClient::onChunk();
		} else {
//			printf("Received packet\n");
			icnVideoChunkingClient::SendInterest();
//			Simulator::Schedule(Seconds(0.0), &icnVideoChunkingClient::SendInterest, this);
			return;
		}
	}
}
