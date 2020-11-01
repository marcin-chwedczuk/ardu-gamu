
# Simple Arduino game

![Gameplay](docs/photo2.jpg)
![Setup](docs/photo.jpg)

Parts:
* Arduino Uno
* SNES Controller (bought on Aliexpress)
* 128x64 graphic LCD (bought on [TME](https://www.tme.eu/en/details/rg12864b1/lcd-graphic-displays/raystar-optronics/rg12864b-ghw-v/)) - _RG12864B-GHW-V RAYSTAR OPTRONICS_

In `docs` folder you will find:
* LCD Datasheet, LCD Controller Datasheet
* SNES Controller interface description (thanks to Propeller/Hydra project)

To compile you need:
* `openGLCD` library - https://bitbucket.org/bperrybap/openglcd/wiki/Home

To install `openGLCD` [download](https://bitbucket.org/bperrybap/openglcd/downloads/) `.zip` file with the most recent release.
In Arduino Studio choose Sketch -> Include Library -> Add ZIP Library... 
and restart Arduino Studio. The library comes with great documentation that can be found in Arduino Projects Directory / libraries / openGLCD / docs
(open HTML file using your browser).