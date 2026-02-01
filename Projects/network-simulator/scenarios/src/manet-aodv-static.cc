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
static uint64_t g_macRxBytes[4] = {0}, g_macRxPkts[4] = {0};
static uint64_t g_macTxBytes[4] = {0}, g_macTxPkts[4] = {0};
static uint64_t g_ipRxBytes[4] = {0}, g_ipRxPkts[4] = {0};
static uint64_t g_ipTxBytes[4] = {0}, g_ipTxPkts[4] = {0};
static uint64_t g_ipDropPkts = 0;
static uint64_t g_phyTxBegin = 0, g_phyTxEnd = 0, g_phyTxDrop = 0;
static uint64_t g_phyRxBegin = 0, g_phyRxEnd = 0, g_phyRxDrop = 0;

// AODV verification counters
static uint64_t g_aodvRreqTx = 0, g_aodvRrepTx = 0;
static uint64_t g_aodvRreqRx = 0, g_aodvRrepRx = 0;

// Trace callbacks
static void IpDropCallback(const Ipv4Header &header, Ptr<const Packet> p,
                           Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface)
{
    g_ipDropPkts++;
    std::cout << "[DROP] IP packet to " << header.GetDestination()
              << " reason=" << reason << " iface=" << interface << "\n";
}

static void PhyTxBeginCallback(Ptr<const Packet> p, double txPowerW) { g_phyTxBegin++; }
static void PhyTxEndCallback(Ptr<const Packet> p) { g_phyTxEnd++; }
static void PhyTxDropCallback(Ptr<const Packet> p) { g_phyTxDrop++; std::cout << "[DROP] PHY TX\n"; }
static void PhyRxBeginCallback(Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW) { g_phyRxBegin++; }
static void PhyRxEndCallback(Ptr<const Packet> p) { g_phyRxEnd++; }

static void PhyRxDropCallback(Ptr<const Packet> p, WifiPhyRxfailureReason reason) {
    g_phyRxDrop++;
    std::cout << "[DROP] PHY RX reason=" << reason << " (";
    switch(reason) {
        case UNSUPPORTED_SETTINGS: std::cout << "UNSUPPORTED_SETTINGS"; break;
        case CHANNEL_SWITCHING: std::cout << "CHANNEL_SWITCHING"; break;
        case RXING: std::cout << "RXING"; break;
        case TXING: std::cout << "TXING"; break;
        case SLEEPING: std::cout << "SLEEPING"; break;
        case POWERED_OFF: std::cout << "POWERED_OFF"; break;
        case TRUNCATED_TX: std::cout << "TRUNCATED_TX"; break;
        case BUSY_DECODING_PREAMBLE: std::cout << "BUSY_DECODING_PREAMBLE"; break;
        case PREAMBLE_DETECT_FAILURE: std::cout << "PREAMBLE_DETECT_FAILURE"; break;
        case RECEPTION_ABORTED_BY_TX: std::cout << "RECEPTION_ABORTED_BY_TX"; break;
        case L_SIG_FAILURE: std::cout << "L_SIG_FAILURE"; break;
        case HT_SIG_FAILURE: std::cout << "HT_SIG_FAILURE"; break;
        case SIG_A_FAILURE: std::cout << "SIG_A_FAILURE"; break;
        case SIG_B_FAILURE: std::cout << "SIG_B_FAILURE"; break;
        case PREAMBLE_DETECTION_PACKET_SWITCH: std::cout << "PREAMBLE_DETECTION_PACKET_SWITCH"; break;
        case FRAME_CAPTURE_PACKET_SWITCH: std::cout << "FRAME_CAPTURE_PACKET_SWITCH"; break;
        case OBSS_PD_CCA_RESET: std::cout << "OBSS_PD_CCA_RESET"; break;
        case FILTERED: std::cout << "FILTERED"; break;
        default: std::cout << "UNKNOWN"; break;
    }
    std::cout << ")\n";
}

static void MacRxCallback(uint32_t idx, Ptr<const Packet> p) {
    g_macRxBytes[idx] += p->GetSize();
    g_macRxPkts[idx]++;
    NS_LOG_INFO("MAC RX Node " << idx << ": " << p->GetSize() << " bytes");
}

static void MacTxCallback(uint32_t idx, Ptr<const Packet> p) {
    g_macTxBytes[idx] += p->GetSize();
    g_macTxPkts[idx]++;
    NS_LOG_INFO("MAC TX Node " << idx << ": " << p->GetSize() << " bytes");
}

