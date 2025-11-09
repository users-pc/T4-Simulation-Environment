#include <omnetpp.h>
#include <cstring>
#include <string> // statt <string.h> in C++
#include "AppPacket_m.h"        // aus der .msg generiert
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

bool connectAndEcho(const std::string& server_ip, int server_port, const std::string& message)
{
    // 1️⃣ Socket erstellen (IPv4, TCP)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return false;
    }

    // 2️⃣ Server-Adresse vorbereiten
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Ungültige IP-Adresse\n";
        close(sock);
        return false;
    }

    // 3️⃣ Verbindung herstellen
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return false;
    }

    std::cout << "✅ Verbunden mit " << server_ip << ":" << server_port << "\n";

    // 4️⃣ Nachricht senden
    ssize_t sent = send(sock, message.c_str(), message.size(), 0);
    if (sent < 0) {
        perror("send");
        close(sock);
        return false;
    }

    // 5️⃣ Antwort empfangen
    char buffer[1024] = {0};
    ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        perror("recv");
        close(sock);
        return false;
    }

    buffer[n] = '\0';
    std::cout << "💬 Antwort vom Server: " << buffer << "\n";

    // 6️⃣ Verbindung schließen
    close(sock);
    return true;
}

using namespace omnetpp;

class Txc1 : public cSimpleModule
{
  protected:
    int address;                // eigene Adresse aus NED-Parameter

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Txc1);

void Txc1::initialize()
{
    // random destination
    srand(time(nullptr));
    int rDest = rand() % 11 + 1;


    address = par("address");   // 0 für tic, 1 für toc

    if (strcmp(getName(), "tic") == 0) {
        auto *pkt = new AppPacket("hello");
        pkt->setSrc(address);
        pkt->setDest(rDest);
        pkt->setPayload("Das ist die Nachricht, die versendet wird");

        // Länge korrekt setzen:
        size_t bytes = std::strlen(pkt->getPayload());
        pkt->setByteLength(bytes);              // oder: pkt->setBitLength(8*bytes);

        send(pkt, "out");
    }
}

void Txc1::handleMessage(cMessage *msg)
{
    auto *pkt = check_and_cast<AppPacket*>(msg);

    if (pkt->getDest() == address) {
        EV << "Packet angekommen: src=" << pkt->getSrc()
           << " dest=" << pkt->getDest()
           << " hops=" << pkt->getHopCount()
           << " payload=\"" << pkt->getPayload() << "\"\n";
        // Echo in eigenem Thread starten
        std::thread t([](){
            connectAndEcho("127.0.0.1", 65432, "Hallo vom C++-Client!");
        });
        t.detach();

        delete pkt;
    } else {
        // Weiterleiten (Beispiel: simple forward)
        pkt->setHopCount(pkt->getHopCount() + 1);
        send(pkt, "out");
    }
}









