#pragma once
#include <vector>

// TODO(you): Reliability on top of UDP lab session

class DeliveryManagerServer;

class DeliveryDelegate
{
public:
	void onDeliverySuccess(DeliveryManagerServer* deliveryManager);
	void onDeliveryFailure(DeliveryManagerServer* deliveryManager);
};

struct Delivery
{
	Delivery(uint32 sequenceNumber, double dispatchTime);
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManagerServer
{
public:
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();
	void clear();

private:
	//Sender variables
	uint32 nextSequenceNum;
	std::vector<Delivery*> pendingDeliveries;

	friend class DeliveryDelegateCreate;
};

class DeliveryManagerClient {
public:
	bool processSequenceNumber(const InputMemoryStream& packet);
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);
	void clear();

private:
	//Receiver variables
	uint32 expectedSequenceNum;
	//- List of sequence number pending acknoweledgment
	std::vector<int> pendingAck;
};