static void IpRxCallback(Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interface) {
    uint32_t nodeId = ipv4->GetObject<Node>()->GetId();
    if (nodeId < 4) {
        g_ipRxBytes[nodeId] += p->GetSize();
        g_ipRxPkts[nodeId]++;
        NS_LOG_INFO("IP RX Node " << nodeId << " iface " << interface << ": " << p->GetSize() << " bytes");
    }
}

static void IpTxCallback(Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interface) {
    uint32_t nodeId = ipv4->GetObject<Node>()->GetId();
    if (nodeId < 4) {
        g_ipTxBytes[nodeId] += p->GetSize();
        g_ipTxPkts[nodeId]++;
        NS_LOG_INFO("IP TX Node " << nodeId << " iface " << interface << ": " << p->GetSize() << " bytes");
    }
}

// Individual MAC callbacks (required for TraceConnectWithoutContext)
static void MacRx0(Ptr<const Packet> p) { MacRxCallback(0, p); }
static void MacRx1(Ptr<const Packet> p) { MacRxCallback(1, p); }
static void MacRx2(Ptr<const Packet> p) { MacRxCallback(2, p); }
static void MacRx3(Ptr<const Packet> p) { MacRxCallback(3, p); }
static void MacTx0(Ptr<const Packet> p) { MacTxCallback(0, p); }
static void MacTx1(Ptr<const Packet> p) { MacTxCallback(1, p); }
static void MacTx2(Ptr<const Packet> p) { MacTxCallback(2, p); }
static void MacTx3(Ptr<const Packet> p) { MacTxCallback(3, p); }

// Statistics functions
static void PrintStats() {
    std::cout << "\n[" << Simulator::Now().GetSeconds() << "s] Traffic Statistics:\n";
    std::cout << "  MAC Layer (WiFi):\n";
    uint64_t totalMacTx = 0, totalMacRx = 0;
    for (int i = 0; i < 4; i++) {
        std::cout << "    Node" << i << ": TX=" << g_macTxPkts[i] << "pkts/" << g_macTxBytes[i] << "B"
                  << " RX=" << g_macRxPkts[i] << "pkts/" << g_macRxBytes[i] << "B\n";
        totalMacTx += g_macTxPkts[i];
        totalMacRx += g_macRxPkts[i];
    }
    std::cout << "  IP Layer (includes TAP traffic):\n";
    uint64_t totalIpTx = 0, totalIpRx = 0;
    for (int i = 0; i < 4; i++) {
        std::cout << "    Node" << i << ": TX=" << g_ipTxPkts[i] << "pkts/" << g_ipTxBytes[i] << "B"
                  << " RX=" << g_ipRxPkts[i] << "pkts/" << g_ipRxBytes[i] << "B\n";
        totalIpTx += g_ipTxPkts[i];
        totalIpRx += g_ipRxPkts[i];
    }
    std::cout << "  Totals: MAC TX=" << totalMacTx << " RX=" << totalMacRx
              << " | IP TX=" << totalIpTx << " RX=" << totalIpRx << "\n";
    std::cout << "  PHY: TxBegin=" << g_phyTxBegin << " TxEnd=" << g_phyTxEnd << " TxDrop=" << g_phyTxDrop
              << " | RxBegin=" << g_phyRxBegin << " RxEnd=" << g_phyRxEnd << " RxDrop=" << g_phyRxDrop << "\n";
    std::cout << "  Drops: IP=" << g_ipDropPkts << "\n";
    Simulator::Schedule(Seconds(10.0), &PrintStats);
}

