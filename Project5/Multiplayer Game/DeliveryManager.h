#pragma once
#include <vector>

// TODO(you): Reliability on top of UDP lab session

class DeliveryManager;

class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	Delivery(uint32 sequenceNumber, double dispatchTime);
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager
{
public:
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);
	bool processSequenceNumber(const InputMemoryStream& packet);
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();
	void clear();

private:
	//Sender variables
	uint32 nextSequenceNum;
	std::vector<Delivery*> pendingDeliveries;

	//Receiver variables
	uint32 expectedSequenceNum;
	//- List of sequence number pending acknoweledgment
	std::vector<int> pendingAck;
};

