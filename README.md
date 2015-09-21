## Overview
Missing HUD 2 is an Open-GL powered informational overlay for the Binding of Isaac: Rebirth.

The developers of Rebirth (Edmund McMillen, Nicalis) decided that one of their design decisions for the game would be to hide raw player statistics from the player as to not to overwhelm them.
This project attempts to give the player the choice to see their raw statistics if they choose to.

## Using
Missing HUD 2 aims to be nearly transparent to the user (and to Rebirth itself).

You simply run the main executable (which acts as the DLL injector) and the HUD will be drawn onto an active Rebirth process.
Note: The HUD only appears if you are in an active run.

If you wish to no longer see the HUD, just close the main executable and the HUD will disappear (the DLL will be unloaded).

## Current features
* Works in fullscreen and windowed mode (as it's a direct OpenGL implementation)
* Show statistics HUD on the left of the Rebirth viewport
  * Speed
  * Range
  * Tear firerate
  * Shot speed
  * Damage
  * Luck
  * TODO: Deal with the Devil % chance

## Building
Missing HUD 2 has the below dependencies:

1. Main executable (DLL injector)
  * [Qt5](http://www.qt.io/) (static version directory set manually in CMake)
  * [EasyLogging++](https://github.com/easylogging/easyloggingpp)
2. Injected DLL
  * [MinHook](https://github.com/TsudaKageyu/minhook)
  * [GLEW](https://github.com/nigels-com/glew)
  * [SOIL2](https://bitbucket.org/SpartanJ/soil2)
  
It uses the CMake build system to compile.