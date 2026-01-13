/*
 * TAP-AODV-NETANIM - 4 Node MANET with TAP Bridges
 * Combines: TAP interface → WiFi Ad-hoc → AODV Routing → NetAnim Visualization
 *
 * Traffic from Docker containers flows through TAP devices into the ns-3
 * simulation, gets routed via AODV, and is visualized in NetAnim.
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/netanim-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapAodvNetanim");

// Traffic counters
static uint64_t g_bytes[4] = {0}, g_packets[4] = {0};

static void RxCallback(uint32_t idx, Ptr<const Packet> p) {
    g_bytes[idx] += p->GetSize();
    g_packets[idx]++;
}
static void Rx0(Ptr<const Packet> p) { RxCallback(0, p); }
static void Rx1(Ptr<const Packet> p) { RxCallback(1, p); }
static void Rx2(Ptr<const Packet> p) { RxCallback(2, p); }
static void Rx3(Ptr<const Packet> p) { RxCallback(3, p); }

static void PrintStats() {
    std::cout << "\n[" << Simulator::Now().GetSeconds() << "s] Traffic: ";
    uint64_t total = 0;
    for (int i = 0; i < 4; i++) {
        std::cout << "N" << i << ":" << g_packets[i] << "p/" << g_bytes[i] << "B ";
        total += g_packets[i];
    }
    std::cout << "| Total:" << total << " packets\n";
    Simulator::Schedule(Seconds(10.0), &PrintStats);
}

int main(int argc, char* argv[])
{
    double time = 600.0;
    bool verbose = false;
    std::string animFile = "manet-aodv-tap.xml";

    CommandLine cmd(__FILE__);
    cmd.AddValue("time", "Simulation time in seconds", time);
    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("animFile", "NetAnim output XML file", animFile);
    cmd.Parse(argc, argv);

    if (verbose) {
        LogComponentEnable("TapAodvNetanim", LOG_LEVEL_INFO);
        LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_DEBUG);
        LogComponentEnable("TapBridge", LOG_LEVEL_INFO);
    }

    // Real-time simulation required for TAP bridges
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    std::cout << "\n=== TAP-AODV-NETANIM: 4-Node MANET ===\n";

    // Create nodes
    NodeContainer nodes;
    nodes.Create(4);

    // Configure WiFi (Ad-hoc mode for MANET)
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue("OfdmRate6Mbps"),
                                  "ControlMode", StringValue("OfdmRate6Mbps"));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("TxPowerStart", DoubleValue(20.0));
    wifiPhy.Set("TxPowerEnd", DoubleValue(20.0));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(50.0));
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Set static positions (2x2 grid, 50m apart)
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(50.0, 0.0, 0.0));
    positionAlloc->Add(Vector(0.0, 50.0, 0.0));
    positionAlloc->Add(Vector(60.0, 60.0, 0.0));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(nodes);

    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    for (uint32_t i = 0; i < 4; i++) {
        std::cout << "Node " << i << ": " << interfaces.GetAddress(i) << "\n";
    }

    // Enable PCAP tracing
    wifiPhy.EnablePcapAll("/tmp/aodv-tap");

    // Traffic monitoring
    devices.Get(0)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx0));
    devices.Get(1)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx1));
    devices.Get(2)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx2));
    devices.Get(3)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx3));

    // TAP bridges - UseLocal mode for Layer 3 routing with IP stack
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("UseLocal"));
    const char* taps[] = {"tap-0", "tap-1", "tap-2", "tap-3"};
    for (uint32_t i = 0; i < 4; i++) {
        tapBridge.SetAttribute("DeviceName", StringValue(taps[i]));
        tapBridge.Install(nodes.Get(i), devices.Get(i));
        std::cout << "TAP: " << taps[i] << " -> Node " << i << "\n";
    }

    // Setup NetAnim visualization
    AnimationInterface anim(animFile);
    anim.SetMaxPktsPerTraceFile(1000000);
    anim.EnablePacketMetadata(true);
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(time));

    // Set node descriptions and colors for NetAnim
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        std::ostringstream oss;
        oss << "Node" << i << " (" << interfaces.GetAddress(i) << ")";
        anim.UpdateNodeDescription(nodes.Get(i), oss.str());
        anim.UpdateNodeColor(nodes.Get(i), 0, 128, 255);  // Blue
        anim.UpdateNodeSize(nodes.Get(i)->GetId(), 5, 5);
    }

    std::cout << "\nNetAnim output: " << animFile << "\n";
    std::cout << "PCAP files: /tmp/aodv-tap-*.pcap\n";
    std::cout << "Waiting for TAP traffic...\n\n";

    // Run simulation
    Simulator::Schedule(Seconds(10.0), &PrintStats);
    Simulator::Stop(Seconds(time));
    Simulator::Run();

    // Final stats
    std::cout << "\n=== FINAL STATISTICS ===\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "Node " << i << ": " << g_packets[i] << " packets, " << g_bytes[i] << " bytes\n";
    }

    Simulator::Destroy();
    return 0;
}
