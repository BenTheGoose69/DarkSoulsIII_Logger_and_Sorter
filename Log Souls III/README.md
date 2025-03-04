# Log Souls III
This module was written in Visual Studio using C / C++ elements. You don't need any Nuget packages to run it for yourself and make a build of your own
## How to use
You can start the program, either:
* Before launching DS3
* While you are in-game

While running, it will log your game's data. When you are finished with your session for the day, as you close the game, LS3 will close automatically too.

**WARNING!**

If you started the logging process, started a game, and then decided to change characters, then restart the logger, because it can not handle profile changing!



## Explanation for logging
The number of #-s represent a different kind of logged data. The module logs 5 different data types:
1. When a player dies
```
#
Death number
Timestamp
```
2. When they sit down at a bonfire
```
##
Resting at which bonfire
Current level
Current souls
Total souls
Timestamp
```
3. When they recover souls
```
###
Recovered souls amount
Timestamp
```
4. When they enter/exit/defeat a bossfight
```
####
ENTERED / DIED / DEFEATED (bossfights)
Boss name / NIL
Timestamp
```
5. When they started a game session, using ingame clock
```
#####
Sat down to play at what time (Timestamp)
```
## Structure
The module is made up of 3 main files:
* **main.cpp**: Where everything happens
* **MemoryManipulation.h**: Connecting to the memory of the computer and finding the game's specific attributes _(PID, base address)_
* **DataGathering.h**: Getting the ingame data from memory, and stashing it away _(specific pointers)_

You can look at the [Logger](https://github.com/BenTheGoose69/DarkSoulsIII_Logger_and_Sorter/tree/6c178b9e85b7902345db5d52924f82595846c8a6/Log%20Souls%20III/Logger) folder for a deeper understanding or run the .sln file to open in VS.
