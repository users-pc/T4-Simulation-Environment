//
// CustomAodvApp.cc - Custom AODV Application Implementation
//
#include "CustomAodvApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

using namespace inet;

Define_Module(CustomAodvApp);

CustomAodvApp::~CustomAodvApp()
{
    cancelAndDelete(selfMsg);
}

void CustomAodvApp::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // Read parameters
        localPort = par("localPort");
        destPort = par("destPort");
        messageLength = par("messageLength");
        sendInterval = par("sendInterval");
        startTime = par("startTime");
        stopTime = par("stopTime");

        // Initialize statistics
        numSent = 0;
        numReceived = 0;
        packetSentSignal = registerSignal("packetSent");
        packetReceivedSignal = registerSignal("packetReceived");

        // Create self message for timing
        selfMsg = new cMessage("sendTimer");

        EV_INFO << "CustomAodvApp initialized at " << getParentModule()->getFullPath() << endl;
    }
}

void CustomAodvApp::handleStartOperation(LifecycleOperation *operation)
{
    // Resolve destination address
    const char *destAddressStr = par("destAddress");
    if (strlen(destAddressStr) > 0) {
        L3AddressResolver().tryResolve(destAddressStr, destAddress);
        if (destAddress.isUnspecified()) {
            EV_WARN << "Cannot resolve destination address: " << destAddressStr << endl;
        }
    }

    // Setup UDP socket
    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    
    const char *localAddressStr = par("localAddress");
    L3Address localAddr;
    if (strlen(localAddressStr) > 0) {
        L3AddressResolver().tryResolve(localAddressStr, localAddr);
    }
    
    socket.bind(localAddr, localPort);
    setSocketOptions();

    EV_INFO << "CustomAodvApp started, bound to port " << localPort << endl;

    // Schedule first packet if destination is configured
    if (!destAddress.isUnspecified() && sendInterval > 0) {
        simtime_t start = std::max(startTime, simTime());
        scheduleAt(start, selfMsg);
        EV_INFO << "First packet scheduled at " << start << endl;
    }
}

void CustomAodvApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    EV_INFO << "CustomAodvApp stopped" << endl;
}

void CustomAodvApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.destroy();
}

void CustomAodvApp::setSocketOptions()
{
    socket.setBroadcast(true);
    // Set TTL for multi-hop routing
    socket.setTimeToLive(32);
}

void CustomAodvApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // Time to send a packet
        sendPacket();
        
        // Check if we should continue sending
        if (stopTime < SIMTIME_ZERO || simTime() < stopTime - sendInterval) {
            scheduleNextPacket(simTime());
        }
    }
    else if (socket.belongsToSocket(msg)) {
        socket.processMessage(msg);
    }
    else {
        throw cRuntimeError("Unknown message received: %s", msg->getName());
    }
}

void CustomAodvApp::scheduleNextPacket(simtime_t previous)
{
    simtime_t next = previous + sendInterval;
    if (stopTime < SIMTIME_ZERO || next < stopTime) {
        scheduleAt(next, selfMsg);
    }
}

Packet *CustomAodvApp::createPacket()
{
    char msgName[64];
    sprintf(msgName, "CustomAodvPkt-%d", numSent);
    
    Packet *packet = new Packet(msgName);
    
    // Create payload with timestamp
    const auto& payload = makeShared<ByteCountChunk>(B(messageLength));
    packet->insertAtBack(payload);
    
    // Add creation time tag for latency measurement
    packet->addTag<CreationTimeTag>()->setCreationTime(simTime());
    
    return packet;
}

void CustomAodvApp::sendPacket()
{
    Packet *packet = createPacket();
    
    EV_INFO << "Sending packet " << packet->getName() 
            << " to " << destAddress << ":" << destPort << endl;
    
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddress, destPort);
    numSent++;
}

void CustomAodvApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    EV_INFO << "Received packet: " << packet->getName() << endl;
    processPacket(packet);
}

void CustomAodvApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    numReceived++;
    
    // Calculate end-to-end delay if timestamp is available
    auto creationTimeTag = pk->findTag<CreationTimeTag>();
    if (creationTimeTag != nullptr) {
        simtime_t delay = simTime() - creationTimeTag->getCreationTime();
        EV_INFO << "Packet delay: " << delay << endl;
    }
    
    EV_INFO << "Packet " << pk->getName() << " received, total received: " << numReceived << endl;
    
    delete pk;
}

void CustomAodvApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Socket error: " << indication->getName() << endl;
    delete indication;
}

void CustomAodvApp::socketClosed(UdpSocket *socket)
{
    EV_INFO << "Socket closed" << endl;
}

void CustomAodvApp::refreshDisplay() const
{
    char buf[64];
    sprintf(buf, "sent: %d rcvd: %d", numSent, numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}

void CustomAodvApp::finish()
{
    OperationalBase::finish();
    
    EV_INFO << "CustomAodvApp finished: sent=" << numSent 
            << " received=" << numReceived << endl;
    
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
}
