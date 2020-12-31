#pragma once
#include <vector>
#include <map>

// TODO(you): Reliability on top of UDP lab session

struct ReplicationCommand;

struct Delivery
{
	Delivery(uint32 sequenceNumber, double dispatchTime);
	~Delivery();
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	void onDeliverySucess(int clientIndex);
	void onDeliveryFailure(int clientIndex);

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
	int getClientIndex();

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
	uint32 expectedSequenceNum = 0;
	//- List of sequence number pending acknoweledgment
	std::vector<uint32> pendingAck;
};

