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
    for (auto i = pendingDeliveries.begin(); i != pendingDeliveries.end();)
    {
        if ((*i)->dispatchTime < Time.time)
        {
            // TODO execute callback onDeliveryFailed()
            if((*i)->delegate != nullptr)
                (*i)->delegate->onDeliveryFailure(this);

            i = pendingDeliveries.erase(i);
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
    }
    else if (sequenceNumber < expectedSequenceNum) {
        //send success
    }

    return false;
}
