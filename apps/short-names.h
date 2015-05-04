#ifndef __ICN_STATS_H__
#define __ICN_STATS_H__

#define STATE_VAR this->helper.video_state
#define STATS_VAR this->helper.stats

/* All the video state varibles */
#define LAST_CHUNK_VIEW_TIME STATE_VAR.last_chunk_view_time
#define THIS_CHUNK_REQ_TIME STATE_VAR.this_chunk_req_time
#define CURRENT_OFFSET STATE_VAR.current_offset
#define NCHUNKS STATE_VAR.nchunks

/* All the total stats variables */
#define TOTAL_BUFFERING_TIME STATS_VAR.total_buffering_time
#define TOTAL_START_TIME STATS_VAR.total_start_time
#define TOTAL_VIEW_TIME STATS_VAR.total_view_time
#define TOTAL_VIEWS STATS_VAR.total_views
#define TOTAL_REQUESTS STATS_VAR.total_requests
#define TOTAL_RESPONSE_TIME STATS_VAR.total_response_time

#endif
