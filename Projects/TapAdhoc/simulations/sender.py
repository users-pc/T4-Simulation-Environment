from scapy.all import Ether, IP, ICMP, sendp

pkt = (
    Ether(dst="ff:ff:ff:ff:ff:ff") /
    IP(dst="192.168.3.20") /
    ICMP()
)
sendp(pkt, iface="tapa", verbose=False)