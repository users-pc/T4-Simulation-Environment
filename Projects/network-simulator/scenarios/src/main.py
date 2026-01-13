import sys
import re
import asyncio
import signal
import random
import socket
import time

# Global variables - ip will be set in main() after network is ready
ip = "0.0.0.0"
peers = []

def get_local_ip(max_retries=30, retry_delay=2):
    """
    Get local IP address. Retries if network isn't ready yet.
    This is needed for NS-3 TAP bridge scenarios where the network
    namespace is attached after the container starts.
    """
    for attempt in range(max_retries):
        try:
            # Try to get IP from hostname
            hostname = socket.gethostname()
            ip = socket.gethostbyname(hostname)
            if ip and ip != "127.0.0.1":
                return ip
        except socket.gaierror:
            pass

        # Try to get IP by connecting to external address (doesn't actually connect)
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("10.0.0.1", 80))
            ip = s.getsockname()[0]
            s.close()
            if ip and ip != "127.0.0.1":
                return ip
        except:
            pass

        print(f"[!] Network not ready, retrying in {retry_delay}s... ({attempt+1}/{max_retries})", flush=True)
        time.sleep(retry_delay)

    # Fallback to hostname if we can't get IP
    print("[!] Could not determine IP, using hostname as identifier", flush=True)
    return socket.gethostname()

async def process(data, addr):
  print(f"[< {int(asyncio.get_event_loop().time())}]: {data.decode(errors='ignore')}", flush=True)

async def handle_client(reader, writer):
  addr = writer.get_extra_info('peername')
  print(f"[+] Connected by {addr}", flush=True)
  try:
    while True:
      data = await reader.read(4096)
      if not data:
        break
      await process(data, addr)
  except asyncio.CancelledError:
    pass
  finally:
    writer.close()
    await writer.wait_closed()
    print(f"[-] Connection closed {addr}", flush=True)

async def send_messages(peers=[],port=5000):
  while True:
    wait_time = 2 #random.uniform(1, 5) # 1 bis 5 Sekunden zufällig
    await asyncio.sleep(wait_time)
    if len(peers)>0:
      try:
        host=peers[int(random.uniform(0, len(peers)))].split(":")
        reader, writer = await asyncio.open_connection(host[0], int(host[1]))
        message = f"Hello from {ip}:{port} to {host[0]}:{host[1]}"
        writer.write(message.encode())
        await writer.drain()
        writer.close()
        await writer.wait_closed()
        print(f"[> {int(asyncio.get_event_loop().time())}]: {message}", flush=True)
      except ConnectionRefusedError:
        print(f"[!] Could not connect to {host[0]}:{host[1]}", flush=True)
    else:
      print("No peers specified", flush=True)

async def main(host="0.0.0.0", port=5000):
  global ip

  # Wait for network to be ready (needed for NS-3 TAP bridge scenarios)
  ip = get_local_ip()
  print(f"[+] Network ready, IP: {ip}", flush=True)

  for param in sys.argv[1:]:
    if re.search("^--?peers?=", param):
      peers=re.findall("(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?):\d{1,5}", param)
    if re.search("^--?port=\d{1,5}", param):
      port=int(re.findall("\d{1,5}", param)[0])

  server = await asyncio.start_server(handle_client, host, port)
  addr = server.sockets[0].getsockname()
  # print(f"[+] Asyncio server listening on {addr}", flush=True)
  print(f"Hello, here is {socket.gethostname()} listening on {ip}:{port}", flush=True)

  loop = asyncio.get_running_loop()
  stop_event = asyncio.Event()

  def shutdown():
      print("\n[!] Shutdown signal received. Closing server...", flush=True)
      stop_event.set()

  # loop.add_signal_handler(signal.SIGINT, shutdown)
  # loop.add_signal_handler(signal.SIGTERM, shutdown)

  async with server:
    print(f"Peers: {peers}", flush=True)
    sender_task = asyncio.create_task(send_messages(peers=peers, port=port))
    await stop_event.wait()
    sender_task.cancel()
    await asyncio.gather(sender_task, return_exceptions=True)
    server.close()
    await server.wait_closed()


if __name__ == "__main__":
  # print(f"Hello, here is {socket.gethostname()}", flush=True)
  try:
    asyncio.run(main())
  except KeyboardInterrupt:
    print("[!] Server stopped manually", flush=True)
