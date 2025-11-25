# T4-Simulation-Environment
authors: Rayan Ben Tanfous, Ilhan Kapcik, Daniel Yu
![Alt-Text](https://upload.wikimedia.org/wikipedia/commons/1/1a/Freifunk_mesh_cloud.png)

## Project Goal

The T4-Simulation-Environment is a network simulation platform built on OMNeT++ and the INET framework, designed to bridge the gap between simulated network environments and real-world applications.
The goal of this project is to design an architecture, where the network communication and the real-world application are isolated from each other 

### Architecture

The project implements a **socket-based bridge** between two distinct domains:

1. **Simulation domain (OMNeT++/INET)**
   - Network simulation using INET framework
   - Simulated nodes, links, and network protocols

2. **Real-World application domain**
   - Actual application programs executed by python script or docker container

3. **Integration layer**
	- using socket proxies

## Project Structure

```
T4-Simulation-Environment/
├── Projects/
│   ├── tutorial-tic-toc-extended/          # Basic OMNeT++ simulation
│   └── tutorial-tic-toc-extended-socket/   # Socket-enabled simulation with external connectivity
└── README.md
```

## Getting Started

The easiest way to get started is by installing all frameworks with a package manager. OMNeT++ comes with its own solution called `opp_env`.

### Prerequisites

- Python 3.x
- pip3
- Nix package manager (will be installed in Step 1 if not present)

### Installation Steps

#### Step 1: Install opp_env

If you already have `opp_env` installed and can run it from the command line, you can skip this step. Otherwise, install the required dependencies:

```bash
curl -L https://nixos.org/nix/install | sh
pip3 install opp-env
```

#### Step 2: Create workspace und use the framework

Create a workspace for OMNeT++ and all its frameworks. Navigate to a folder of your choice, then run:

```bash
opp_env init
```

This creates your workspace in the current folder and sets up hidden configuration files. It is recommended not to move the workspace folder after initialization, as this can cause side effects.

Install all important frameworks including INET and Simu5G:

```bash
opp_env install simu5g-latest
```

This will automatically install OMNeT++, INET, and Simu5G frameworks.

All programs must be executed within the `opp_env` shell. Activate the environment:

```bash
opp_env shell
```

Once inside the `opp_env` shell, you can run various commands:

```bash
omnetpp   # Start the OMNeT++ IDE
build-all # Rebuild the frameworks and IDE
check-all # Check the binaries of the programs
clean-all # Clean all build artifacts
```

### Running the Simulations

After installation, navigate to one of the project directories and follow the instructions in their respective README files:

- `Projects/tutorial-tic-toc-extended/` - Basic OMNeT++ simulation
- `Projects/tutorial-tic-toc-extended-socket/` - Socket-enabled simulation with external connectivity

---

For questions or contributions, please contact the authors.
