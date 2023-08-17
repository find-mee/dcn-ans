#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("lab2");


int
main (int argc, char *argv[])
{
  double simulationTime = 5; //seconds
  std::string socketType;

  CommandLine cmd;
  cmd.Parse (argc, argv);

/*
  if (transportProt.compare ("Tcp") == 0)
    {
      socketType = "ns3::TcpSocketFactory";
    }
  else
    {
      socketType = "ns3::UdpSocketFactory";
    }

*/

  NodeContainer nodes;
  nodes.Create (4);

  InternetStackHelper stack;
  stack.Install (nodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));
  pointToPoint.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));

//udp
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes.Get(0),nodes.Get(1));
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  devices = pointToPoint.Install (nodes.Get(1),nodes.Get(2));
  address.SetBase ("10.1.2.0", "255.255.255.0");
  interfaces = address.Assign (devices);

//tcp

  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.3.0", "255.255.255.0");

  NetDeviceContainer devices1;
  devices1 = pointToPoint.Install (nodes.Get(3),nodes.Get(1));
  Ipv4InterfaceContainer interfaces1 = address1.Assign (devices1);

  devices1 = pointToPoint.Install (nodes.Get(1),nodes.Get(2));
  address1.SetBase ("10.1.4.0", "255.255.255.0");
  interfaces1 = address.Assign (devices1);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

//udp flow

  uint32_t payloadSize = 1448;
  
  OnOffHelper onoff ("ns3::UdpSocketFactory", Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff.SetAttribute ("DataRate", StringValue ("50Mbps"));

  uint16_t port = 7;
  Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", localAddress);
  ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (2));

  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (5.0));

  ApplicationContainer apps;

  InetSocketAddress rmt (interfaces.GetAddress (1), port);
  rmt.SetTos (0xb8);
  AddressValue remoteAddress (rmt);
  onoff.SetAttribute ("Remote", remoteAddress);
  apps.Add (onoff.Install (nodes.Get (0)));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (5.0));

//TCP FLOW

  OnOffHelper onoff1 ("ns3::TcpSocketFactory", Ipv4Address::GetAny ());
  onoff1.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff1.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff1.SetAttribute ("DataRate", StringValue ("50Mbps"));

  uint16_t port1 = 9;
  Address localAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
  PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", localAddress);
  ApplicationContainer sinkApp1 = packetSinkHelper1.Install (nodes.Get (2));

  sinkApp1.Start (Seconds (1.0));
  sinkApp1.Stop (Seconds (simulationTime + 0.1));

  ApplicationContainer apps1;

  InetSocketAddress rmt1 (interfaces.GetAddress (1), port);
  rmt1.SetTos (0xb8);
  AddressValue remoteAddress1 (rmt1);
  onoff1.SetAttribute ("Remote", remoteAddress1);
  apps1.Add (onoff1.Install (nodes.Get (3)));
  apps1.Start (Seconds (1.5));
  apps1.Stop (Seconds (simulationTime + 0.1));

/*
  //Flow
  uint16_t port = 7;
  Address localAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper (socketType, localAddress);
  ApplicationContainer sinkApp = packetSinkHelper.Install (nodes.Get (0));

  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime + 0.1));

  uint32_t payloadSize = 1448;
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

  OnOffHelper onoff (socketType, Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff.SetAttribute ("DataRate", StringValue ("50Mbps")); //bit/s
  ApplicationContainer apps;

  InetSocketAddress rmt (interfaces.GetAddress (0), port);
  rmt.SetTos (0xb8);
  AddressValue remoteAddress (rmt);
  onoff.SetAttribute ("Remote", remoteAddress);
  apps.Add (onoff.Install (nodes.Get (1)));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (simulationTime + 0.1));
*/
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop (Seconds (simulationTime + 5));

  AnimationInterface anim ("ex2.xml");
  AnimationInterface::SetConstantPosition(nodes.Get(0),1.0,1.0);
  AnimationInterface::SetConstantPosition(nodes.Get(1),20.0,2.0);
  AnimationInterface::SetConstantPosition(nodes.Get(2),30.0,3.0);
  AnimationInterface::SetConstantPosition(nodes.Get(3),1.0,15.0);
  anim.EnablePacketMetadata();


  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  std::cout << std::endl << "*** Flow monitor statistics ***" << std::endl;
  std::cout << "  Tx Packets/Bytes:   " << stats[1].txPackets
            << " / " << stats[1].txBytes << std::endl;
  std::cout << "  Offered Load: " << stats[1].txBytes * 8.0 / (stats[1].timeLastTxPacket.GetSeconds () - stats[1].timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Rx Packets/Bytes:   " << stats[1].rxPackets
            << " / " << stats[1].rxBytes << std::endl;
  uint32_t packetsDroppedByQueueDisc = 0;
  uint64_t bytesDroppedByQueueDisc = 0;
  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE_DISC)
    {
      packetsDroppedByQueueDisc = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
      bytesDroppedByQueueDisc = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
    }
  std::cout << "  Packets/Bytes Dropped by Queue Disc:   " << packetsDroppedByQueueDisc
            << " / " << bytesDroppedByQueueDisc << std::endl;
  

  Simulator::Destroy ();

  
  return 0;
}
