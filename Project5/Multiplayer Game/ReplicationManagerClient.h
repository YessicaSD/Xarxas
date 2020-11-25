#pragma once

// TODO(you): World state replication lab session
class ReplicationManagerClient
{
public:
	void read(const InputMemoryStream &packet);
	void instantiatePlayerGameObject(uint8 spaceshipType, uint32 networkId, vec2 initialPosition, float initialAngle);
};