# Ad-hoc-emulation-Environment (T4)
**Authors:** Rayan Ben Tanfous, Ilhan Kapcik, Daniel Yu

<img width="890" height="503" alt="t4-simulation" src="https://github.com/user-attachments/assets/b7a016d4-739d-4e19-a7ce-547e598c6064" />

## Project Overview

### The Problem
Decentralized applications like peer-to-peer chat systems rely on stable network infrastructure. This project tests whether such applications work reliably in ad hoc networks with high churn, bandwidth limitations, and delays.

**Solution Approach:** A emulation environment that:
- Provides network interfaces with assigned IP addresses
- Simulates data traffic between Docker containers
- Models realistic network behavior (churn, latency, bandwidth constraints)

### Architecture

The emulation environment is built on **OMNeT++ (or NS-3)** with the **INET Framework** and integrates network emulation with real applications (Python scripts, Docker containers):

```
┌─────────────────────────────────────┐
│  Docker Container / Python Script   │
│      (Decentralized Application)    │
└────────────┬────────────────────────┘
             │ 
         Tap-Interfaces
             ↓
┌─────────────────────────────────────┐
│ OMNeT++ or NS3 Emulation Environment│
│  - Ad hoc Network (INET Framework)  │
│  - Simulate Network Effects         │
│                                     │
└─────────────────────────────────────┘
             │
             ↓ Network emulation
    [Node A] ←→ [Node B] ←→ [Node C]
```

## Getting Started

### Prerequisites
- OMNeT++ 5.x or higher
- INET Framework
- Python 3.x (for client testing)
- NS3
- Linux OS (Tap Interfaces)

### Installation & Execution

1. **Open project:**
   ```bash
   cd Projects/
   ```
2. **Read Instructions:**
   ```bash
   cat Readme.md
   ```
# Info 
The project is not yet functional and is still under development. Under Projects, we have many tutorials that are not directly related to the problem.  

## Further Resources

- [OMNeT++ Documentation](https://omnetpp.org/intro/)
- [INET Framework Guide](http://inet.omnetpp.org/)
- [NS-3](https://www.nsnam.org/)
