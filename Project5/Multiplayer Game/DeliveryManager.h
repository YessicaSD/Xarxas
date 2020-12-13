#pragma once

// TODO(you): Reliability on top of UDP lab session
class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryMa)
};
struct Delivery
{
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

};