static void PrintFinalStats() {
    std::cout << "\n=== FINAL STATISTICS ===\n";
    std::cout << "MAC Layer (WiFi):\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "  Node " << i << ": TX=" << g_macTxPkts[i] << " pkts/" << g_macTxBytes[i] << " bytes"
                  << ", RX=" << g_macRxPkts[i] << " pkts/" << g_macRxBytes[i] << " bytes\n";
    }
    std::cout << "IP Layer (includes TAP traffic):\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "  Node " << i << ": TX=" << g_ipTxPkts[i] << " pkts/" << g_ipTxBytes[i] << " bytes"
                  << ", RX=" << g_ipRxPkts[i] << " pkts/" << g_ipRxBytes[i] << " bytes\n";
    }
    std::cout << "PHY Layer:\n";
    std::cout << "  TX: Begin=" << g_phyTxBegin << " End=" << g_phyTxEnd << " Drop=" << g_phyTxDrop << "\n";
    std::cout << "  RX: Begin=" << g_phyRxBegin << " End=" << g_phyRxEnd << " Drop=" << g_phyRxDrop << "\n";
    std::cout << "AODV Routing:\n";
    std::cout << "  RREQ: TX=" << g_aodvRreqTx << " RX=" << g_aodvRreqRx << "\n";
    std::cout << "  RREP: TX=" << g_aodvRrepTx << " RX=" << g_aodvRrepRx << "\n";
    std::cout << "Drops: IP=" << g_ipDropPkts << "\n";
}

// Setup functions

/**
 * Configure WiFi ad-hoc network
 */
static NetDeviceContainer SetupWifi(NodeContainer &nodes, YansWifiPhyHelper &wifiPhy) {
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("OfdmRate6Mbps"),
                                 "ControlMode", StringValue("OfdmRate6Mbps"));

    wifiPhy.Set("TxPowerStart", DoubleValue(20.0));
    wifiPhy.Set("TxPowerEnd", DoubleValue(20.0));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(50.0));
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    return wifi.Install(wifiPhy, wifiMac, nodes);
}

/**
 * Configure node mobility model
 */
static void SetupMobility(NodeContainer &nodes, const std::string &mobility_model,
                          double speed, double pause) {
    MobilityHelper mobility;

    // Initial positions (2x2 grid)
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(50.0, 0.0, 0.0));
    positionAlloc->Add(Vector(0.0, 50.0, 0.0));
    positionAlloc->Add(Vector(60.0, 60.0, 0.0));
    mobility.SetPositionAllocator(positionAlloc);

    if (mobility_model == "random-waypoint") {
        std::ostringstream speedStr, pauseStr;
        speedStr << "ns3::UniformRandomVariable[Min=0|Max=" << speed << "]";
        pauseStr << "ns3::ConstantRandomVariable[Constant=" << pause << "]";
        mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
            "Speed", StringValue(speedStr.str()),
            "Pause", StringValue(pauseStr.str()),
            "PositionAllocator", PointerValue(CreateObjectWithAttributes<RandomRectanglePositionAllocator>(
                "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"),
                "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"))));
    } else if (mobility_model == "random-walk") {
        mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
            "Bounds", RectangleValue(Rectangle(0, 100, 0, 100)),
            "Speed", StringValue("ns3::UniformRandomVariable[Min=1|Max=" + std::to_string(speed) + "]"));
    } else {
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    }
    mobility.Install(nodes);
}

/**
 * Install AODV routing and IP stack
 */
static Ipv4InterfaceContainer SetupNetwork(NodeContainer &nodes, NetDeviceContainer &devices) {
    AodvHelper aodv;
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    return address.Assign(devices);
}

/**
 * Setup all trace connections for monitoring
 */
static void SetupTracing(NodeContainer &nodes) {
    // MAC layer traces
    Config::ConnectWithoutContext("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx", MakeCallback(&MacRx0));
    Config::ConnectWithoutContext("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx", MakeCallback(&MacRx1));
    Config::ConnectWithoutContext("/NodeList/2/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx", MakeCallback(&MacRx2));
    Config::ConnectWithoutContext("/NodeList/3/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx", MakeCallback(&MacRx3));
    Config::ConnectWithoutContext("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx", MakeCallback(&MacTx0));
    Config::ConnectWithoutContext("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx", MakeCallback(&MacTx1));
    Config::ConnectWithoutContext("/NodeList/2/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx", MakeCallback(&MacTx2));
    Config::ConnectWithoutContext("/NodeList/3/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx", MakeCallback(&MacTx3));

    // IP layer traces
    for (uint32_t i = 0; i < 4; i++) {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
        ipv4->TraceConnectWithoutContext("Tx", MakeCallback(&IpTxCallback));
        ipv4->TraceConnectWithoutContext("Rx", MakeCallback(&IpRxCallback));
        ipv4->TraceConnectWithoutContext("Drop", MakeCallback(&IpDropCallback));
    }

    // PHY layer traces
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback(&PhyTxBeginCallback));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxEnd", MakeCallback(&PhyTxEndCallback));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDropCallback));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxBegin", MakeCallback(&PhyRxBeginCallback));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxEnd", MakeCallback(&PhyRxEndCallback));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDropCallback));

    std::cout << "Tracing enabled: MAC, IP, PHY layers\n";
}

