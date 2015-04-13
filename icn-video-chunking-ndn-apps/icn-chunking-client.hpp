/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

// custom-app.hpp

#ifndef CUSTOM_APP_H_
#define CUSTOM_APP_H_

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

  class icnVideoChunkingClient : public ndn::App {
  public:
    // register NS-3 type "icnVideoChunkingClient"
    static TypeId
    GetTypeId();

    // (overridden from ndn::App) Processing upon start of the application
    virtual void
    StartApplication();

    // (overridden from ndn::App) Processing when application is stopped
    virtual void
    StopApplication();

    // (overridden from ndn::App) Callback that will be called when Interest arrives
    virtual void
    OnInterest(std::shared_ptr<const ndn::Interest> interest);

    // (overridden from ndn::App) Callback that will be called when Data arrives
    virtual void
    OnData(std::shared_ptr<const ndn::Data> contentObject);

    int read_video_file(void);
    struct video *get_next_video(void);

  private:
    struct video_state video_state;
    int video_access_dist;
    int video_size_dist;
    struct video *video_list;
    int n_videos;
    int total_views;
    void
    SendInterest();
  };

} // namespace ns3

#endif // CUSTOM_APP_H_
