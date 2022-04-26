/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
 *               2016 Stefano Avallone <stavallo@unina.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "traffic-control-layer.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/log.h"
#include "ns3/object-map.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/queue-disc.h"
#include <tuple>
#include "ns3/core-module.h"
#include "ns3/address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TrafficControlLayer");

NS_OBJECT_ENSURE_REGISTERED (TrafficControlLayer);

TypeId
TrafficControlLayer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TrafficControlLayer")
    .SetParent<Object> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<TrafficControlLayer> ()
    .AddAttribute ("RootQueueDiscList", "The list of root queue discs associated to this Traffic Control layer.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&TrafficControlLayer::GetNDevices,
                                          &TrafficControlLayer::GetRootQueueDiscOnDeviceByIndex),
                   MakeObjectMapChecker<QueueDisc> ())
  ;
  return tid;
}

TypeId
TrafficControlLayer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

TrafficControlLayer::TrafficControlLayer ()
  : Object ()
{
  NS_LOG_FUNCTION (this);
}

TrafficControlLayer::~TrafficControlLayer ()
{
  NS_LOG_FUNCTION (this);
}

void
TrafficControlLayer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_handlers.clear ();
  m_netDevices.clear ();
  Object::DoDispose ();
}

void
TrafficControlLayer::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  ScanDevices ();

  // initialize the root queue discs
  for (auto& ndi : m_netDevices)
    {
      if (ndi.second.m_rootQueueDisc)
        {
          ndi.second.m_rootQueueDisc->Initialize ();
        }
    }

  Object::DoInitialize ();
}

void
TrafficControlLayer::RegisterProtocolHandler (Node::ProtocolHandler handler,
                                              uint16_t protocolType, Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << protocolType << device);

  struct ProtocolHandlerEntry entry;
  entry.handler = handler;
  entry.protocol = protocolType;
  entry.device = device;
  entry.promiscuous = false;

  m_handlers.push_back (entry);

  NS_LOG_DEBUG ("Handler for NetDevice: " << device << " registered for protocol " <<
                protocolType << ".");
}

