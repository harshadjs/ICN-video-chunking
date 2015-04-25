
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

// hijacker.cpp

#include "server.hpp"
#include "ns3/log.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

NS_LOG_COMPONENT_DEFINE("icnVideoChunkingServer");

namespace ns3 {

// Necessary if you are planning to use ndn::AppHelper
NS_OBJECT_ENSURE_REGISTERED(icnVideoChunkingServer);

TypeId
icnVideoChunkingServer::GetTypeId()
{
  static TypeId tid = TypeId("icnVideoChunkingServer").SetParent<ndn::App>().AddConstructor<icnVideoChunkingServer>();

  return tid;
}

icnVideoChunkingServer::icnVideoChunkingServer()
{
}

void
icnVideoChunkingServer::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  struct video *video;
  // ndn::App::OnInterest(interest); // forward call to perform app-level tracing
  // // do nothing else (hijack interest)
  // NS_LOG_DEBUG("Do nothing for incoming interest for" << interest->getName());

  ndn::App::OnInterest(interest);

//  std::cout << "Producer received interest " << interest->getName() << std::endl;

  // Note that Interests send out by the app will not be sent back to the app !

  video = this->helper.lookup_video(interest->getName().toUri().c_str());

  auto data = std::make_shared<ndn::Data>(interest->getName());
  data->setFreshnessPeriod(ndn::time::seconds(1000));
  data->setContent(std::make_shared< ::ndn::Buffer>(1000));
  ndn::StackHelper::getKeyChain().sign(*data);

  // Call trace (for logging purposes)
  m_transmittedDatas(data, this, m_face);
  m_face->onReceiveData(*data);
}

void
icnVideoChunkingServer::StartApplication()
{
  App::StartApplication();

  // equivalent to setting interest filter for "/prefix" prefix
  ndn::FibHelper::AddRoute(GetNode(), "/prefix/sub", m_face, 0);
  //  printf("P: Registered route.\n");
  this->current_video = NULL;
  this->helper.read_video_file();
}

void
icnVideoChunkingServer::StopApplication()
{
  App::StopApplication();
}

} // namespace ns3
