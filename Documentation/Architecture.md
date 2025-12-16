# Architectural Documentation  
## Simulation of Python/Docker Nodes over an OMNeT++-Based Network Environment

## 1. Goal of Architecture 
The goal of this system is to evaluate how distributed applications behave in dynamic network environments with high packet loss, latency, and limited bandwidth.
Python scripts represent application-level nodes, while OMNeT++ is used to simulate the underlying network topology, routing behavior, and network dynamics.
## 2. Systemoverview

### A) Application Layer – Python-Scripts
- Jeder Knoten hat eine IP im Applikationsnetz: 192.168.17.x
- Five Docker containers with IP addresses: 192.168.17.x
- Each container:
  - Opens a socket on port 5000
  - Runs a Python script
  - Periodically sends data to other nodes
  - Receives application-level messages
### B) Simulation Layer – OMNeT++ Hosts
- Jeder Host hat eine IP: 192.168.18.x
- Öffnet fünf Ports: 5001–5005
- Empfängt, prüft und leitet Pakete weiter
- Simuliert ein dynamisches Ad-hoc/MANET-Netz

## 3. Architektur 
<img width="1694" height="950" alt="image" src="https://github.com/user-attachments/assets/e8b82f3d-566c-40dd-bc07-772e5fa370bc" />

## Kommt noch

## 5. Weiterleitung innerhalb OMNeT++
OMNeT++ Hosts lauschen auf Ports 5001–5005.

### Entscheidung:
- **Fall 1: Paket gehört zum zugeordneten Python-Knoten**  
  → Weiterleitung an `192.168.17.x:5000`.
- **Fall 2: Paket gehört nicht dazu**  
  → Weiterleitung an alle Nachbarn über denselben Port (Flooding).

## 6. Vermeidung von Routing-Loops

### Option A: Eigene Duplicate-Check-Logik
- Nachrichten-IDs speichern  
- Pakete nur einmal weiterleiten

### Option B: Einsatz eines MANET-Protokolls (OSI Layer 3)
- RIP  
- 6LoWPAN  
- AODV  
- OLSR  

OMNeT++/INET kann solche Protokolle unterstützen.

## 7. Bezug zum OSI-Modell

- **Layer 1–2:** Simulation durch OMNeT++ (Funkreichweite, Kollisionen)
- **Layer 3:** Routing und Forwarding (simuliert)
- **Layer 4:** Python nutzt TCP/UDP
- **Layer 7:** Python-Skripte erzeugen/empfangen Daten

## 8. Vorteile der Architektur
- Realistische Simulation eines MANET/Ad-hoc-Netzes  
- Python-Anwendungen bleiben unverändert  
- Gute Skalierbarkeit  
- Klare Trennung zwischen Applikation (Python) und Netzwerk (OMNeT++)  
- Unterstützt Delay, Loss, Mobility, Bandwidth-Limits  

## 9. Zusammenfassung
Die Architektur verbindet reale Python-Anwendungen mit einer simulationsbasierten Netzwerkumgebung. OMNeT++ übernimmt Routing, Nachbarschaftsbeziehungen und dynamische Topologie. Python kümmert sich um die Anwendung und erzeugt echten Datenverkehr.




