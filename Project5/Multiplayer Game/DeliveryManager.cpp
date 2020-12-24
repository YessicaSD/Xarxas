#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery::Delivery(uint32 sequenceNumber, double dispatchTime) : sequenceNumber(sequenceNumber), dispatchTime(dispatchTime)
{}

Delivery* DeliveryManagerServer::writeSequenceNumber(OutputMemoryStream& packet)
{
    Delivery* newDelivery = new Delivery(nextSequenceNum, Time.time + PACKET_TIMEOUT_INTERVAL);
    packet << nextSequenceNum;
    nextSequenceNum++;
    pendingDeliveries.push_back(newDelivery);
    return newDelivery;
}

void DeliveryManagerServer::processTimedOutPackets()
{
    auto i = pendingDeliveries.begin();
    while (i != pendingDeliveries.end())
    {
        if ((*i)->dispatchTime < Time.time)
        {
            if ((*i)->delegate != nullptr) {
                (*i)->delegate->onDeliveryFailure(this);
            }
            pendingDeliveries.erase(i);
        }
        else
        {
            i++;
        }
    }
}

bool DeliveryManagerClient::processSequenceNumber(const InputMemoryStream& packet)
{
    uint32 sequenceNumber = 0;
    packet >> sequenceNumber;
    
    pendingAck.push_back(sequenceNumber);

    if (sequenceNumber >= expectedSequenceNum) {
        //send success
        expectedSequenceNum = expectedSequenceNum + 1;

        //if we update a gameobject that has not been created, create it
        return true;
    }
    else if (sequenceNumber < expectedSequenceNum) {
        //only process if it's a destroy
        
        //send success
        return false;
    }
    return false;
}

DeliveryDelegateDestroy::DeliveryDelegateDestroy(Delivery* parent) : DeliveryDelegate::DeliveryDelegate(parent)
{
}

void DeliveryDelegateDestroy::onDeliverySuccess(DeliveryManagerServer* deliveryManager)
{}

void DeliveryDelegateDestroy::onDeliveryFailure(DeliveryManagerServer* deliveryManager)
{
    //ReplicationManagerServer;
    OutputMemoryStream packet;
    ReplicationManagerServer::Write(packet, deliveryManager, parent->indispensableCommands);
}

DeliveryDelegate::DeliveryDelegate(Delivery* parent) : parent(parent)
{
}
