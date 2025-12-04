//
// Custom AODV Router with configurable IP and socket connection
//
package adhoc_simulation_t4;

import inet.node.aodv.AodvRouter;
import inet.applications.contract.IApp;

//
// Custom AODV Router that extends the standard AodvRouter
// with custom IP addressing and socket-based communication
//
module CustomAodvRouter extends AodvRouter
{
    parameters:
        @display("i=device/cellphone");
        
        // Custom IP configuration parameters
        string customIpAddress = default("");
        string customNetmask = default("255.255.255.0");
        
        // Socket configuration
        int socketLocalPort = default(5000);
        int socketDestPort = default(5000);
        string socketDestAddress = default("");
        
        // Number of custom apps
        int numCustomApps = default(1);
        
    submodules:
        // Custom UDP application for socket communication
        customApp[numCustomApps]: <default("CustomAodvApp")> like IApp {
            parameters:
                interfaceTableModule = default(absPath(".interfaceTable"));
                @display("p=375,76,row,150");
        }
        
    connections:
        for i=0..numCustomApps-1 {
            customApp[i].socketIn <-- at.out++;
            customApp[i].socketOut --> at.in++;
        }
}