void
TrafficControlLayer::ScanDevices (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_node, "Cannot run ScanDevices without an aggregated node");

  NS_LOG_DEBUG ("Scanning devices on node " << m_node->GetId ());
  for (uint32_t i = 0; i < m_node->GetNDevices (); i++)
    {
      NS_LOG_DEBUG ("Scanning devices on node " << m_node->GetId ());
      Ptr<NetDevice> dev = m_node->GetDevice (i);
      NS_LOG_DEBUG ("Checking device " << i << " with pointer " << dev << " of type " << dev->GetInstanceTypeId ().GetName ());

      // note: there may be no NetDeviceQueueInterface aggregated to the device
      Ptr<NetDeviceQueueInterface> ndqi = dev->GetObject<NetDeviceQueueInterface> ();
      NS_LOG_DEBUG ("Pointer to NetDeviceQueueInterface: " << ndqi);

      std::map<Ptr<NetDevice>, NetDeviceInfo>::iterator ndi = m_netDevices.find (dev);

      if (ndi != m_netDevices.end ())
        {
          NS_LOG_DEBUG ("Device entry found; installing NetDeviceQueueInterface pointer " << ndqi << " to internal map");
          ndi->second.m_ndqi = ndqi;
        }
      else if (ndqi)
      // if no entry for the device is found, it means that no queue disc has been
      // installed. Nonetheless, create an entry for the device and store a pointer
      // to the NetDeviceQueueInterface object if the latter is not null, because
      // the Traffic Control layer checks whether the device queue is stopped even
      // when there is no queue disc.
        {
          NS_LOG_DEBUG ("No device entry found; create entry for device and store pointer to NetDeviceQueueInterface: " << ndqi);
          m_netDevices[dev] = {nullptr, ndqi, QueueDiscVector ()};
          ndi = m_netDevices.find (dev);
        }

      // if a queue disc is installed, set the wake callbacks on netdevice queues
      if (ndi != m_netDevices.end () && ndi->second.m_rootQueueDisc)
        {
          NS_LOG_DEBUG ("Setting the wake callbacks on NetDevice queues");
          ndi->second.m_queueDiscsToWake.clear ();

          if (ndqi)
            {
              for (uint16_t i = 0; i < ndqi->GetNTxQueues (); i++)
                {
                  Ptr<QueueDisc> qd;

                  if (ndi->second.m_rootQueueDisc->GetWakeMode () == QueueDisc::WAKE_ROOT)
                    {
                      qd = ndi->second.m_rootQueueDisc;
                    }
                  else if (ndi->second.m_rootQueueDisc->GetWakeMode () == QueueDisc::WAKE_CHILD)
                    {
                      NS_ABORT_MSG_IF (ndi->second.m_rootQueueDisc->GetNQueueDiscClasses () != ndqi->GetNTxQueues (),
                                      "The number of child queue discs does not match the number of netdevice queues");

                      qd = ndi->second.m_rootQueueDisc->GetQueueDiscClass (i)->GetQueueDisc ();
                    }
                  else
                    {
                      NS_ABORT_MSG ("Invalid wake mode");
                    }

                  ndqi->GetTxQueue (i)->SetWakeCallback (MakeCallback (&QueueDisc::Run, qd));
                  ndi->second.m_queueDiscsToWake.push_back (qd);
                }
            }
          else
            {
              ndi->second.m_queueDiscsToWake.push_back (ndi->second.m_rootQueueDisc);
            }

          // set the NetDeviceQueueInterface object and the SendCallback on the queue discs
          // into which packets are enqueued and dequeued by calling Run
          for (auto& q : ndi->second.m_queueDiscsToWake)
            {
              q->SetNetDeviceQueueInterface (ndqi);
              q->SetSendCallback ([dev] (Ptr<QueueDiscItem> item)
                                  { dev->Send (item->GetPacket (), item->GetAddress (), item->GetProtocol ()); });
            }
        }
    }
}

void
TrafficControlLayer::SetRootQueueDiscOnDevice (Ptr<NetDevice> device, Ptr<QueueDisc> qDisc)
{
  NS_LOG_FUNCTION (this << device << qDisc);

  std::map<Ptr<NetDevice>, NetDeviceInfo>::iterator ndi = m_netDevices.find (device);

  if (ndi == m_netDevices.end ())
    {
      // No entry found for this device. Create one.
      m_netDevices[device] = {qDisc, nullptr, QueueDiscVector ()};
    }
  else
    {
      NS_ABORT_MSG_IF (ndi->second.m_rootQueueDisc,
                       "Cannot install a root queue disc on a device already having one. "
                       "Delete the existing queue disc first.");

      ndi->second.m_rootQueueDisc = qDisc;
    }
}

Ptr<QueueDisc>
TrafficControlLayer::GetRootQueueDiscOnDevice (Ptr<NetDevice> device) const
{
  NS_LOG_FUNCTION (this << device);

  std::map<Ptr<NetDevice>, NetDeviceInfo>::const_iterator ndi = m_netDevices.find (device);

  if (ndi == m_netDevices.end ())
    {
      return 0;
    }
  return ndi->second.m_rootQueueDisc;
}

Ptr<QueueDisc>
TrafficControlLayer::GetRootQueueDiscOnDeviceByIndex (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  return GetRootQueueDiscOnDevice (m_node->GetDevice (index));
}

