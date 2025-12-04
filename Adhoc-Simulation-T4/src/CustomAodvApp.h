//
// CustomAodvApp.h - Custom AODV Application Header
//
#ifndef __ADHOC_SIMULATION_T4_CUSTOMAODVAPP_H_
#define __ADHOC_SIMULATION_T4_CUSTOMAODVAPP_H_

#include <omnetpp.h>
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;
using namespace inet;

/**
 * Custom AODV Application with UDP socket communication.
 * This module provides a simple UDP-based messaging system
 * for custom AODV routers.
 */
class CustomAodvApp : public OperationalBase, public UdpSocket::ICallback
{
  protected:
    // Configuration parameters
    int localPort;
    int destPort;
    L3Address destAddress;
    int messageLength;
    simtime_t sendInterval;
    simtime_t startTime;
    simtime_t stopTime;

    // UDP Socket
    UdpSocket socket;
    
    // Self messages for timing
    cMessage *selfMsg = nullptr;
    
    // Statistics
    int numSent = 0;
    int numReceived = 0;
    
    // Signals for statistics
    simsignal_t packetSentSignal;
    simsignal_t packetReceivedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    
    // Required by OperationalBase
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_APPLICATION_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_APPLICATION_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_APPLICATION_LAYER; }

    // Lifecycle operations
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // UDP Socket callbacks
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // Application logic
    virtual void sendPacket();
    virtual void processPacket(Packet *pk);
    virtual void setSocketOptions();
    
    // Helper methods
    virtual Packet *createPacket();
    virtual void scheduleNextPacket(simtime_t previous);

  public:
    CustomAodvApp() {}
    virtual ~CustomAodvApp();
};

#endif // __ADHOC_SIMULATION_T4_CUSTOMAODVAPP_H_
