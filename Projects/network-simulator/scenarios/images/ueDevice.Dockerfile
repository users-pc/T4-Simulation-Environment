FROM ubuntu:18.04

RUN apt-get update && apt-get install -y \
	iputils-ping 	\
	iproute2     	\
	netcat		\
  traceroute \
  curl \
	&& rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]
