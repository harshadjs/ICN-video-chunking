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

// custom-app.cpp

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "icn-chunking-client.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include <ndn-cxx/encoding/block.hpp>


#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE("icnVideoChunkingClient");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(icnVideoChunkingClient);

// register NS-3 type
TypeId
icnVideoChunkingClient::GetTypeId()
{
  static TypeId tid = TypeId("icnVideoChunkingClient").SetParent<ndn::App>().AddConstructor<icnVideoChunkingClient>();
  return tid;
}

// Processing upon start of the application
void
icnVideoChunkingClient::StartApplication()
{
  struct video *v;

  this->helper.video_access_dist = UNI;
  this->helper.video_size_dist = UNI;

  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);

  printf("Here\n");
  this->helper.read_video_file();
  v = this->helper.get_next_video();
  printf("Got video %d\n", v->index);
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
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////

  // Create and configure ndn::Interest
  struct video *video;
  char interest_name[50];
  UniformVariable rand(0, std::numeric_limits<uint32_t>::max());

  if(this->helper.video_state.active == 1) {
    sprintf(interest_name, "/prefix/sub/video_%d/%d",
            this->helper.video_state.video->index, this->helper.video_state.current_offset);
  } else {
    video = this->helper.get_next_video();
    this->helper.video_state.video = video;
    this->helper.video_state.current_offset = 0;
    sprintf(interest_name, "/prefix/sub/video_%d/%d",
            this->helper.video_state.video->index, this->helper.video_state.current_offset);
    this->helper.video_state.active = 1;
  }

  auto interest = std::make_shared<ndn::Interest>(interest_name);
  interest->setNonce(rand.GetValue());
  interest->setInterestLifetime(ndn::time::seconds(1));

  std::cout << "C:[Interest]\t-->" << *interest << std::endl;

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_face->onReceiveInterest(*interest);
}

// Callback that will be called when Interest arrives
void
icnVideoChunkingClient::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  //  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  // Note that Interests send out by the app will not be sent back to the app !

  auto data = std::make_shared<ndn::Data>(interest->getName());
  data->setFreshnessPeriod(ndn::time::milliseconds(1000));
  data->setContent(std::make_shared< ::ndn::Buffer>(1024));
  ndn::StackHelper::getKeyChain().sign(*data);

  //  NS_LOG_DEBUG("Sending Data packet for " << data->getName());

  // Call trace (for logging purposes)
  m_transmittedDatas(data, this, m_face);

  m_face->onReceiveData(*data);
}

// Callback that will be called when Data arrives
void
icnVideoChunkingClient::OnData(std::shared_ptr<const ndn::Data> data)
{
  ndn::Block block = data->getContent();
  //  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());
  std::cout << "C:[DATA]\t<--" << data->getName() << " Size:" << block.value_size() << std::endl;
  this->helper.video_state.current_offset += block.value_size();
  if(this->helper.video_state.current_offset >=
     this->helper.video_state.video->size) {
    /* New video starts here: TODO Wait time distribution? */
    printf("Finished watching this video.\n");
    this->helper.video_state.active = 0;
    Simulator::Schedule(Seconds(1.0), &icnVideoChunkingClient::SendInterest, this);
  } else {
    Simulator::Schedule(Seconds(0.0), &icnVideoChunkingClient::SendInterest, this);
  }
}

} // namespace ns3