#pragma once

// TODO(you): World state replication lab session

union ReplicationCommand {
	//ReplicationActionType actionType;

	struct destroyCommand {
		int networkId;
		//nothings
	};

	struct creationCommand{
		int networkId;
		//position
		//rotation
	};
};