void
TrafficControlLayer::DeleteRootQueueDiscOnDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);

  std::map<Ptr<NetDevice>, NetDeviceInfo>::iterator ndi = m_netDevices.find (device);

  NS_ASSERT_MSG (ndi != m_netDevices.end () && ndi->second.m_rootQueueDisc != 0,
                 "No root queue disc installed on device " << device);

  // remove the root queue disc
  ndi->second.m_rootQueueDisc = 0;
  for (auto& q : ndi->second.m_queueDiscsToWake)
    {
      q->SetNetDeviceQueueInterface (nullptr);
      q->SetSendCallback (nullptr);
    }
  ndi->second.m_queueDiscsToWake.clear ();

  Ptr<NetDeviceQueueInterface> ndqi = ndi->second.m_ndqi;
  if (ndqi)
    {
      // remove configured callbacks, if any
      for (uint16_t i = 0; i < ndqi->GetNTxQueues (); i++)
        {
          ndqi->GetTxQueue (i)->SetWakeCallback (MakeNullCallback <void> ());
        }
    }
  else
    {
      // remove the empty entry
      m_netDevices.erase (ndi);
    }
}

void
TrafficControlLayer::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
  elapsedTimeSinceLastICD = Simulator::Now();
}

void
TrafficControlLayer::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION (this);
  if (m_node == 0)
    {
      Ptr<Node> node = this->GetObject<Node> ();
      //verify that it's a valid node and that
      //the node was not set before
      if (node != 0)
        {
          this->SetNode (node);
        }
    }
  Object::NotifyNewAggregate ();
}

uint32_t
TrafficControlLayer::GetNDevices (void) const
{
  return m_node->GetNDevices ();
}


void
TrafficControlLayer::Receive (Ptr<NetDevice> device, Ptr<const Packet> p,
                              uint16_t protocol, const Address &from, const Address &to,
                              NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << device << p << protocol << from << to << packetType);

  /*--------------------------- my changes starts ---------------------------*/

  std::cout<<"node="<<m_node->GetId()
          << ": receive traffic: from:"<< from<<"," << m_node->GetNodeOption()->GetParentAddress()<<"\n";



    Ptr<NodeOption> option = m_node->GetPrevNode()->GetNodeOption();

    std::cout<<"node="<<m_node->GetId()<<": rec entr time: "<<Simulator::Now().GetSeconds()<<"\t"<<m_node->GetPrevNode()->entrTimeInMac<<"\n";

    int diff = Simulator::Now().GetMicroSeconds() - m_node->GetPrevNode()->entrTimeInMac;
    double newTs = 0.9 * option->GetTs() + 0.1 * diff / 100.0;
    //std::cout<<"new Ts: " << newTs<<", curTs="<<option->GetTs()<<", diff="<<diff <<"\n";
    option->SetTs(newTs);
    std::cout<<"node="<<m_node->GetId()<<": updating Ts="<< option->GetTs()<<"\n";

  /*--------------------------- my changes ends ---------------------------*/


  bool found = false;

  for (ProtocolHandlerList::iterator i = m_handlers.begin ();
       i != m_handlers.end (); i++)
    {
      if (i->device == 0
          || (i->device != 0 && i->device == device))
        {
          if (i->protocol == 0
              || i->protocol == protocol)
            {
              NS_LOG_DEBUG ("Found handler for packet " << p << ", protocol " <<
                            protocol << " and NetDevice " << device <<
                            ". Send packet up");
              i->handler (device, p, protocol, from, to, packetType);
              found = true;
            }
        }
    }

  NS_ABORT_MSG_IF (!found, "Handler for protocol " << p << " and device " << device <<
                           " not found. It isn't forwarded up; it dies here.");
}

