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
    for (auto i = pendingDeliveries.begin(); i != pendingDeliveries.end(); i++)
    {
        if ((*i)->dispatchTime < Time.time)
        {

        }
    }
}
