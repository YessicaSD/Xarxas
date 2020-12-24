#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery::Delivery(uint32 sequenceNumber, double dispatchTime) : sequenceNumber(sequenceNumber), dispatchTime(dispatchTime)
{}

Delivery* DeliveryManagerServer::writeSequenceNumber(OutputMemoryStream& packet)
{
    Delivery* newDelivery = new Delivery(nextSequenceNum, Time.time);
    packet << nextSequenceNum;
    nextSequenceNum++;
    pendingDeliveries.push_back(newDelivery);
    return newDelivery;
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
    }
    else if (sequenceNumber < expectedSequenceNum) {
        //only process if it's a destroy
        
        //send success
    }

    return false;
}

void DeliveryDelegateCreate::onDeliverySuccess(DeliveryManagerServer* deliveryManager)
{
    deliveryManager->pendingDeliveries.erase();
}
