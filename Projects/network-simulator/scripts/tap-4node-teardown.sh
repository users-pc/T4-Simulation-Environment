#!/bin/env bash
# TAP Device Teardown for 4-Node NS-3 Scenarios

set -e

echo "Tearing down..."
docker compose -f scenarios/tap-4pynode-scenario.yaml down 2>/dev/null || true
docker compose -f scenarios/tap-4node-scenario.yaml down 2>/dev/null || true

for i in 0 1 2 3; do
    sudo ip link delete veth-int-$i 2>/dev/null || true
    sudo ip link set tap-$i nomaster 2>/dev/null || true
    sudo ip link delete tap-$i 2>/dev/null || true
    sudo ip link delete br-$i 2>/dev/null || true
done

for f in /var/run/netns/*; do [ -L "$f" ] && sudo rm -f "$f" 2>/dev/null || true; done
echo "Done."
