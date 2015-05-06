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
		struct video *video;			/* Information about the video being
										 * watched right now */
		uint8_t v_started;
		uint8_t v_active;

		uint32_t v_bytes_downloaded;
		uint32_t v_size;
		uint32_t v_bytes_viewed;
		uint32_t v_buffer;
		uint32_t v_next_packet;

		uint64_t v_buffer_time;
		uint64_t v_download_start_time;
		uint64_t v_start_time;
		uint64_t v_view_last_noted;

		float v_watch_fraction;
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
		void video_stopped(void);
		void dump_state(void);
		void set_client_id(int id);
	};

} // namespace ns3

