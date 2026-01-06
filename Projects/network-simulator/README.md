# Network Simulator
This is the source repository for creating the experiments that will be run on ns-3. The developer codes an ns-3 simulation, creates a docker-compose file to setup the necessary containers and volumes, and executes using a script that both sets up the required linux networking interfaces on host and connects them to the running containers and simulation.

## Quickstart
Ensure you have docker compose installed in your system.
Run `docker compose -f scenarios/hello-world-scenario.yaml up`

To run other scenarios, such as the tap-csma-scenario, you must instead use a setup script such as `/scripts/setup.sh`. To teardown everything run the corresponding teardown script `/scripts/teardown.sh`.

## Project structure
### images
Contains the docker images used to create user-end devices, master nodes, ns-3 simulation, and any other kind of node that a simulation requires. Images are currently stored in the local workstation and need to be uploaded to the image registry so others can use.

### scenarios
These are docker compose files that set up containers and volumes for a specific experiment, or "scenario." This allows for rapid deployment and teardowns across different workstations.

### src
Contains the ns-3 development code that creates the simulation. Note that the development file and docker compose file names coincide for readability. ALL ns-3 code should reside here, since the docker compose scenario files use this location as a mount for setting up the ns-3 simulation. Internally for ns-3, code is mounted on `<ns-3 installation>/scratch/`, which is an ns-3 specific location for development.

### scripts
Contains the scripts that actually run a scenario. Scripts set up host networking interfaces, start docker compose scenarios and connect these interfaces to the newly created containers. Scripts also exist to quickly teardown all devices and containers.

Starting a script:

Default (uses tap-4pynode-scenario.yaml)

`sudo ./scripts/tap-4node-setup.sh`

Or specify any 4-node scenario
`sudo ./scripts/tap-4node-setup.sh scenarios/tap-4node-scenario.yaml`

### Usage
Run `docker exec ns-3 ./ns3 run scratch/tap-csma-scenario.cc` to run the simulation.

## Development
ns-3 development files are available in `src` folder. They are mounted as a volume when `docker compose` is called for the appropiate scenario. **Only perform development on this folder**.

