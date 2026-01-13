FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt update \
    && apt install git g++ python3 cmake make tar wget libc6-dev sqlite sqlite3 libsqlite3-dev vim -y \
    && rm -rf /var/lib/apt/lists/*

# Install ns-3
RUN cd /usr/local && \
    wget https://www.nsnam.org/release/ns-allinone-3.37.tar.bz2 && \
    tar xjf ns-allinone-3.37.tar.bz2

# Build ns-3
WORKDIR /usr/local/ns-allinone-3.37/ns-3.37

RUN ./ns3 configure --enable-examples --enable-tests \
    && ./ns3 build

# Install 5G Lena project and rebuild
RUN cd contrib && \
    git clone https://gitlab.com/cttc-lena/nr.git && \
    cd nr && \
    git checkout 5g-lena-v2.3.y \
    && cd ../../  \
    && ./ns3 configure --enable-examples --enable-tests \
    && ./ns3 build

# Test installation is successful
RUN ./ns3 run test.py
