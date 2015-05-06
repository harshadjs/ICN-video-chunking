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

#define START_THRESHOLD (0.1f)

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
		SendInterestBatch(16384);
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

		Simulator::Schedule(Seconds(0.0), &icnVideoChunkingClient::SendInterest, this);
// 		}
	}

	void
	icnVideoChunkingClient::SendInterest()
	{
		struct video *video;
		char interest_name[50];
		uint64_t current_time = get_time();
		UniformVariable rand(0, std::numeric_limits<uint32_t>::max());

		if(this->helper.video_state.v_active == 1) {
			sprintf(interest_name, "/prefix/sub/video_%d/%d",
					this->helper.video_state.video->index,
					STATE_VAR.v_next_packet);
//			printf("[REQ] %s\n", interest_name);
		} else {
			video = this->helper.get_next_video();
			this->helper.new_video_started(video);
			sprintf(interest_name, "/prefix/sub/video_%d/%d",
					this->helper.video_state.video->index,
					STATE_VAR.v_next_packet);
//			printf("[REQ] %s\n", interest_name);
			this->helper.video_state.v_active = 1;

			STATE_VAR.v_buffer = 16384;
			STATE_VAR.v_download_start_time = current_time;
			STATS_VAR.total_views++;
		}

		auto interest = std::make_shared<ndn::Interest>(interest_name);
		interest->setNonce(rand.GetValue());
		interest->setInterestLifetime(ndn::time::seconds(1));

		m_transmittedInterests(interest, this, m_face);

		m_face->onReceiveInterest(*interest);
	}


	/* This function would never be called */
	void icnVideoChunkingClient::OnInterest(std::shared_ptr<const ndn::Interest> interest)
	{
	}


	void icnVideoChunkingClient::dumpStats(void)
	{
		this->log_fp = fopen(this->log_file, "w");

//		printf("Dumping to %s\n", this->log_file);
		fprintf(this->log_fp, "%u, %lu, %lu, %lu\n",
				STATS_VAR.total_views - 1,
				STATS_VAR.total_start_time,
				STATS_VAR.total_view_time,
				STATS_VAR.total_buffering_time);
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

	void
	icnVideoChunkingClient::SendInterestBatch(uint32_t bytes_to_download)
	{
		STATE_VAR.v_buffer += bytes_to_download;
//		printf("1 buffer = %d\n", STATE_VAR.v_buffer);
		while(bytes_to_download > 0) {
			this->SendInterest();
			bytes_to_download = bytes_to_download - this->helper.chunk_size;
//			printf("bytes_to_download = %ld\n", bytes_to_download);
			STATE_VAR.v_next_packet += this->helper.chunk_size;
		}
	}

	/*
	 * When Data arrives: Lot of stuff to do!
	 */
	void
	icnVideoChunkingClient::OnData(std::shared_ptr<const ndn::Data> data)
	{
#define FRACTION(__num, __denom) (((double)__num) / (__denom))
#define TIME_TO_BYTES(__us) ((__us) / 30)
#define BYTES_TO_TIME(__bytes) ((__bytes) * 30)

		ndn::Block block = data->getContent();
		uint32_t expected_view_time;
		uint32_t current_time = get_time();

		if(STATE_VAR.v_started) {
			/*
			 * In the ideal world where there was no video buffering, we would
			 * have watched these many bytes (expected_view_time)
			 */
			expected_view_time = current_time - STATE_VAR.v_view_last_noted;

			if(STATE_VAR.v_bytes_viewed + TIME_TO_BYTES(expected_view_time)
			   < STATE_VAR.v_bytes_downloaded) {
				/* We had as many bytes as needed to watch expected_view_time */
				STATE_VAR.v_bytes_viewed += TIME_TO_BYTES(expected_view_time);
			} else {
				STATE_VAR.v_buffer_time += expected_view_time -
					BYTES_TO_TIME(STATE_VAR.v_bytes_downloaded - STATE_VAR.v_bytes_viewed);
				STATE_VAR.v_bytes_viewed = STATE_VAR.v_bytes_downloaded;
			}

			STATE_VAR.v_bytes_downloaded += block.value_size();
			STATE_VAR.v_view_last_noted = current_time;
		} else {
			/* The video has not started yet */
			STATE_VAR.v_bytes_downloaded += block.value_size();

			/* Check if we have enough to start the video */
			if(FRACTION(STATE_VAR.v_bytes_downloaded, STATE_VAR.v_size) > START_THRESHOLD) {
				STATE_VAR.v_start_time = current_time - STATE_VAR.v_download_start_time;
				STATE_VAR.v_started = 1;
				STATE_VAR.v_view_last_noted = current_time;
				STATE_VAR.v_bytes_viewed = 0;
			}
		}

		if((FRACTION(STATE_VAR.v_bytes_viewed, STATE_VAR.v_size)
			>= STATE_VAR.v_watch_fraction)
			|| (STATE_VAR.v_bytes_downloaded >= STATE_VAR.v_size)) {
			printf("Finished\n");
			/*
			 * Video stopped
			 * Take a break and watch another video after 1 second
			 */
			this->helper.video_stopped();
			STATE_VAR.v_active = 0;
			icnVideoChunkingClient::SendInterestBatch(16384);
		} else {
//			printf("2 buffer = %d\n", STATE_VAR.v_buffer);
			STATE_VAR.v_buffer -= block.value_size();
//			printf("3 buffer = %ld\n", STATE_VAR.v_buffer);
			if(STATE_VAR.v_buffer < 16384) {
				icnVideoChunkingClient::SendInterestBatch(16384 - STATE_VAR.v_buffer);
			}
		}

		// printf("[%Lu] ", current_time);
		// this->helper.dump_state();
	}
}