void
TrafficControlLayer::Send (Ptr<NetDevice> device, Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << device << item);

  /*--------------------------- my changes starts ---------------------------*/

  Ptr<NodeOption> option = m_node->GetNodeOption();

  m_node->entrTimeInMac = Simulator::Now().GetMicroSeconds();

  std::cout<<"node="<<m_node->GetId()<<": entr time: "<<Simulator::Now().GetSeconds()<<"\t"<<m_node->entrTimeInMac<<"\n";

  //Time after = Simulator::Now();
  // double curTs = option->GetTs();
  double curTa = option->GetTa();


  //if(option->GetPktsSinceLastICD() % 2 == 1){
    Time curTime = Simulator::Now();
    int elapsedTime = (curTime - elapsedTimeSinceLastICD).GetMilliSeconds();
    option->SetTa(0.9 * curTa + 0.1 * elapsedTime / 100.0);
    option->SetPktsSinceLastICD(option->GetPktsSinceLastICD() + 1);
    std::cout<<"node="<<m_node->GetId()<<": updating Ta="<< option->GetTa()<<"\n";
    elapsedTimeSinceLastICD = curTime;
  //}


  /*--------------------------- my changes ends ---------------------------*/


  NS_LOG_DEBUG ("Send packet to device " << device << " protocol number " <<
                item->GetProtocol ());

  Ptr<NetDeviceQueueInterface> devQueueIface;
  std::map<Ptr<NetDevice>, NetDeviceInfo>::iterator ndi = m_netDevices.find (device);

  if (ndi != m_netDevices.end ())
  {
    devQueueIface = ndi->second.m_ndqi;
  }

  // determine the transmission queue of the device where the packet will be enqueued
  std::size_t txq = 0;
  if (devQueueIface && devQueueIface->GetNTxQueues () > 1)
    {
      txq = devQueueIface->GetSelectQueueCallback () (item);
      // otherwise, Linux determines the queue index by using a hash function
      // and associates such index to the socket which the packet belongs to,
      // so that subsequent packets of the same socket will be mapped to the
      // same tx queue (__netdev_pick_tx function in net/core/dev.c). It is
      // pointless to implement this in ns-3 because currently the multi-queue
      // devices provide a select queue callback
    }

  NS_ASSERT (!devQueueIface || txq < devQueueIface->GetNTxQueues ());


  /*--------------------------- my changes starts ---------------------------*/

  if(device->ipAddr == option->GetParentAddress()){
    std::cout<<"node="<<m_node->GetId()<< ": Upstream traffic " << ", " << option->GetParentAddress()<< "\n";

    double r_src = SrcRate();
    std::cout<<"node="<<m_node->GetId()<< ": src rate="<< r_src << "\n";

    if(r_src > 0){

      Ptr<NodeOption> parentOption = m_node->GetParentNode()->GetNodeOption();
      double degree = parentOption->GetTs() * 1.0 / parentOption->GetTa();

      if(degree >= 3.0){
        std::cout<<"node="<<m_node->GetId()<<": About to enqueue. degree="<< degree <<"\n";
        pccpQueue.push(item);
        return;
      }

    }
    else{
      std::cout<< "No congestion\n";
    }

  }
  else{
    std::cout<<"node="<<m_node->GetId()<< ": Not Upstream traffic: " << device->ipAddr << ", " << option->GetParentAddress() << "\n";
  } 


  if(!pccpQueue.empty()){
    pccpQueue.push(item);
    std::cout<<"node="<<m_node->GetId()<<" queue size="<<pccpQueue.size() <<"\n";
    Ptr<QueueDiscItem> curItem = pccpQueue.front();
    pccpQueue.pop();
    item = curItem;
  }


  /*--------------------------- my changes ends ---------------------------*/


  if (ndi == m_netDevices.end () || ndi->second.m_rootQueueDisc == 0)
    {
      // The device has no attached queue disc, thus add the header to the packet and
      // send it directly to the device if the selected queue is not stopped
      if (!devQueueIface || !devQueueIface->GetTxQueue (txq)->IsStopped ())
        {
          item->AddHeader ();
          // a single queue device makes no use of the priority tag
          if (!devQueueIface || devQueueIface->GetNTxQueues () == 1)
            {
              SocketPriorityTag priorityTag;
              item->GetPacket ()->RemovePacketTag (priorityTag);
            }
          device->Send (item->GetPacket (), item->GetAddress (), item->GetProtocol ());

          /*--------------------------- my changes starts ---------------------------*/

          // after = Simulator::Now();
          // Time diff = after - pktServiceStart;
          // option->SetTs(0.9 * curTs + 0.1 * diff.GetSeconds());
          // std::cout<<"updating Ta="<< option->GetTs()<<"\n";

          /*--------------------------- my changes ends ---------------------------*/
        }
    }
  else
    {
      // Enqueue the packet in the queue disc associated with the netdevice queue
      // selected for the packet and try to dequeue packets from such queue disc
      item->SetTxQueueIndex (txq);

      Ptr<QueueDisc> qDisc = ndi->second.m_queueDiscsToWake[txq];
      NS_ASSERT (qDisc);
      bool isQueued = qDisc->Enqueue (item);

      if(isQueued == false){
        std::cout<<"Packet is dropped\n";
      }
      else{
        std::cout<<"node="<<m_node->GetId()<<": Packet is enqueued\n";
      }

      qDisc->Run ();

      /*--------------------------- my changes starts ---------------------------*/

      //if(qDisc->isTransmitted){
        //std::cout<<"Packet is dequeued and transmitted\n";
        //after = Simulator::Now();
        //Time diff = after - before;


        // double newTs = 0.9 * curTs + 0.1 * 0.01;
        // std::cout<<"new Ts: " << newTs<<", curTs="<<curTs<<"\n";
        // option->SetTs(newTs);
        // std::cout<<"node="<<m_node->GetId()<<": updating Ts="<< option->GetTs()<<"\n";

        //qDisc->isTransmitted = false;        
      //}

      /*--------------------------- my changes ends ---------------------------*/
    }
}



