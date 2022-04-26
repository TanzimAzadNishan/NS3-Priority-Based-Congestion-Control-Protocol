#include <iostream>
#include <cmath>
#include <string>
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/applications-module.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include <vector>
#include<time.h>
#include "ns3/yans-error-rate-model.h"
#include "ns3/wifi-net-device.h"
#include <fstream>
#include "ns3/nist-error-rate-model.h"
#include "ns3/yans-wifi-phy.h"
#include <unistd.h>


NS_LOG_COMPONENT_DEFINE ("tcp-test");

using namespace ns3;


/*
Default Network Topology

  Wifi 10.1.1.0                                            Wifi 10.1.3.0                  
    *   *   *                  *                           *    *    *    *
    |   |   |    10.1.4.0      |          10.1.5.0         |    |    |    |
    n4  n3  n0 --------------  n1  ----------------------  n2   n7   n8   n9    
                  p2p                         p2p
                               n5 - *
                              
                               n6 - *  
                              
                          Wifi 10.1.2.0
*/



uint64_t lastTotalRx = 0;
double simulationTime;


class SocketApp : public Application 
{
public:

  SocketApp ();
  virtual ~SocketApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
  void StartApplication (void);
  void StopApplication (void);

private:

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

SocketApp::SocketApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

SocketApp::~SocketApp()
{
  m_socket = 0;
}

void
SocketApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
SocketApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void 
SocketApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
SocketApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void 
SocketApp::ScheduleTx (void)
{
  if (m_running)
    {
      //std::cout << "time " << Simulator::Now().GetSeconds () << " s ...\n";
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &SocketApp::SendPacket, this);
    }
}




class TcpApp : public Application 
{
public:

  TcpApp ();
  void SetTcpVariant();

  uint32_t payloadSize;                       
  std::string tcpVariant;
  std::string phyRate; 
  bool pcapTracing;

private:
};

TcpApp::TcpApp()
  : payloadSize(1472),
    phyRate("HtMcs7"),
    pcapTracing(true) 
{}


void TcpApp::SetTcpVariant(){
  tcpVariant = std::string ("ns3::") + tcpVariant;
  if (tcpVariant.compare ("ns3::TcpWestwoodPlus") == 0)
  {
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
    Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
  }
  else
  {
    TypeId tcpTid;
    NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " 
            << tcpVariant << " not found");
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
  }
}




int noOfFlows;
int noOfPacketsPerSec;
int coverageArea;

uint32_t noOfNodes;
int noOfNetworks;  

NodeContainer* nodes;
NodeContainer* apNodes;
NetDeviceContainer* staDevices;
NetDeviceContainer* apDevices;
Ipv4InterfaceContainer* staInterfaces;
Ipv4InterfaceContainer* apInterfaces;

ApplicationContainer sinkApps;
ApplicationContainer serverApp;





class TcpTest : public Application 
{
public:
  TcpTest ();
  bool Configure (int argc, char **argv, TcpApp tcpApp);
  void Run(TcpApp tcpApp);
  void setNoOfNodes(uint32_t nodes);
  void setNoOfFlows(int flows);
  void setNoOfNetworks(int networks);
  void setErrorModel(Ptr<NetDevice> destDevice);

  // int noOfFlows;
  // int noOfPacketsPerSec;
  // int coverageArea;

private:

  // uint32_t noOfNodes;
  // int noOfNetworks;  

  // NodeContainer* nodes;
  // NodeContainer* apNodes;
  // NetDeviceContainer* staDevices;
  // NetDeviceContainer* apDevices;
  // Ipv4InterfaceContainer* staInterfaces;
  // Ipv4InterfaceContainer* apInterfaces;

  // ApplicationContainer sinkApps;
  // ApplicationContainer serverApp;

private:
  void CreateNodes ();
  void ApplyMobility();
  void CreateDevices (TcpApp tcpApp);
  void InstallInternetStack ();
  //void InstallApplications (int i);
  void SetNodeOptions();
};

//-----------------------------------------------------------------------------
TcpTest::TcpTest ()
{}


void TcpTest:: setNoOfNodes(uint32_t nodes){
  noOfNodes = nodes;
}

