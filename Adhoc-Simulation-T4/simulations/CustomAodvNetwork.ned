//
// Custom AODV Network Demo
//
package adhoc_simulation_t4.simulations;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;
import adhoc_simulation_t4.CustomAodvRouter;

//
// Demo network showcasing CustomAodvRouter with custom IP and socket connections
//
network CustomAodvNetwork
{
    parameters:
        @display("bgb=800,600");
        
    submodules:
        // Custom AODV Routers with manual IP configuration
        node1: CustomAodvRouter {
            @display("p=150,300");
        }
        node2: CustomAodvRouter {
            @display("p=300,200");
        }
        node3: CustomAodvRouter {
            @display("p=450,300");
        }
        node4: CustomAodvRouter {
            @display("p=300,400");
        }
        node5: CustomAodvRouter {
            @display("p=600,300");
        }
        
        // Network configuration and visualization
        configurator: Ipv4NetworkConfigurator {
            @display("p=700,50");
        }
        visualizer: IntegratedVisualizer {
            @display("p=700,130");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=700,210");
        }
}
