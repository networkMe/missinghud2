## Overview
Missing HUD 2 is an Open-GL powered informational overlay for the Binding of Isaac: Rebirth.

The developers of Rebirth (Edmund McMillen, Nicalis) decided that one of their design decisions for the game would be to hide raw player statistics from the player as to not to overwhelm them.
This project gives the player the choice to see their raw statistics if they choose to.

It is a transparent mod that **DOES NOT** disable achievements nor alter your Isaac game files in any way. As a result, it can be enabled and disabled at any point, during any run, with no lasting consequences.

Unlike other mods, it uses your live character statistics during a run. As a result, it remains 100% accurate even after picking up items like [Experimental Treatment](http://bindingofisaacrebirth.gamepedia.com/Experimental_Treatment) and [Libra](http://bindingofisaacrebirth.gamepedia.com/Libra).

![Image of MissingHUD2](https://raw.githubusercontent.com/networkMe/missinghud2/master/doc/isaac-mhud2-example-121.jpg)

## Using
Missing HUD 2 aims to be nearly transparent to the user (and to Rebirth itself).

You simply run the main executable (which acts as the DLL injector) and the HUD will be drawn onto an active Rebirth process.
Note: The HUD only appears if you are in an active run.

If you wish to no longer see the HUD, just close the main executable and the HUD will disappear (the DLL will be unloaded).

The latest binary release can be found here:
https://github.com/networkMe/missinghud2/releases/latest

## Runtime Requirements
* Run from a folder with no Unicode characters in the path (aka. characters not on a standard keyboard) ? ?_? ??
* A graphics card with OpenGL 3.3 support (any graphics card released since 2010 will have this).

## Current features
* Works in fullscreen and windowed mode (as it's a direct OpenGL implementation)
* Shows how your raw statistics change as you pick up items and use pills, real-time
* Statistics HUD on the left of the Rebirth viewport shows:
  * Speed
  * Range
  * Tear firerate (Tear delay, lower is faster)
  * Shot speed
  * Damage
  * Luck
  * Deal with the Devil % chance

## Building
Missing HUD 2 has the below dependencies:

1. Main executable (DLL injector)
  * [Qt5](http://www.qt.io/) (static version directory set manually in CMake)
  * [EasyLogging++](https://github.com/easylogging/easyloggingpp)
2. Injected DLL
  * [GLEW](https://github.com/nigels-com/glew)
  * [SOIL2](https://bitbucket.org/SpartanJ/soil2)
  
It uses the CMake build system to compile.