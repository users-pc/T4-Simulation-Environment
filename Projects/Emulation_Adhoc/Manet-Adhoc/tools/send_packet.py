#!/usr/bin/env python3
"""Minimal UDP test client for the MANET gateway."""

import argparse
import socket
import sys
import time


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Send UDP packets into the MANET and wait for echoes")
    parser.add_argument("dest", help="destination IP inside the MANET (e.g., 10.0.0.2)")
    parser.add_argument("--port", type=int, default=5000, help="destination UDP port")
    parser.add_argument("--payload", default="Hello MANET", help="payload string to send")
    parser.add_argument("--bind", metavar="IP", help="source IP to bind to (use your tap0, e.g., 192.168.100.1)")
    parser.add_argument("--count", type=int, default=1, help="number of packets to send")
    parser.add_argument("--interval", type=float, default=1.0, help="seconds between sends")
    parser.add_argument("--timeout", type=float, default=2.0, help="receive timeout in seconds")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    payload = args.payload.encode()
    dest = (args.dest, args.port)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        if args.bind:
            sock.bind((args.bind, 0))
        sock.settimeout(args.timeout)

        for i in range(args.count):
            sock.sendto(payload, dest)
            try:
                data, src = sock.recvfrom(2048)
                print(f"[{i+1}/{args.count}] reply from {src[0]}:{src[1]} -> {data.decode(errors='replace')}")
            except socket.timeout:
                print(f"[{i+1}/{args.count}] timeout waiting for echo")

            if i + 1 < args.count:
                time.sleep(args.interval)

    return 0


if __name__ == "__main__":
    sys.exit(main())
