## Branch Tutorial-Tic-Toc-Extended: 
Eine kleine OMNeT++-Simulation mit einer einfachen Tictoc-Kette (Sender → Middlemen → Empfänger).

### Wichtige Komponenten

- Nachrichtentyp: AppPacket — AppPacket.msg
- Modul-/Logik-Implementierung: Txc1 — txc1.cc
- NED-Netzwerk: TictocExtended und das einfache Modul Txc1 — tutorial.ned
- Laufkonfiguration: omnetpp.ini

### Kurzanleitung zum Ausführen

1. Projekt in OMNeT++ öffnen.
2. Quellcode kompilieren (Build).
3. Simulation starten; Konfiguration wird in omnetpp.ini gesetzt (Netzwerk: TictocExtended).