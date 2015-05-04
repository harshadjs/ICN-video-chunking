#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3 {
	/*
	 * A video
	 */
	struct video {
		uint32_t size;			/* Video Size */
		uint32_t index;			/* Index of the video */
		uint32_t chunk_size;
		uint32_t n_chunks;
	};

	/*
	 * Statistics over entire runtime
	 */
	struct stats {
		uint64_t total_buffering_time;	/* Total buffering time */
		uint64_t total_start_time;		/* Total start time */
		uint64_t total_view_time;		/* Time spent in watching the video */
		uint32_t total_views;			/* Number of videos watched */
        uint32_t total_requests;        /* Number of interests sent */
        uint64_t total_response_time;   /* Time all of the interests took */
	};

	/*
	 * Dynamic video state:
	 * Refers to the video that is currently being watched
	 */
	struct video_state {
		uint8_t active;					/* Is the video being watched? */
		struct video *video;			/* Information about the video being
										 * watched right now */
		uint32_t current_offset;		/* Total bytes watched */
		uint64_t this_chunk_req_time;	/* Time when last chunk was requested */
		uint64_t last_chunk_view_time;	/* Total view time last chunk */
		uint32_t current_chunk;
		uint32_t current_chunk_offset;

		uint32_t nchunks;
		uint64_t this_chunk_total_rtt;
		uint64_t last_pkt_req_time;
		uint64_t num_pkts_in_this_chunk;
		float frac_video;
		/* uint8_t access_dist; */
	};

	struct video_access {
		int index;
		float frac_video;
	};

	enum {
		EXP,
		UNI,
		PARETO,
	};

	class icn_chunking_helper {
	public:
		/* Variables */
		struct video_state video_state;
		struct stats stats;
		int video_access_dist;
		int video_size_dist;
		struct video *video_list;
		struct video_access video_access[500];
		int next_access_index;
		int n_videos;
		int client_id;
		uint32_t chunk_size;

		int read_video_file(void);
		struct video *get_next_video(void);
		struct video *lookup_video(const char *name);
		void new_video_started(struct video *video);
		void set_client_id(int id);
	};

} // namespace ns3

