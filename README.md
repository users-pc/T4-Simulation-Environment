# Ad-hoc-Simulation-Environment (T4)
**Authors:** Rayan Ben Tanfous, Ilhan Kapcik, Daniel Yu

<img width="890" height="503" alt="t4-simulation" src="https://github.com/user-attachments/assets/b7a016d4-739d-4e19-a7ce-547e598c6064" />

## Project Overview

### The Problem
Decentralized applications like peer-to-peer chat systems rely on stable network infrastructure. This project tests whether such applications work reliably in ad hoc networks with high churn, bandwidth limitations, and delays.

**Solution Approach:** A simulation environment that:
- Provides network interfaces with assigned IP addresses
- Simulates data traffic between Docker containers
- Models realistic network behavior (churn, latency, bandwidth constraints)

### Architecture

The simulation environment is built on **OMNeT++** with the **INET Framework** and integrates network simulation with real applications (Python scripts, Docker containers):

```
┌─────────────────────────────────────┐
│  Docker Container / Python Script   │
│      (Decentralized Application)    │
└────────────┬────────────────────────┘
             │ API / Socket-Interface
             ↓
┌─────────────────────────────────────┐
│    OMNeT++ Simulation Environment   │
│  - Ad hoc Network (INET Framework)  │
│  - Socket API for Communication     │
│  - Simulate Network Effects         │
└─────────────────────────────────────┘
             │
             ↓ Network Simulation
    [Node A] ←→ [Node B] ←→ [Node C]
```

## Project Structure

```
Projects/
├── tutorial-tic-toc-extended/          # Basic Network Simulation
│   ├── tutorial.ned                    # Network Topology
│   ├── txc1.cc                         # Node Program
│   ├── AppPacket.msg                   # Message Format
│   └── omnetpp.ini                     # Simulation Configuration
│
└── tutorial-tic-toc-extended-socket/   # Socket-based Integration
    ├── tutorial.ned                    # Network with Socket Support
    ├── txc1.cc                         # Nodes with Socket API
    ├── AppPacket.msg                   # Application Message Format
    ├── socket_client.py                # Python Client for Testing
    └── omnetpp.ini                     # Simulation Configuration
```

## Getting Started

### Prerequisites
- OMNeT++ 5.x or higher
- INET Framework
- Python 3.x (for client testing)

### Installation & Execution

1. **Open project:**
   ```bash
   cd Projects/tutorial-tic-toc-extended
   ```

2. **Compile:**
   - In OMNeT++ IDE: `Project → Build Project`

3. **Start simulation:**
   - Select run configuration in `omnetpp.ini`
   - Run simulation (Run)

4. **Test with Python client** (Socket variant):
   ```bash
   python socket_client.py
   ```

## Next Steps

- [ ] Extend INET Framework for ad hoc network models
- [ ] Implement API for external applications
- [ ] Realize Docker integration
- [ ] Test network effects (churn, latency, bandwidth)
- [ ] Evaluate with decentralized applications

## Further Resources

- [OMNeT++ Documentation](https://omnetpp.org/intro/)
- [INET Framework Guide](http://inet.omnetpp.org/)
- [Socket Support in INET](https://inet.omnetpp.org/docs/developers-guide/ch-sockets.html)
- [Documentation](./Documentation/)

