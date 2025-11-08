#include <omnetpp.h>
#include <cstring>              // statt <string.h> in C++
#include "AppPacket_m.h"        // aus der .msg generiert
#include <cstdlib>
#include <ctime>

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
        delete pkt;
    } else {
        // Weiterleiten (Beispiel: simple forward)
        pkt->setHopCount(pkt->getHopCount() + 1);
        send(pkt, "out");
    }
}
