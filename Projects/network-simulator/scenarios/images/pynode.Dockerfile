FROM ubuntu:18.04

RUN apt-get update && apt-get install -y \
  iputils-ping 	\
  iproute2     	\
  netcat		\
  traceroute \
  curl \
  python3.10 \
  python3-pip \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

# COPY requirements.txt ./
# RUN pip install --no-cache-dir asyncio==3.4.3

COPY src/main.py .


# CMD ["/bin/bash"]
ENTRYPOINT ["tail"]
CMD [ "-f", "/dev/null" ]
