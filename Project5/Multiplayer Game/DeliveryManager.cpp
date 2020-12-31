#include "Networks.h"
#include "DeliveryManager.h"
#include <algorithm>

// TODO(you): Reliability on top of UDP lab session

Delivery::Delivery(uint32 sequenceNumber, double dispatchTime) : sequenceNumber(sequenceNumber), dispatchTime(dispatchTime)
{}

Delivery::~Delivery()
{
    indispensableCommands.clear();
}

void Delivery::onDeliverySucess(int clientIndex)
{}

void Delivery::onDeliveryFailure(int clientIndex)
{
    bool sendPacket = false;

    for (int i = indispensableCommands.size() - 1; i >= 0; --i) {
        if (indispensableCommands[i].action == ReplicationAction::Destroy) {
            if (App->modLinkingContext->getNetworkGameObject(indispensableCommands[i].networkId) == nullptr) {
                sendPacket = true;
            }
            else {
                //If  a new gameobject has been created on that position, don't send the delete
                indispensableCommands.erase(indispensableCommands.begin() + i);
            }
        }
        else if (indispensableCommands[i].action == ReplicationAction::Create) {
            sendPacket = true;
        }
    }

    if (sendPacket) {
        ModuleNetworkingServer::ClientProxy* proxy = App->modNetServer->getClientProxy(clientIndex);
        
        OutputMemoryStream packet;
        proxy->replicationManager.Write(packet, &proxy->deliveryManager, indispensableCommands);
        App->modNetServer->sendPacket(packet, proxy->address);
    }
}

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
        uint32 packetReceivedIndex;
        packet >> packetReceivedIndex;
        if (pendingDeliveries.size() > 0)
        {
            auto iter = std::find_if(
                pendingDeliveries.begin(),
                pendingDeliveries.end(),
                [packetReceivedIndex](Delivery* d)
                    {return d->sequenceNumber == packetReceivedIndex; });
            
            if (iter != pendingDeliveries.end())
            {
                
                (*iter)->onDeliverySucess(getClientIndex());
                delete (*iter);
                pendingDeliveries.erase(iter);
            }
        }
    }
}

int DeliveryManagerServer::getClientIndex() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (&App->modNetServer->getClientProxy(i)->deliveryManager == this) {
            return i;
        }
    }
    ASSERT(false);//None of the delivery managers are this one
    return -1;
}

void DeliveryManagerServer::processTimedOutPackets()
{
    for (int i = pendingDeliveries.size() - 1; i >= 0; --i)
    {
        if (pendingDeliveries[i]->dispatchTime < Time.time)
        {
            pendingDeliveries[i]->onDeliveryFailure(getClientIndex());
            delete pendingDeliveries[i];
            pendingDeliveries.erase(pendingDeliveries.begin() + i);
        }
    }
}

void DeliveryManagerServer::clear()
{
    for (int i = 0; i < pendingDeliveries.size(); i++)
    {
        delete pendingDeliveries[i];
    }
    pendingDeliveries.clear();
}

bool DeliveryManagerClient::processSequenceNumber(const InputMemoryStream& packet)
{
    uint32 sequenceNumber = 0;
    packet >> sequenceNumber;

    if (sequenceNumber >= expectedSequenceNum) {
        pendingAck.push_back(sequenceNumber);
        expectedSequenceNum = expectedSequenceNum + 1;
        return true;
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

void DeliveryManagerClient::clear()
{
    expectedSequenceNum = 0;
    pendingAck.clear();
}
