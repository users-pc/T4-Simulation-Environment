/*
 * TAP-WIFI-AODV MANET - 4 Mobile Nodes
 * Wireless ad-hoc network with AODV routing and RandomWaypoint mobility
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/tap-bridge-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapWifiAodvManet");

static uint64_t g_bytes[4] = {0}, g_packets[4] = {0};

static void RxCallback(uint32_t idx, Ptr<const Packet> p) {
    g_bytes[idx] += p->GetSize();
    g_packets[idx]++;
}
static void Rx0(Ptr<const Packet> p) { RxCallback(0, p); }
static void Rx1(Ptr<const Packet> p) { RxCallback(1, p); }
static void Rx2(Ptr<const Packet> p) { RxCallback(2, p); }
static void Rx3(Ptr<const Packet> p) { RxCallback(3, p); }

static void PrintPositions(NodeContainer& nodes) {
    std::cout << "\n[" << Simulator::Now().GetSeconds() << "s] Positions: ";
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        Vector pos = nodes.Get(i)->GetObject<MobilityModel>()->GetPosition();
        std::cout << "N" << i << "(" << (int)pos.x << "," << (int)pos.y << ") ";
    }
    std::cout << "\n";
    Simulator::Schedule(Seconds(30.0), &PrintPositions, nodes);
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    double time = 300.0, speed = 2.0, pause = 5.0, txPower = 20.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("time", "Simulation time", time);
    cmd.AddValue("speed", "Max node speed (m/s)", speed);
    cmd.AddValue("pause", "Pause at waypoints (s)", pause);
    cmd.AddValue("txPower", "TX power (dBm)", txPower);
    cmd.Parse(argc, argv);

    if (verbose) {
        LogComponentEnable("TapWifiAodvManet", LOG_LEVEL_INFO);
        LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_DEBUG);
    }

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    std::cout << "TAP-WIFI-AODV MANET (4 Mobile Nodes)\n";

    NodeContainer nodes;
    nodes.Create(4);

    // WiFi setup
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("TxPowerStart", DoubleValue(txPower));
    wifiPhy.Set("TxPowerEnd", DoubleValue(txPower));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(100.0));
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Mobility
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
        "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"),
        "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"));

    std::ostringstream ss, ps;
    ss << "ns3::UniformRandomVariable[Min=0|Max=" << speed << "]";
    ps << "ns3::ConstantRandomVariable[Constant=" << pause << "]";
    mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
        "Speed", StringValue(ss.str()), "Pause", StringValue(ps.str()));
    mobility.Install(nodes);

    // AODV routing
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    for (uint32_t i = 0; i < 4; i++)
        std::cout << "Node " << i << ": " << interfaces.GetAddress(i) << "\n";

    // Tracing
    wifiPhy.EnablePcapAll("/tmp/aodv");
    devices.Get(0)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx0));
    devices.Get(1)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx1));
    devices.Get(2)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx2));
    devices.Get(3)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx3));

    // TAP bridges
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("UseLocal"));
    const char* taps[] = {"tap-0", "tap-1", "tap-2", "tap-3"};
    for (uint32_t i = 0; i < 4; i++) {
        tapBridge.SetAttribute("DeviceName", StringValue(taps[i]));
        tapBridge.Install(nodes.Get(i), devices.Get(i));
    }

    Simulator::Schedule(Seconds(1.0), &PrintPositions, nodes);
    Simulator::Stop(Seconds(time));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