/**
 * Setup TAP bridges for Docker container connectivity
 */
static void SetupTapBridges(NodeContainer &nodes, NetDeviceContainer &devices) {
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("UseLocal"));
    const char *taps[] = {"tap-0", "tap-1", "tap-2", "tap-3"};

    for (uint32_t i = 0; i < 4; i++) {
        tapBridge.SetAttribute("DeviceName", StringValue(taps[i]));
        tapBridge.Install(nodes.Get(i), devices.Get(i));
        std::cout << "TAP: " << taps[i] << " -> Node " << i << "\n";
    }
}

/**
 * Setup NetAnim visualization (anim must be created in main to stay alive)
 */
static void SetupNetAnim(AnimationInterface &anim, NodeContainer &nodes,
                         Ipv4InterfaceContainer &interfaces, double time,
                         const std::string &animFile) {
    anim.SetMaxPktsPerTraceFile(1000000);
    anim.EnablePacketMetadata(true);
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(time));

    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        std::ostringstream oss;
        oss << "Node" << i << " (" << interfaces.GetAddress(i) << ")";
        anim.UpdateNodeDescription(nodes.Get(i), oss.str());
        anim.UpdateNodeColor(nodes.Get(i), 0, 128, 255);
        anim.UpdateNodeSize(nodes.Get(i)->GetId(), 5, 5);
    }

    std::cout << "\nNetAnim output: " << animFile << "\n";
    std::cout << "PCAP files: /tmp/aodv-tap-*.pcap\n";
    std::cout << "Waiting for TAP traffic...\n\n";
}

// Main function
int main(int argc, char *argv[]) {
    // Configuration parameters
    double time = 60.0;
    bool verbose = false;
    std::string animFile = "manet-aodv-tap.xml";
    std::string mobility_model = "static";
    double speed = 5.0;
    double pause = 2.0;

    // Parse command line
    CommandLine cmd(__FILE__);
    cmd.AddValue("time", "Simulation time in seconds", time);
    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("animFile", "NetAnim output XML file", animFile);
    cmd.AddValue("mobility", "Mobility model: static, random-waypoint, random-walk", mobility_model);
    cmd.AddValue("speed", "Max speed in m/s (for mobile models)", speed);
    cmd.AddValue("pause", "Pause time in seconds (for random-waypoint)", pause);
    cmd.Parse(argc, argv);

    // Enable logging if verbose
    if (verbose) {
        LogComponentEnable("TapAodvNetanim", LOG_LEVEL_INFO);
        LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_DEBUG);
        LogComponentEnable("TapBridge", LOG_LEVEL_INFO);
    }

    // Real-time simulation required for TAP bridges
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    std::cout << "\n=== TAP-AODV-NETANIM: 4-Node MANET ===\n";
    std::cout << "Mobility: " << mobility_model << " (speed=" << speed << "m/s, pause=" << pause << "s)\n";

    // Create and configure network
    NodeContainer nodes;
    nodes.Create(4);

    YansWifiPhyHelper wifiPhy;
    NetDeviceContainer devices = SetupWifi(nodes, wifiPhy);
    SetupMobility(nodes, mobility_model, speed, pause);
    Ipv4InterfaceContainer interfaces = SetupNetwork(nodes, devices);

    // Print node addresses
    for (uint32_t i = 0; i < 4; i++) {
        std::cout << "Node " << i << ": " << interfaces.GetAddress(i) << "\n";
    }

    // Enable PCAP tracing
    wifiPhy.EnablePcapAll("/tmp/aodv-tap");

    // Setup all tracing and TAP bridges
    SetupTracing(nodes);
    SetupTapBridges(nodes, devices);

    // NetAnim must be created here (in main) to stay alive during simulation
    AnimationInterface anim(animFile);
    SetupNetAnim(anim, nodes, interfaces, time, animFile);

    // Run simulation
    Simulator::Schedule(Seconds(10.0), &PrintStats);
    Simulator::Stop(Seconds(time));
    Simulator::Run();

    // Print final statistics
    PrintFinalStats();

    Simulator::Destroy();
    return 0;
}
