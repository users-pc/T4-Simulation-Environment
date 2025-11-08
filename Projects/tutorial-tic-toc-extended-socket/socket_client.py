"""Simple TCP echo server used by the OMNeT++ simulation."""

from __future__ import annotations

import logging
import socketserver
from contextlib import suppress


class EchoRequestHandler(socketserver.BaseRequestHandler):
    """Handles a single client connection and sends back an echo message."""

    def handle(self) -> None:
        data = self.request.recv(1024)
        if not data:
            return

        message = data.decode("utf-8", errors="replace").strip()
        logging.info("received '%s' from %s:%s", message, *self.client_address)

        response = f"Echo vom Python-Server: {message}".encode("utf-8")
        self.request.sendall(response)


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True


def serve(host: str = "127.0.0.1", port: int = 65432) -> None:
    """Starts the TCP server and blocks until interrupted."""

    with ThreadedTCPServer((host, port), EchoRequestHandler) as server:
        logging.info("listening on %s:%s", host, port)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            logging.info("shutdown requested by user")
        finally:
            with suppress(Exception):
                server.shutdown()


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )
    serve()
