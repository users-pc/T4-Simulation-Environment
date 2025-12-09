## Architectural Documentation  
### Simulation von Python-/Docker-Knoten über eine OMNeT++-basierte Netzwerkumgebung

## 1. Ziel der Architektur
Das System dient dazu zu testen, wie verteilte Anwendungen sich in dynamischen Netzwerken mit hoher  Paketverlusten, Verzögerungen und limitierter Bandbreite verhalten.  
Python-Skripte bilden Applikationsknoten, während OMNeT++ die Netzwerkstruktur simuliert.

## 2. Systemüberblick

### A) Application Layer – Python-Skripte 
- Jeder Knoten hat eine IP im Applikationsnetz: **192.168.17.x**
- Öffnet einen Socket auf **Port 5000**
- Sendet periodisch Daten an andere Knoten
- Empfängt Nachrichten vom OMNeT++-Host

### B) Simulation Layer – OMNeT++ Hosts
- Jeder Host hat eine IP: **192.168.18.x**
- Öffnet **fünf Ports**: 5001–5005
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