void TcpTest:: setNoOfFlows(int flows){
  noOfFlows = flows;
}

void TcpTest:: setNoOfNetworks(int networks){
  noOfNetworks = networks;
  nodes = new NodeContainer[networks];
  apNodes = new NodeContainer[networks];
  staDevices = new NetDeviceContainer[networks];
  staInterfaces = new Ipv4InterfaceContainer[networks];
  apDevices = new NetDeviceContainer[networks];
  apInterfaces = new Ipv4InterfaceContainer[networks];
}


bool TcpTest::Configure (int argc, char **argv, TcpApp tcpApp)
{
  // Enable AODV logs by default. Comment this if too noisy
  //LogComponentEnable("TcpCongestionOps", LOG_LEVEL_ALL);
  //LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
  // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
  // LogComponentEnable("TrafficControlLayer", LOG_LEVEL_ALL);
  // LogComponentEnable("Ipv4Interface", LOG_LEVEL_ALL);

  //SeedManager::SetSeed (12345);
  CommandLine cmd (__FILE__);

  cmd.AddValue ("simTime", "Simulation time in seconds", simulationTime);
  // cmd.AddValue("nNodes", "Number of nodes", noOfNodes);
  cmd.AddValue("nFlows", "Number of flows", noOfFlows);
  // cmd.AddValue("nPktsPerSec", "Number of packets per second", noOfPacketsPerSec);
  // cmd.AddValue("coverage", "Coverage Area", coverageArea);

  cmd.Parse (argc, argv);
  return true;
}

void TcpTest::CreateNodes ()
{
  NodeContainer tempNodes;
  tempNodes.Create(noOfNodes);

  nodes[0].Add(tempNodes.Get(0));
  nodes[0].Add(tempNodes.Get(1));
  apNodes[0].Add(tempNodes.Get(4));

  nodes[1].Add(tempNodes.Get(2));
  nodes[1].Add(tempNodes.Get(3));
  apNodes[1].Add(tempNodes.Get(5));

  nodes[2].Add(tempNodes.Get(4));
  nodes[2].Add(tempNodes.Get(5));
  apNodes[2].Add(tempNodes.Get(6));
}


void TcpTest::ApplyMobility(){
  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                "MinX", DoubleValue (0.0),
                                "MinY", DoubleValue (0.0),
                                "DeltaX", DoubleValue (5.0),
                                "DeltaY", DoubleValue (5.0),
                                "GridWidth", UintegerValue (3),
                                "LayoutType", StringValue ("RowFirst"));
                                
  
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  for(int i = 0; i < noOfNetworks; i++){
    mobility.Install (nodes[i].Get(0));
    mobility.Install (apNodes[i].Get(0));
    mobility.Install (nodes[i].Get(1));
  }    
}

void
TcpTest::CreateDevices(TcpApp tcpApp)
{

  AsciiTraceHelper ascii;


  /* Build wireless devices */
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_STANDARD_80211n_5GHZ);

  YansWifiChannelHelper wifiChannelHelper[noOfNetworks];

  for(int i = 0; i < noOfNetworks; i++){
    wifiChannelHelper[i].SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannelHelper[i].AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(20.0 * coverageArea));
  }

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhyHelper[noOfNetworks];

  for(int i = 0; i < noOfNetworks; i++){
    wifiPhyHelper[i].SetChannel (wifiChannelHelper[i].Create ());
    //wifiPhyHelper[i].SetErrorRateModel ("ns3::YansErrorRateModel");
    wifiPhyHelper[i].SetErrorRateModel ("ns3::NistErrorRateModel");
  }
    

  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue (tcpApp.phyRate),
                                    "ControlMode", StringValue ("HtMcs0")); 

  WifiMacHelper wifiMacHelper;

  Ssid ssid[noOfNetworks];


  for(int i = 0; i < noOfNetworks; i++){
      /* Configure AP */
      ssid[i] = Ssid ("network");
      wifiMacHelper.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid[i]));
      apDevices[i] = wifiHelper.Install (wifiPhyHelper[i], wifiMacHelper, apNodes[i]);

      /* Configure STA */
      wifiMacHelper.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid[i]));
      staDevices[i] = wifiHelper.Install (wifiPhyHelper[i], wifiMacHelper, nodes[i]); 
  }

}

