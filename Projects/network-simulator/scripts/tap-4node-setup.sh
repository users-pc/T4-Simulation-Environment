#!/bin/env bash
# TAP Device Setup for 4-Node NS-3 Scenarios
# Usage: sudo ./scripts/tap-4node-setup.sh [scenario-yaml]

select_scenario() {
    mapfile -t scenarios < <(find scenarios -name "*.yaml" -o -name "*.yml" 2>/dev/null | sort)
    if [ ${#scenarios[@]} -eq 0 ]; then
        echo "No scenarios found in scenarios/"
        exit 1
    fi

    # Find fzf - check common paths
    FZF_CMD=""
    for path in /usr/bin/fzf /usr/local/bin/fzf /home/*/.fzf/bin/fzf; do
        if [ -x "$path" ] 2>/dev/null; then
            FZF_CMD="$path"
            break
        fi
    done

    if [ -n "$FZF_CMD" ]; then
        SCENARIO_FILE=$(printf '%s\n' "${scenarios[@]}" | $FZF_CMD --height=40% --reverse --border --prompt="Select scenario: ")
        if [ -z "$SCENARIO_FILE" ]; then
            echo "No selection made"
            exit 1
        fi
    else
        echo "Select a scenario:"
        for i in "${!scenarios[@]}"; do
            echo "  [$((i+1))] ${scenarios[$i]}"
        done
        read -p "Selection: " sel
        if ! [[ "$sel" =~ ^[0-9]+$ ]] || [ "$sel" -lt 1 ] || [ "$sel" -gt ${#scenarios[@]} ]; then
            echo "Invalid selection"
            exit 1
        fi
        SCENARIO_FILE="${scenarios[$((sel-1))]}"
    fi
}

# Get scenario file
if [ -n "$1" ]; then
    SCENARIO_FILE="$1"
else
    select_scenario
fi

echo "Setting up: $SCENARIO_FILE"

# Create bridges and TAP devices
for i in 0 1 2 3; do
    ip link show br-$i &>/dev/null || sudo ip link add name br-$i type bridge
    ip link show tap-$i &>/dev/null || sudo ip tuntap add tap-$i mode tap
    sudo ifconfig tap-$i 0.0.0.0 promisc up
    sudo ip link set tap-$i master br-$i
    sudo ip link set br-$i up
done

# Disable bridge netfilter
if [ -d /proc/sys/net/bridge ]; then
    pushd /proc/sys/net/bridge >/dev/null
    for f in bridge-nf-*; do echo 0 > $f 2>/dev/null || true; done
    popd >/dev/null
fi

# Start containers
sudo mkdir -p /var/run/netns
docker compose -f "$SCENARIO_FILE" up -d || { echo "Failed to start containers"; exit 1; }
sleep 2

# Get PIDs and create veth pairs
declare -a pids
declare -a macs=("12:34:88:5D:61:B0" "12:34:88:5D:61:B1" "12:34:88:5D:61:B2" "12:34:88:5D:61:B3")

for i in 0 1 2 3; do
    pids[$i]=$(docker inspect --format '{{ .State.Pid }}' node-$i 2>/dev/null)
    if [ -z "${pids[$i]}" ] || [ "${pids[$i]}" = "0" ]; then
        echo "Warning: node-$i not running, skipping"
        continue
    fi

    sudo ln -sf /proc/${pids[$i]}/ns/net /var/run/netns/${pids[$i]}

    # Remove existing veth if present
    sudo ip link delete veth-int-$i 2>/dev/null || true

    sudo ip link add veth-int-$i type veth peer name veth-ext-$i
    sudo ip link set veth-int-$i master br-$i
    sudo ip link set veth-int-$i up
    sudo ip link set veth-ext-$i netns ${pids[$i]}
    sudo ip netns exec ${pids[$i]} ip link set dev veth-ext-$i name eth0
    sudo ip netns exec ${pids[$i]} ip link set eth0 address ${macs[$i]}
    sudo ip netns exec ${pids[$i]} ip link set eth0 up
    sudo ip netns exec ${pids[$i]} ip addr add 10.0.0.$((i+1))/24 dev eth0
    echo "node-$i: 10.0.0.$((i+1))"
done

echo "Done. Run: docker exec ns-3 ./ns3 run scratch/tap-csma-line.cc"
