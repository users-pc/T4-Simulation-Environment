set -e

# create TAP interfaces
sudo ip tuntap add mode tap dev tapa
sudo ip tuntap add mode tap dev tapb

sudo ip link set tapa promisc on
sudo ip link set tapb promisc on

# assign IP addresses to interfaces
# wrong ?
# sudo ip addr add 192.168.2.30/24 dev tapa
# sudo ip addr add 192.168.3.30/24 dev tapb

# bring up all interfaces
sudo ip link
set dev tapa up
sudo ip link set dev tapb up

# create netwrok bridges for connecting container to tap device
sudo ip link add name br-1 type bridge
sudo ip link add name br-2 type bridge
sudo ip link set dev br-1 up
sudo ip link set dev br-2 up

# connect tap devices to bridges
sudo ip link set tapa master br-1
sudo ip link set tapb master br-2

# verify with ip addr show
# that the ips being used

docker network rm br-1 br-2 2>/dev/null || true

docker network create \
  -o com.docker.network.bridge.name=br-1 \
  br-1

docker network create \
  -o com.docker.network.bridge.name=br-2 \
  br-2

# allow frames to forward
sudo iptables -I FORWARD -m physdev --physdev-is-bridged -i br-1 -p tcp -j ACCEPT
sudo iptables -I FORWARD -m physdev --physdev-is-bridged -i br-2 -p tcp -j ACCEPT

docker run -dit --name left  --network br-1 nicolaka/netshoot
docker run -dit --name right --network br-2 nicolaka/netshoot

# get pid cfrom container
pid_left=$(docker inspect --format '{{ .State.Pid }}' left)
pid_right=$(docker inspect --format '{{ .State.Pid }}' right)
echo 1;
# create netns links
sudo mkdir -p /var/run/netns
sudo ln -s /proc/$pid_left/ns/net /var/run/netns/$pid_left
sudo ln -s /proc/$pid_right/ns/net /var/run/netns/$pid_right

sudo ip netns exec $pid_left ip link set dev eth0 address 12:34:88:5D:61:BD
sudo ip netns exec $pid_right ip link set dev eth0 address 12:34:88:5D:61:BE

# set up local connection
#
# ip link add veth-link-a type veth peer name veth-link-b
# ip link set veth-link-a up
# ip link set veth-link-b up
# ip link set veth-link-a master br-1
# ip link set veth-link-b master br-2
# -- end

sudo ip link add name br-connection type bridge
sudo ip link set dev br-connection up
sudo ip link set tapa master br-connection
sudo ip link set tapb master br-connection