/*--------------------------- my changes starts ---------------------------*/

// void TrafficControlLayer::InitializePRA(){

// }
double TrafficControlLayer::ScheduleRate(){

  Ptr<NodeOption> option = m_node->GetNodeOption();
  Ptr<Node> parent = m_node->GetParentNode();
  Ptr<NodeOption> parentOption = parent->GetNodeOption();

  double curTs = parentOption->GetTs();
  double curTa = parentOption->GetTa();

  double degree = curTs/ curTa;

  if(degree < 1.0){
    return -1.0;
  }
  else if(degree > 1.0){
    std::cout<<"node="<<m_node->GetId()<< ": congestion has occured. curTs="<<curTs<<", curTa="<<curTa<<", degree = " << degree <<"\n";
  }

  double total_rate = 1.0 / curTs;
  double r_svc = option->GetSvc();

  if(parentOption->GetOffsprings() < option->GetParentOffsprings()){
    std::cout<<"option 1\n";
    r_svc = r_svc * 1.0 / degree;
  }
  else if(parentOption->GetOffsprings() > option->GetParentOffsprings()){
    std::cout<<"option 2\n";
    r_svc = total_rate * (option->GetGP() * 1.0 / parentOption->GetGP());
  }
  else{
    if(degree <= option->GetParentDegree()){
      std::cout<<"option 3\n";
      r_svc = r_svc * 1.0 / degree;
    }
    else{
      std::cout<<"option 4\n";
      r_svc = total_rate * (option->GetGP() * 1.0 / parentOption->GetGP());
    }
  }
  option->SetParentDegree(degree);  
  option->SetParentOffsprings(parentOption->GetOffsprings());

  if(r_svc > (1.0 / option->GetTs())){
    r_svc = 1.0 / option->GetTs();
  }

  return r_svc * 0.98;
}

double TrafficControlLayer::SrcRate(){
  double r_svc = ScheduleRate();
  std::cout<<"node="<<m_node->GetId()<<": schedule rate: " << r_svc << "\n";
  double r_src = r_svc * (m_node->GetNodeOption()->GetSP() * 1.0 / m_node->GetNodeOption()->GetGP());
  m_node->GetNodeOption()->SetSvc(r_src);
  return r_src;
}

/*--------------------------- my changes ends ---------------------------*/



} // namespace ns3