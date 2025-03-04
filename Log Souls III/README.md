# Log Souls III
_Log Souls III_ is a Dark Souls 3 is an application designed to track and analyze player progress in the game, providing insights into key aspects of gameplay. With a focus on decision-making and in-game actions, the tool collects and organizes data on deaths, boss encounters, bonfire rests, and soul retrievals. It captures critical game events, such as when a boss is defeated, and updates previous log entries to reflect the actual boss names, even if initially marked as "NIL."
## How to use
You can start the program, either:
* Before launching DS3
* While you are in-game

While running, it will log your game's data. When you are finished with your session for the day, as you close the game, LS3 will close automatically too.

#### WARNING!

If you started the logging process -> started a game -> and then decided to change characters:
* restart the logger, because it can not handle profile changing!



## Explanation for logging
The application stores this data in structured .txt files, using a multi-level hashtag format to categorize different types of information:. The 5 different data types:
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
This module was written in Visual Studio using C / C++ elements. You don't need any Nuget packages to run it for yourself and make a build of your own. The module is made up of 3 main files:
* **main.cpp**: Where everything happens
* **MemoryManipulation.h**: Connecting to the memory of the computer and finding the game's specific attributes _(PID, base address)_
* **DataGathering.h**: Getting the ingame data from memory, and stashing it away _(specific pointers)_

You can look at the [Logger](https://github.com/BenTheGoose69/DarkSoulsIII_Logger_and_Sorter/tree/6c178b9e85b7902345db5d52924f82595846c8a6/Log%20Souls%20III/Logger) folder for a deeper understanding or run the .sln file to open in VS.
