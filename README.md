# Souless Knights

Souless knights is an online multiplayer game about being the one that stays alive and kills the most enemies in the lobby.
You can kill other players by shooting projectiles at them, but be careful, projectiles will bounce off walls and can damage you too!


## How to play

- One of you must Start Server, so that the others can join their lobby.
- Your friends can connect using their public IP address and the port the server was executed at.
- Once you join, there's no waiting, try to kill as many players as you can!
- **Move:** WASD (you must have your controller disconnected)
- **Aim:** Rotate the mouse around the character.
- **Shoot:** Press Left Mouse Button.
- Everytime you're killed, you'll respawn after some seconds, so you can try again.
- See who gets more kills!


## Contribution

### Yessica Servin Dominguez

- [Achieved] Lab 6: World State Replication.
- [Achieved] Lab 7: Delivery Manager (timeout packets), Redundant sending of input packets.
- [Achieved] Lab 8: Entity interpolation, Client-side prediction, Server reconciliation.
- [Achieved] Gameplay: Players can't exit play zone, lasers bounce off walls, background sprite, kill counter.


### Jaume Montagut Guix

- [Achieved] Lab 6: World State Replication.
- [Achieved] Lab 7: Delivery Manager.
- [Achieved] Lab 8: Solve Client-side prediction bugs (object was adding more inputs than necessary).
- [Achieved] Solve Packet loss bugs: Update non created object, Create where there is one gameobject.
- [Achieved] Gameplay: Character's aiming, moving, knight animations, load DragonBones animation from JSON.

## Releases

- You can find our releases at:
https://github.com/YessicaSD/Xarxas/releases
- Find the code inside ./Project 5/Multiplayer Game/:
https://github.com/YessicaSD/Xarxas/tree/main/Project5/Multiplayer%20Game


## Considerations

- Lab 7: Delivery Manager
We decided to split the DeliveryManager class into two classes since they used their own function and didn't share much functionality.