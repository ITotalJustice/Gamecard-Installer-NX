# Gamecard-Installer-NX

A way to install your gamecards directly to the switch!

----

## How to use

__Installing__

Download the latest release and unzip the contents to the root of the sd card.

This should result in a folder struct like this `/switch/gamecard_installer/gamecard_installer.nro`.


__Playing music__

This app can play music!

Simply drop any .mp3 files into `/switch/gamecard_installer/`.

You can have the .mp3 in folders if you prefer, the app will scan recursively. E.g. `/switch/gamecard_installer/music/song.mp3`.

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
