#include <stdint.h>
#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3 {

  struct video {
    uint32_t popularity;
    uint32_t access;
    uint32_t size;
    uint32_t index;
  };

  struct video_state {
    uint8_t active;
    struct video *video;
    uint32_t current_offset;
    /* uint8_t access_dist; */
  };

  /**
   * @brief A simple custom application
   *
   * This applications demonstrates how to send Interests and respond with Datas to incoming interests
   *
   * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
   * sends Interest for this prefix
   *
   * When an Interest is received, it is replied with a Data with 1024-byte fake payload
   */
  enum {
    EXP,
    UNI,
    PARETO,
  };

	class icn_chunking_helper {
	public:
      int read_video_file(void);
      struct video *get_next_video(void);
      struct video *lookup_video(const char *name);

      struct video_state video_state;
      int video_access_dist;
      int video_size_dist;
      struct video *video_list;
      int n_videos;
      int total_views;
	};

} // namespace ns3

