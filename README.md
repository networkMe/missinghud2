## Overview
Missing HUD 2 is an OpenGL powered informational overlay for the Binding of Isaac: Rebirth + Afterbirth.

The developers of the Binding of Isaac (Edmund McMillen, Nicalis) decided that one of their design decisions for the game would be to hide raw player statistics from the player as to not to overwhelm them. This project gives the player the choice to see their raw statistics if they choose to.

It is a transparent mod that **DOES NOT** disable achievements nor alter your Isaac game files in any way. **Note:** It can be used in Afterbirth daily runs with no repercussions.

It can be enabled and disabled at any point, during any run, with no lasting consequences. One can run other mods side-by-side with Missing HUD 2 with no issues.

Unlike other statistic based mods, it uses your live character statistics during a run. This translates to Missing HUD 2 remaining 100% accurate even after picking up items like [Experimental Treatment](http://bindingofisaacrebirth.gamepedia.com/Experimental_Treatment) and [Libra](http://bindingofisaacrebirth.gamepedia.com/Libra).

![Image of MissingHUD2](https://raw.githubusercontent.com/networkMe/missinghud2/master/doc/isaac-mhud2-example-141.jpg)

## Using
Missing HUD 2 aims to be nearly transparent to the user (and to Isaac itself).

You simply run the main executable (which acts as the DLL injector) and the HUD will be drawn onto an active Isaac process.
Note: The HUD only appears if you are in an active run. You must leave Missing HUD 2 open while you play the game.

If you wish to no longer see the HUD, just close the main executable and the HUD will disappear (the DLL will be unloaded).

The latest binary release can be found here:
https://github.com/networkMe/missinghud2/releases/latest

## Current features
* Works in fullscreen and windowed mode (as it's a direct OpenGL implementation)
* Shows how your raw statistics change as you pick up items and use pills, real-time
* Allows you to choose at what precision you see the raw statistics (default is 2 decimal place)
* Statistics HUD on the left of the Isaac viewport shows:
  * Total tears fired in run (optional)
  * Speed
  * Range
  * Tear firerate (Tear delay, lower is faster)
  * Shot speed
  * Shot height (optional)
  * Damage
  * Luck
  * Deal with the Devil % chance
  * Deal with the Angel % chance

## Building
Missing HUD 2 has the below dependencies:

1. Main executable (DLL injector)
  * [Qt5](http://www.qt.io/) (static version directory set manually in CMake)
  * [EasyLogging++](https://github.com/easylogging/easyloggingpp)
  * [Boost.Interprocess](http://www.boost.org/doc/libs/1_59_0/doc/html/interprocess.html)
  * [Google's Protobuf](https://github.com/google/protobuf)
2. Injected DLL
  * [GLEW](https://github.com/nigels-com/glew)
  * [SOIL2](https://bitbucket.org/SpartanJ/soil2)
  * [Boost.Interprocess](http://www.boost.org/doc/libs/1_59_0/doc/html/interprocess.html)
  * [Google's Protobuf](https://github.com/google/protobuf)
  
It uses the CMake build system to compile.
The easiest Windows MinGW environment to compile it on is the MSYS2 enviroment.