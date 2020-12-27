#include "Networks.h"
#include "DeliveryManager.h"
#include <algorithm>

// TODO(you): Reliability on top of UDP lab session

Delivery::Delivery(uint32 sequenceNumber, double dispatchTime) : sequenceNumber(sequenceNumber), dispatchTime(dispatchTime)
{}

Delivery* DeliveryManagerServer::writeSequenceNumber(OutputMemoryStream& packet)
{

    Delivery* newDelivery = new Delivery(nextSequenceNum, Time.time + PACKET_TIMEOUT_INTERVAL);
    packet << nextSequenceNum;
    this->nextSequenceNum += 1;
    pendingDeliveries.push_back(newDelivery);
    return newDelivery;
}

void DeliveryManagerServer::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
    int numberOfPacketRecived = 0;
    packet >> numberOfPacketRecived;
    for (int i = 0; i < numberOfPacketRecived; i++)
    {
        uint32 packetResivedIndex;
        packet >> packetResivedIndex;
        if (pendingDeliveries.size() > 0)
        {
            auto iter = std::find_if(pendingDeliveries.begin(), pendingDeliveries.end(),
                        [packetResivedIndex](Delivery* d) {return d->sequenceNumber == packetResivedIndex; });
            
            if (iter != pendingDeliveries.end())
            {
                if ((*iter)->delegate != nullptr)
                {
                    (*iter)->delegate->onDeliverySuccess(this);
                }
                delete (*iter);
                pendingDeliveries.erase(iter);
            }
        }
    }
}

void DeliveryManagerServer::processTimedOutPackets()
{
    for (int i = pendingDeliveries.size() - 1; i >= 0; --i)
    {
        if (pendingDeliveries[i]->dispatchTime < Time.time)
        {
            if (pendingDeliveries[i]->delegate != nullptr) {
                pendingDeliveries[i]->delegate->onDeliveryFailure(this);
            }

            delete pendingDeliveries[i];
            pendingDeliveries.erase(pendingDeliveries.begin() + i);
        }
    }
}

bool DeliveryManagerClient::processSequenceNumber(const InputMemoryStream& packet)
{
    uint32 sequenceNumber = 0;
    packet >> sequenceNumber;
    
    pendingAck.push_back(sequenceNumber);

    if (sequenceNumber >= expectedSequenceNum) {
        expectedSequenceNum = expectedSequenceNum + 1;
        //send success
        //if we update a gameobject that has not been created, create it
        return true;
    }
    else if (sequenceNumber < expectedSequenceNum) {
        return false;
        //send success
        return false;
    }
    return false;
}

bool DeliveryManagerClient::hasSequenceNumbersPendingAck() const
{
    return pendingAck.size()!=0;
}

void DeliveryManagerClient::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
    packet << (int)pendingAck.size();
    for (auto i = pendingAck.begin(); i < pendingAck.end(); i++)
    {
        packet << (*i);
    }
    pendingAck.clear();
}

DeliveryDelegate::DeliveryDelegate(Delivery* parent)
    : parent(parent)
{}

DeliveryDelegateDestroy::DeliveryDelegateDestroy(Delivery* parent)
    : DeliveryDelegate::DeliveryDelegate(parent)
{}

void DeliveryDelegateDestroy::onDeliverySuccess(DeliveryManagerServer* deliveryManager)
{}

void DeliveryDelegateDestroy::onDeliveryFailure(DeliveryManagerServer* deliveryManager)
{
    OutputMemoryStream packet;
    // ReplicationManagerServer::Write(packet, deliveryManager, parent->indispensableCommands);
    //TODO: send
}
