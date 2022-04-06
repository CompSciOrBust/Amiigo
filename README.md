![Logo](icon.jpg)

# Amiigo

A GUI for Emuiibo. The [Emuiibo](https://github.com/XorTroll/emuiibo) sysmodule is required in order to use Amiigo.
Amiigo's UI is built with the [Arriba](https://github.com/CompSciOrBust/Arriba) framework.

<a  href="https://github.com/CompSciOrBust/Amiigo/releases"><img  src="https://img.shields.io/github/downloads/CompSciOrBust/Amiigo/total?style=for-the-badge"  alr = "Downloads "  /></a> <a  href="https://discord.gg/ZhRn3nn"><img  src="https://img.shields.io/discord/673622282790502402?color=green&label=Discord&logo=discord&style=for-the-badge"  alt="Discord Server"  /></a> <a  href="https://twitter.com/CompSciOrBust?ref_src=twsrc%5Etfw"><img  src="https://img.shields.io/twitter/follow/CompSciOrBust?color=blue&label=follow&logo=twitter&style=for-the-badge"  alt="Follow on twitter"  /></a>
<!--ts-->
   * [Credits](#Credits)
   * [Controls](#Controls)
   * [Support](#Support)
   * [Building](#Building)
   * [Donating](#Donating)
<!--te-->
  

## Credits

LibNX and DevkitPro for making the toolchain.

[XorTroll](https://github.com/XorTroll/) for [Emuiibo](https://github.com/XorTroll/emuiibo) and general help.

[N3evin](https://github.com/N3evin/) for [AmiiboAPI](https://github.com/N3evin/AmiiboAPI) which is used in Amiigo Store.

nlohmann for his [JSON library](https://github.com/nlohmann/json) .

[Kronos2308](https://github.com/Kronos2308/) for maintaining Amiigo Mod.

Victoria_Borodinova for the [sombrero](https://pixabay.com/illustrations/sombrero-hat-mexico-mexican-4280389/) used in the logo.

Team-Xecuter for providing me with free hardware after my Switch broke. (Xecuter ❤️ open source)

Za for bringing pizzas.

All of the beta testers in the [CompSciOrBust Discord server](https://discord.gg/ZhRn3nn).

Everyone in the Team AtlasNX dev chat for general help in this and other projects.  

## Controls

D-pad to move between on screen buttons.

A to select an Amiibo from the list or click an on screen button.

X toggles emulation state.

B backs out of a category if one is selected.

Touchscreen can also be used to click onscreen buttons and scroll lists.

## Support

For support please use [the offical GBATemp thread](https://gbatemp.net/threads/amiigo-emuiibo-gui.549964/) unless you are reporting a bug in which case open a issue here on GitHub. Alternatively join [my Discord server](https://discord.gg/ZhRn3nn) or @ me on [twitter](https://twitter.com/CompSciOrBust).

## Screenshots

<img  src="https://raw.githubusercontent.com/CompSciOrBust/Amiigo/master/Screenshots/Screenshot_1.jpg"  width="432"/><img  src="https://raw.githubusercontent.com/CompSciOrBust/Amiigo/master/Screenshots/Screenshot_2.jpg"  width="432"/><img  src="https://raw.githubusercontent.com/CompSciOrBust/Amiigo/master/Screenshots/Screenshot_3.jpg" width="432"/>

## Building

Install [LibNX](https://switchbrew.org/wiki/Setting_up_Development_Environment).

Recursively clone this repo.

Run (dkp-)pacman -S switch-glm

Run (dkp-)pacman -S switch-glm

Run (dkp-)pacman -S switch-glfw

Run (dkp-)pacman -S switch-glad

Run (dkp-)pacman -S switch-curl

Run "Make".

Optionally run "nxlink -s Amiigo.nro" after opening NetLoader to get debugging info.

## Donating
Before donating to someone who makes bad homebrew consider donating to a charity instead.
|Method|Info|How it will be used|
|--|--|--|
|BTC|1GUYKgask9u81MspethuF826iT8VCSg6XP|This is the "buy me a beer" option. I'll just spend it on whatever I want.|
|Deliveroo credit|Contact me on Twitter or Discord|I'll order pizza instead of cooking and use the time saved to write Homebrew.|
