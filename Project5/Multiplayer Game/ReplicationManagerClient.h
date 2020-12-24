#pragma once

// TODO(you): World state replication lab session
class ReplicationManagerClient
{
public:
	void Read(const InputMemoryStream &packet, DeliveryManagerClient * deliveryManager);
	void instantiateGameObject(uint32 networkId, const InputMemoryStream &packet);
};