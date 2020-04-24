# THIS PROJECT IS ARCHIVED.

I am no longer updating any of my switch homebrew apps. This also means that there will be no more public releases made by me.

Thank you to everyone that was kind to me along the way.

----

## Gamecard-Installer-NX

A way to install your gamecards directly to the switch!

![Alt text](images/example.gif)

----

## How to use

__Installing__

Download the latest release and unzip the contents to the root of the sd card.

This should result in a folder struct like this `/switch/gamecard_installer/gamecard_installer.nro`.


__Multi gamecards__

Some gamecards have multiple games inside. An example of this is the Child of Light & Valiant Hearts gamecard.

These can still be installed. Pressing `L` or `R` will allow you to cycle through the games on the gamecard.

__Lower Keygen Version__

Lowering the keygen (to zero) can allow for games to be launched on all firmware version. This does not guarantee that the game will work however.

This option will also convert any titlekey encrypted nca's to standard crypto (ticketless).

An example of a working game is Pokemon Sword, this game requires firmware 7 usually, however it can work on firmware 6.

To use this option, you must have a keys file named either `prod.keys` or `keys.txt` in either `/switch` or `/switch/gamecard_installer/`.

To dump your own keys please use [Lockpick RCM](https://github.com/shchmue/Lockpick_RCM).

__Playing music__

This app can play music!

Simply drop any .mp3 files into `/switch/gamecard_installer/`.

You can have the .mp3 in folders if you prefer, the app will scan recursively. E.g. `/switch/gamecard_installer/music/song.mp3`.

[Here's a great site for high quality video game music.](https://downloads.khinsider.com/).

----

## How to build

Install the Devkitpro tool chain from [here](https://devkitpro.org/wiki/Getting_Started).

Using pacman (installed with devkitpro) install the following libraries:

* switch-freetype
* switch-sdl2
* switch-sdl2_mixer
* switch-sdl2_gfx
* switch-sdl2_image
* switch-sdl2_ttf
* switch-mpg123
* switch-libwebp
* switch-libpng
* switch-opusfile
* switch-libvorbisidec
* switch-libmodplug
* switch-libjpeg-turbo
* switch-flac
* switch-libopus
* switch-libogg
* switch-zlib

If you have any problems building, feel free to open an issue including any build errors.

----

## Special thanks

[toph](https://github.com/sudot0ph) for the design of the app icon!

The great people over at AtlasNX discord for gui advice and helping test.

----

## Support my work

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/P5P81CQOY)

<a href="https://www.patreon.com/totaljustice"><img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" alt="Patreon donate button" /> </a>
