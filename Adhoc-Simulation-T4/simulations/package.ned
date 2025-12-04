package adhoc_simulation_t4.simulations;

@license(LGPL);
package inet.showcases.visualizer.canvas.routingtable;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.aodv.AodvRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;

network RoutingTableVisualizationDynamicShowcase
{
    @display("bgb=975,635");
    submodules:
        host1: AodvRouter {
            @display("p=360,274");
        }
        host2: AodvRouter {
            @display("p=485,366");
        }
        host3: AodvRouter {
            @display("p=486,198");
        }
        host4: AodvRouter {
            @display("p=218,337");
        }
        host5: AodvRouter {
            @display("p=229,186");
        }

        configurator: Ipv4NetworkConfigurator {
            @display("p=827,186");
        }
        visualizer: IntegratedVisualizer {
            @display("p=827,275");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=827,366");
        }
}
