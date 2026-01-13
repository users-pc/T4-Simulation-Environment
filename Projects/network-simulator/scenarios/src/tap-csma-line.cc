/*
 * TAP-CSMA Line Topology - 4 Nodes
 * Node0 -- Node1 -- Node2 -- Node3 (all on shared CSMA channel)
 */

#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/internet-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapCsmaLineTopology");

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
    std::cout << "\n[" << Simulator::Now().GetSeconds() << "s] ";
    uint64_t total = 0;
    for (int i = 0; i < 4; i++) {
        std::cout << "N" << i << ":" << g_packets[i] << "p ";
        total += g_packets[i];
    }
    std::cout << "Total:" << total << "\n";
    Simulator::Schedule(Seconds(10.0), &PrintStats);
}

int main(int argc, char* argv[]) {
    bool verbose = false;
    std::string dataRate = "100Mbps";
    double time = 600.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("dataRate", "CSMA data rate", dataRate);
    cmd.AddValue("time", "Simulation time", time);
    cmd.Parse(argc, argv);

    if (verbose) {
        LogComponentEnable("TapCsmaLineTopology", LOG_LEVEL_INFO);
        LogComponentEnable("TapBridge", LOG_LEVEL_INFO);
    }

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    std::cout << "TAP-CSMA 4-Node Line Topology\n";

    NodeContainer nodes;
    nodes.Create(4);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer devices = csma.Install(nodes);

    // PCAP tracing
    for (uint32_t i = 0; i < 4; i++) {
        std::ostringstream oss;
        oss << "/tmp/node" << i;
        csma.EnablePcap(oss.str(), devices.Get(i), true);
    }

    // Traffic monitoring
    devices.Get(0)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx0));
    devices.Get(1)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx1));
    devices.Get(2)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx2));
    devices.Get(3)->TraceConnectWithoutContext("MacRx", MakeCallback(&Rx3));

    // TAP bridges
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("UseBridge"));
    const char* taps[] = {"tap-0", "tap-1", "tap-2", "tap-3"};
    for (uint32_t i = 0; i < 4; i++) {
        tapBridge.SetAttribute("DeviceName", StringValue(taps[i]));
        tapBridge.Install(nodes.Get(i), devices.Get(i));
        std::cout << "Node " << i << " -> " << taps[i] << "\n";
    }

    Simulator::Schedule(Seconds(10.0), &PrintStats);
    Simulator::Stop(Seconds(time));
    Simulator::Run();
    PrintStats();
    Simulator::Destroy();
    return 0;
}