void
TcpTest::InstallInternetStack ()
{
  InternetStackHelper stack;

  Ipv4AddressHelper address;

  stack.Install(apNodes[2]);

  for(int i = 0; i < noOfNetworks; i++){
      stack.Install (nodes[i]);
  }

  for(int i = 0; i < noOfNetworks; i++){

    std::string ip = "10.1." + std::to_string((i + 1)) +".0";
    address.SetBase (ip.c_str(), "255.255.255.0");

    apInterfaces[i] = address.Assign(apDevices[i]);
    staInterfaces[i] = address.Assign(staDevices[i]);

  }
}

void
InstallApplications(int fl)
{

  //for(int k = 0; k < 6; k++){


    for(int i = 0; i < 1; i++){

      for(int k = 0; k < 6; k++){
        // if(k == 2){
        //   continue;
        // }

        Ipv4Address addr;
        Ptr<Node> destNode;
        Ptr<Node> srcNode;

        if(k == 0){
          addr = apInterfaces[0].GetAddress(0);
          destNode = apNodes[0].Get(0);
          srcNode = nodes[0].Get(0);       
        }
        else if(k == 1){
          addr = apInterfaces[0].GetAddress(0);
          destNode = apNodes[0].Get(0);
          srcNode = nodes[0].Get(1);
        }
        else if(k == 2){
          addr = apInterfaces[1].GetAddress(0);
          destNode = apNodes[1].Get(0);
          srcNode = nodes[1].Get(0);
        }        
        else if(k == 3){
          addr = apInterfaces[1].GetAddress(0);
          destNode = apNodes[1].Get(0);
          srcNode = nodes[1].Get(1);
        }
        else if(k == 4){
          addr = apInterfaces[2].GetAddress(0);
          destNode = apNodes[2].Get(0);
          srcNode = nodes[2].Get(0);
        }
        else if(k == 5){
          addr = apInterfaces[2].GetAddress(0);
          destNode = apNodes[2].Get(0);
          srcNode = nodes[2].Get(1);
        }    

        srcNode->SetPrevNode(destNode);
        destNode->SetPrevNode(srcNode);

        if(i == 0){
          int curOffsprings = destNode->GetNodeOption()->GetOffsprings();
          destNode->GetNodeOption()->SetOffsprings(curOffsprings + 1);
        }


        uint16_t sinkPort = 1000*fl + k;

        Address sinkAddress (InetSocketAddress (addr, sinkPort));
        PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sinkPort));
        sinkApps = packetSinkHelper.Install(destNode);

        // packetSinkVec[i] = StaticCast<PacketSink>(sinkApps.Get(0));

        sinkApps.Start (Seconds (0.0));

        Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (srcNode, TcpSocketFactory::GetTypeId ());
        Ptr<SocketApp> socketApp = CreateObject<SocketApp> ();

        std::string dataRate = std::to_string(noOfPacketsPerSec * 1500 * 8);
        socketApp->Setup (ns3TcpSocket, sinkAddress, 1500, 1000, DataRate (dataRate));
        srcNode->AddApplication(socketApp);

        socketApp->SetStartTime (Seconds (1.0));

    
      }

      // sleep(1);

    }

  //}
}


