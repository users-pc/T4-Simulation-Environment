//
// Custom AODV Application - UDP-based socket communication app
//
package adhoc_simulation_t4;

import inet.applications.contract.IApp;

//
// Custom UDP Application for AODV Router
// Provides socket-based communication between AODV nodes
//
simple CustomAodvApp like IApp
{
    parameters:
        @class(CustomAodvApp);
        @display("i=block/app");
        
        // Interface table module path (required by IApp)
        string interfaceTableModule;
        
        // Local socket configuration
        int localPort = default(5000);
        string localAddress = default("");
        
        // Destination configuration
        int destPort = default(5000);
        string destAddress = default("");
        
        // Message parameters
        int messageLength @unit(B) = default(512B);
        double sendInterval @unit(s) = default(1s);
        double startTime @unit(s) = default(1s);
        double stopTime @unit(s) = default(-1s);  // -1 means unlimited
        
        // Lifecycle support
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);
        double stopOperationTimeout @unit(s) = default(2s);
        
        // Statistics
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
