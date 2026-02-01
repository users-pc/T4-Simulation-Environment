# TapAdhoc

This project rebuilds and extends the INET video streaming emulation showcase: https://inet.omnetpp.org/docs/showcases/emulation/videostreaming/doc/index.html. Our focus is an ad-hoc Wi-Fi scenario that bridges OMNeT++ nodes to the host via TAP so real traffic (for example, video streams) can be exchanged in real time.

Goals
- Reproduce the baseline video streaming emulation and keep the setup reproducible for newcomers.
- Create an ad-hoc wireless topology and verify packet exchange between simulated nodes and the host through TAP.
- Provide a clear entrypoint for extending scenarios with additional nodes or traffic patterns.

How to run
- The key commands, environment assumptions, and launch steps live in simulations/run (bash script); read and execute it to start the emulation.
- For background and prerequisites, follow the  [Emulation-Env wiki](https://github.com/users-pc/T4-Simulation-Environment/wiki/Emulation%E2%80%90Env-Introduction) referenced in our team notes.

