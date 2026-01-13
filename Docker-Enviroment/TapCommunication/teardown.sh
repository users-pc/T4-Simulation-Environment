#!/bin/bash

docker rm -f left right 2>/dev/null || true
docker network rm br-a br-b 2>/dev/null || true
sudo ip link delete br-a type bridge 2>/dev/null || true
sudo ip link delete br-b type bridge 2>/dev/null || true
sudo ip tuntap del mode tap dev tapa 2>/dev/null || true
sudo ip tuntap del mode tap dev tapb 2>/dev/null || true
sudo rm -f /var/run/netns/* 2>/dev/null || true
