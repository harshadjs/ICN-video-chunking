#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "helper.hpp"
#include "paths.h"

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
	}

	struct video *icn_chunking_helper::get_next_video(void)
	{
		long unsigned val;
		int i, passed;
		struct video *video;

		val = random() % this->total_views;

//		printf("val = %lu, this->total_views = %lu\n", val, this->total_views);
		passed = 0;
		for(i = 0; i < this->n_videos; i++) {
			video = &this->video_list[i];
			passed += video->popularity;
			if(val < passed) {
				return video;
			}
		}

		return video;
	}

	struct video *icn_chunking_helper::lookup_video(const char *name)
	{
		int i;
		struct video *video;
		int index = strtol(name, NULL, 10);

		for(i = 0; i < this->n_videos; i++) {
			video = &this->video_list[i];
			if(video->index == index) {
				return video;
			}
		}

		return video;
	}

	int icn_chunking_helper::read_video_file(void)
	{
		FILE *fp = fopen(VIDEOS_CONF, "r");
		FILE *fp_chunk = fopen(CHUNK_CONF, "r");
		int chunk_size;
		int popularity, access, size, count = 1;

		this->video_list = NULL;
		this->n_videos = 0;
		this->total_views = 0;

		fscanf(fp_chunk, "%d", &chunk_size);
		fclose(fp_chunk);

		while(!feof(fp)) {
			if(fscanf(fp, "%d,%d,%d\n", &popularity, &access, &size) == 3) {
				//     printf("video_%d, %d, %d, %d\n", count, popularity, access, size);
				//printf("this->n_videos = %d\n", this->n_videos);
				this->video_list = (struct video *)
					realloc((void *)this->video_list,
							(this->n_videos + 1) * sizeof(struct video));

				this->video_list[this->n_videos].index = count;
				this->video_list[this->n_videos].popularity = popularity;
				this->video_list[this->n_videos].access = access;
				this->video_list[this->n_videos].size = size;
				this->video_list[this->n_videos].chunk_size = chunk_size; /* TODO */
				this->video_list[this->n_videos].n_chunks = size/chunk_size; /* TODO */
				this->total_views += popularity;
				this->n_videos++;
				count++;
			}
		}
		fclose(fp);

		srandom((unsigned int)clock());

		return 0;
	}

} // namespace ns3
