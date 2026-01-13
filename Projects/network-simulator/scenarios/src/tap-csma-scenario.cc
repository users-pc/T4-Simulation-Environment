/*
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

//
// This is an illustration of how one could use virtualization techniques to
// allow running applications on virtual machines talking over simulated
// networks.
//
// The actual steps required to configure the virtual machines can be rather
// involved, so we don't go into that here.  Please have a look at one of
// our HOWTOs on the nsnam wiki for more details about how to get the
// system confgured.  For an example, have a look at "HOWTO Use Linux
// Containers to set up virtual networks" which uses this code as an
// example.
//
// The configuration you are after is explained in great detail in the
// HOWTO, but looks like the following:
//
//  +----------+                           +----------+
//  | virtual  |                           | virtual  |
//  |  Linux   |                           |  Linux   |
//  |   Host   |                           |   Host   |
//  |          |                           |          |
//  |   eth0   |                           |   eth0   |
//  +----------+                           +----------+
//       |                                      |
//  +----------+                           +----------+
//  |  Linux   |                           |  Linux   |
//  |  Bridge  |                           |  Bridge  |
//  +----------+                           +----------+
//       |                                      |
//  +------------+                       +-------------+
//  | "tap-left" |                       | "tap-right" |
//  +------------+                       +-------------+
//       |           n0            n1           |
//       |       +--------+    +--------+       |
//       +-------|  tap   |    |  tap   |-------+
//               | bridge |    | bridge |
//               +--------+    +--------+
//               |  CSMA  |    |  CSMA  |
//               +--------+    +--------+
//                   |             |
//                   |             |
//                   |             |
//                   ===============
//                      CSMA LAN
//
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/network-module.h"
#include "ns3/tap-bridge-module.h"

#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TapCsmaVirtualMachineExample");

// Global counters for traffic statistics
static uint64_t g_totalBytesLeft = 0;
static uint64_t g_totalBytesRight = 0;
static uint64_t g_totalPacketsLeft = 0;
static uint64_t g_totalPacketsRight = 0;

// Callback for packet reception on left node
static void
RxCallbackLeft(Ptr<const Packet> packet)
{
    g_totalBytesLeft += packet->GetSize();
    g_totalPacketsLeft++;
    NS_LOG_INFO("LEFT  RX: " << packet->GetSize() << " bytes (Total: "
                << g_totalPacketsLeft << " pkts, " << g_totalBytesLeft << " bytes)");
}

// Callback for packet reception on right node
static void
RxCallbackRight(Ptr<const Packet> packet)
{
    g_totalBytesRight += packet->GetSize();
    g_totalPacketsRight++;
    NS_LOG_INFO("RIGHT RX: " << packet->GetSize() << " bytes (Total: "
                << g_totalPacketsRight << " pkts, " << g_totalBytesRight << " bytes)");
}

// Periodic statistics printout
static void
PrintStats()
{
    std::cout << "\n========== Traffic Statistics at " << Simulator::Now().GetSeconds() << "s ==========\n";
    std::cout << "LEFT  Node: " << g_totalPacketsLeft << " packets, " << g_totalBytesLeft << " bytes\n";
    std::cout << "RIGHT Node: " << g_totalPacketsRight << " packets, " << g_totalBytesRight << " bytes\n";
    std::cout << "TOTAL:      " << (g_totalPacketsLeft + g_totalPacketsRight) << " packets, "
              << (g_totalBytesLeft + g_totalBytesRight) << " bytes\n";
    std::cout << "==========================================================\n\n";

    // Schedule next stats printout in 10 seconds
    Simulator::Schedule(Seconds(10.0), &PrintStats);
}

int
main(int argc, char* argv[])
{
    // Enable logging for this component
    bool verbose = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Enable verbose logging", verbose);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("TapCsmaVirtualMachineExample", LOG_LEVEL_INFO);
        LogComponentEnable("TapBridge", LOG_LEVEL_INFO);
        LogComponentEnable("CsmaNetDevice", LOG_LEVEL_INFO);
    }

    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║           NS-3 TAP-CSMA Scenario                              ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════╣\n";

    //
    // We are interacting with the outside, real, world.  This means we have to
    // interact in real-time and therefore means we have to use the real-time
    // simulator and take the time to calculate checksums.
    //
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

    //
    // Create two ghost nodes.  The first will represent the virtual machine host
    // on the left side of the network; and the second will represent the VM on
    // the right side.
    //
    NodeContainer nodes;
    nodes.Create(2);

    //
    // Use a CsmaHelper to get a CSMA channel created, and the needed net
    // devices installed on both of the nodes.  The data rate and delay for the
    // channel can be set through the command-line parser.  For example,
    //
    // ./ns3 run "tap-csma-virtual-machine --ns3::CsmaChannel::DataRate=10000000"
    //
    CsmaHelper csma;
    // Set reasonable defaults for the CSMA channel
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer devices = csma.Install(nodes);

    //
    // Enable PCAP tracing on the CSMA devices
    // Files will be created in /tmp/tap-csma-left-0.pcap and /tmp/tap-csma-right-0.pcap
    //
    csma.EnablePcap("/tmp/tap-csma-left", devices.Get(0), true);
    csma.EnablePcap("/tmp/tap-csma-right", devices.Get(1), true);

    //
    // Enable ASCII tracing for text-based analysis
    //
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("/tmp/tap-csma-trace.tr"));

    //
    // Connect trace callbacks to monitor traffic
    //
    devices.Get(0)->TraceConnectWithoutContext("MacRx", MakeCallback(&RxCallbackLeft));
    devices.Get(1)->TraceConnectWithoutContext("MacRx", MakeCallback(&RxCallbackRight));

    //
    // Use the TapBridgeHelper to connect to the pre-configured tap devices for
    // the left side.  We go with "UseBridge" mode since the CSMA devices support
    // promiscuous mode and can therefore make it appear that the bridge is
    // extended into ns-3.  The install method essentially bridges the specified
    // tap to the specified CSMA device.
    //
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("UseBridge"));
    tapBridge.SetAttribute("DeviceName", StringValue("tap-left"));
    tapBridge.Install(nodes.Get(0), devices.Get(0));

    //
    // Connect the right side tap to the right side CSMA device on the right-side
    // ghost node.
    //
    tapBridge.SetAttribute("DeviceName", StringValue("tap-right"));
    tapBridge.Install(nodes.Get(1), devices.Get(1));

    Simulator::Schedule(Seconds(10.0), &PrintStats);

    Simulator::Stop(Seconds(600.));
    Simulator::Run();

    // Print final statistics
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    FINAL TRAFFIC SUMMARY                      ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  LEFT  Node: " << g_totalPacketsLeft << " packets, " << g_totalBytesLeft << " bytes\n";
    std::cout << "║  RIGHT Node: " << g_totalPacketsRight << " packets, " << g_totalBytesRight << " bytes\n";
    std::cout << "║  TOTAL:      " << (g_totalPacketsLeft + g_totalPacketsRight) << " packets, "
              << (g_totalBytesLeft + g_totalBytesRight) << " bytes\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\nPCAP files saved to:\n";
    std::cout << "  - /tmp/tap-csma-left-0.pcap\n";
    std::cout << "  - /tmp/tap-csma-right-0.pcap\n";
    std::cout << "Open with: wireshark /tmp/tap-csma-left-0.pcap\n\n";

    Simulator::Destroy();
}

