#pragma once
#include <vector>
#include <map>

// TODO(you): Reliability on top of UDP lab session

class DeliveryManagerServer;
class Delivery;
struct ReplicationCommand;

class DeliveryDelegate
{
public:
	DeliveryDelegate(Delivery* parent);
	virtual void onDeliverySuccess(DeliveryManagerServer* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManagerServer* deliveryManager) = 0;
protected:
	Delivery* parent = nullptr;
};

class DeliveryDelegateDestroy : public DeliveryDelegate {
public:
	DeliveryDelegateDestroy(Delivery* parent);
	void onDeliverySuccess(DeliveryManagerServer * deliveryManager) override;
	void onDeliveryFailure(DeliveryManagerServer * deliveryManager) override;
};

struct Delivery
{
	Delivery(uint32 sequenceNumber, double dispatchTime);
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
	
	std::vector<ReplicationCommand> indispensableCommands;

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
	uint32 nextSequenceNum = 0;
	std::vector<Delivery*> pendingDeliveries;

	friend class DeliveryDelegate;
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
	std::vector<uint32> pendingAck;
};

