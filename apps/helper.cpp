#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "helper.hpp"
#include "paths.h"
#include "short-names.h"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include <ndn-cxx/encoding/block.hpp>

#include "ns3/random-variable.h"


namespace ns3 {

	void icn_chunking_helper::new_video_started(struct video *video) {
		memset(&this->video_state, 0, sizeof(this->video_state));
		this->video_state.video = video;
		this->video_state.v_watch_fraction = this->video_access[video->index].frac_video;
		this->video_state.v_size = video->size;
	}

	void icn_chunking_helper::video_stopped(void) {
		this->stats.total_buffering_time += this->video_state.v_buffer_time;
		this->stats.total_view_time += this->video_state.v_bytes_viewed * 30;
		this->stats.total_start_time += this->video_state.v_start_time;
	}

	void icn_chunking_helper::dump_state() {
		if(video_state.v_started == 1) {
			printf("[>] ");
		} else {
			printf("[-] ");
		}

		printf("Downloaded:\t%ld/%ld\t", video_state.v_bytes_downloaded,
			   video_state.v_size);
		printf("Viewed:\t%ld/%ld\t", video_state.v_bytes_viewed,
			   video_state.v_bytes_downloaded);
		printf("Buffering: %Lu\t", video_state.v_buffer_time);
		printf("Start: %Lu\t\n", video_state.v_start_time);
	}

	struct video *icn_chunking_helper::get_next_video(void)
	{
		return &this->video_list[this->video_access[(this->next_access_index++) % 500].index];

// 		long unsigned val;
// 		int i, passed;
// 		struct video *video;

// 		val = random() % this->total_views;

//  	printf("val = %lu, this->total_views = %lu\n", val, this->total_views);
// 		passed = 0;
// 		for(i = 0; i < this->n_videos; i++) {
// 			video = &this->video_list[i];
// 			passed += video->popularity;
// 			if(val < passed) {
// 				return video;
// 			}
// 		}
	}

	struct video *icn_chunking_helper::lookup_video(const char *name)
	{
		int i;
		struct video *video;
		uint32_t index = strtol(name, NULL, 10);

		for(i = 0; i < this->n_videos; i++) {
			video = &this->video_list[i];
			if(video->index == index) {
				return video;
			}
		}

		return video;
	}

	void icn_chunking_helper::set_client_id(int id)
	{
		this->client_id = id;
	}

	int icn_chunking_helper::read_video_file(void)
	{
		FILE *fp = fopen(VIDEOS_CONF, "r");
		FILE *fp_chunk = fopen(CHUNK_CONF, "r");
		FILE *fp_access;
		char filename[256];
		float frac_video;
		int chunk_size, access;
		int index, dummy, size, count = 1;

		this->video_list = NULL;
		this->n_videos = 0;

		fscanf(fp_chunk, "%d", &chunk_size);
		fclose(fp_chunk);

		while(!feof(fp)) {
			if(fscanf(fp, "%d,%d,%d\n", &index, &size, &dummy) == 3) {
				//     printf("video_%d, %d, %d, %d\n", count, popularity, access, size);
				//printf("this->n_videos = %d\n", this->n_videos);
				this->video_list = (struct video *)
					realloc((void *)this->video_list,
							(this->n_videos + 1) * sizeof(struct video));

				this->video_list[this->n_videos].index = count;
				this->video_list[this->n_videos].size = size;
				this->video_list[this->n_videos].chunk_size = chunk_size; /* TODO */
				this->video_list[this->n_videos].n_chunks = size/chunk_size; /* TODO */
				this->n_videos++;
				count++;
			}
		}
		fclose(fp);

		count = 0;
		printf("Reading client-%d.trace\n", this->client_id);
		sprintf(filename, "%s/client-%d.trace", APPS_PATH, this->client_id);
		fp_access = fopen(filename, "r");
		while(!feof(fp_access)) {
			if(fscanf(fp_access, "%d,%f", &access, &frac_video) != 2)
				continue;
			printf("frac = %f\n", frac_video);
			this->video_access[count].index = access;
			this->video_access[count].frac_video = frac_video;
			count++;
		}
		fclose(fp_access);

		this->next_access_index = 0;

		srandom((unsigned int)clock());
		return 0;
	}

} // namespace ns3