void TcpTest::SetNodeOptions(){
  std::vector<Ptr<Node>> nodeVec;
  nodeVec.resize(noOfNodes);

  std::vector<Ptr<NodeOption>> optionVec;
  optionVec.resize(noOfNodes);

  for(int i = 0; i < noOfNetworks; i++){
    for(int j = 0; j < 2; j++){
      nodeVec[2*i+j] = nodes[i].Get(j);
      nodeVec[2*i+j]->SetPrevNode(apNodes[i].Get(0));
    }
  }

  nodeVec[6] = apNodes[2].Get(0);
  //nodeVec[6]->SetPrevNode

  // set options
  for(int i = 0; i < (int) noOfNodes; i++){
    optionVec[i] = CreateObject<NodeOption>();
    nodeVec[i]->SetNodeOption(optionVec[i]);
  }

  // set attributes
  for(int i = 0; i < noOfNetworks; i++){
    for(int j = 0; j < 2; j++){

      optionVec[2*i+j]->SetPktFactor(10);
      optionVec[2*i+j]->SetPktsSinceLastICD(0);
      optionVec[2*i+j]->SetTa(0.1);
      optionVec[2*i+j]->SetTs(0.1);
      optionVec[2*i+j]->SetSP(2*i + j + 2);
      optionVec[2*i+j]->SetGP(2*i + j + 2);
      optionVec[2*i+j]->SetParentOffsprings(0);
      optionVec[2*i+j]->SetSvc(10);
      optionVec[2*i+j]->SetParentDegree(1.0);
      optionVec[2*i+j]->SetParentAddress(apInterfaces[i].GetAddress(0));
      staDevices[i].Get(j)->ipAddr = apInterfaces[i].GetAddress(0);
      
    }
  }

  optionVec[6]->SetPktsSinceLastICD(0);
  optionVec[6]->SetTs(0.1);
  optionVec[6]->SetTa(0.1);
  optionVec[6]->SetSP(10);
  optionVec[6]->SetGP(10);
  optionVec[6]->SetOffsprings(0);   // active offsprings 
  optionVec[6]->SetParentAddress(apInterfaces[2].GetAddress(0));  


  // set parent node
  for(int i = 0; i < noOfNetworks; i++){
    for(int j = 0; j < 2; j++){
        nodeVec[2*i+j]->SetParentNode(apNodes[i].Get(0));
    }
  }

  int globalParentGP = 10;
  // calculate GP for parent nodes
  for(int i = 0; i < noOfNetworks - 1; i++){
    int gp = apNodes[i].Get(0)->GetNodeOption()->GetSP();

    for(int j = 0; j < 2; j++){
      gp += nodes[i].Get(0)->GetNodeOption()->GetSP();
    }

    apNodes[i].Get(0)->GetNodeOption()->SetGP(gp);
    // active offsprings
    //apNodes[i].Get(0)->GetNodeOption()->SetOffsprings(1);
    apNodes[i].Get(0)->GetNodeOption()->SetOffsprings(0);

    globalParentGP += gp;     
  }

  optionVec[6]->SetGP(globalParentGP);
  optionVec[6]->SetOffsprings(0);

}


void TcpTest::setErrorModel(Ptr<NetDevice> destDevice){

}


void TcpTest::Run(TcpApp tcpApp)
{
  CreateNodes();
  CreateDevices(tcpApp);
  ApplyMobility();
  InstallInternetStack();
  SetNodeOptions();

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  double time = 1.1;
  for(int i = 0; i < noOfFlows/2; i+=2){
    //InstallApplications(i);

    Simulator::Schedule (Seconds (time), &InstallApplications, i);
    time = time + 1.0;
    //sleep(1);
  }

  // for(int i = 0; i < noOfFlows / 2; i++){
  //   Simulator::Schedule (Seconds (1.1), &CalculateThroughput, i);
  // }

  std::cout << "Starting simulation for " << simulationTime << " s ...\n";

  // Flow monitor
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds (simulationTime + 1));

  AnimationInterface anim("test.xml");

  Simulator::Run ();

  flowMonitor->SerializeToXmlFile("flow.xml", true, true);
}


int main (int argc, char **argv)
{
  srand(time(0));
  TcpTest test;
  TcpApp tcpApp;

  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);

  simulationTime = 30;
  test.setNoOfNodes(7);
  test.setNoOfFlows(50);
  noOfPacketsPerSec = 300;
  coverageArea = 1;

  if (!test.Configure (argc, argv, tcpApp))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");


  test.setNoOfNetworks(3);
  tcpApp.tcpVariant = "TcpNewReno";

  // packetSinkVec.resize(test.noOfFlows/2);
  // fout.resize(test.noOfFlows/2);

  // for(int i = 0; i < test.noOfFlows/2; i++){
  //   packetSinkVec[i] = CreateObject<PacketSink>();
  //   std::string file = "sink" + std::to_string(i) + ".txt";
  //   fout[i].open(file.c_str(), std::ios::out);
  // }

  /* Configure TCP Options */
  tcpApp.SetTcpVariant();
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpApp.payloadSize));

  test.Run(tcpApp);


  Simulator::Destroy ();

  return 0;
}

