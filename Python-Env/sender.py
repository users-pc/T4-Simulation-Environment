from scapy.all import Ether, IP, ICMP, sendp

pkt = (
    Ether(dst="ff:ff:ff:ff:ff:ff") /
    IP(dst="10.0.0.2") /
    ICMP()
)
sendp(pkt, iface="tap0", verbose=False)
