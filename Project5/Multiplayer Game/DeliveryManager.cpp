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
    if (sequenceNumber == expectedSequenceNum) {
        //send success
        expectedSequenceNum = expectedSequenceNum + 1;
    }
    else if (sequenceNumber < expectedSequenceNum) {
        //choose to dicard the packet or whatever
        //remove from pending ack
        //send success
    }
    else if (sequenceNumber > expectedSequenceNum){
        for (int i = expectedSequenceNum; i < sequenceNumber; ++i) {
            if (std::find(pendingAck.front(), pendingAck.back(), i)) {
                pendingAck.push_back(i);
            }
        }
        //TODO : send packet to server to resend the pending acknowledgements
        //send success
        expectedSequenceNum = expectedSequenceNum + 1;
    }
    return false;
}
