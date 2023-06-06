![Logo](icon.jpg)

# Amiigo

Amiigo is a GUI for creating and switching virtual Amiibo for use with the [Emuiibo](https://github.com/XorTroll/emuiibo) system module. Amiigo runs entirely on the Nintendo Switch console and no PC is required for setup. A Nintendo Switch with custom firmware is required to use Amiigo. Simply run the NRO file and Amiigo will handle the rest.

All Amiibo data is obtained from the [AmiiboAPI](https://www.amiiboapi.com/) and the [emutool](https://github.com/XorTroll/emuiibo/tree/master/emutool) source code was referenced extensively to understand how virtual Amiibo generation works. 

Amiigo is built with the [Arriba](https://github.com/CompSciOrBust/Arriba) UI framework, which was designed with Amiigo in mind.

<a  href="https://github.com/CompSciOrBust/Amiigo/releases"><img  src="https://img.shields.io/github/downloads/CompSciOrBust/Amiigo/total?style=for-the-badge"  alr = "Downloads "  /></a> <a  href="https://discord.gg/ZhRn3nn"><img  src="https://img.shields.io/discord/673622282790502402?color=green&label=Discord&logo=discord&style=for-the-badge"  alt="Discord Server"  /></a>
<!--ts-->
   * [Credits](#Credits)
   * [Usage](#Usage)
   * [Support](#Support)
   * [Building](#Building)
   * [Donating](#Donating)
<!--te-->
  

## Credits

[XorTroll](https://github.com/XorTroll/) for [Emuiibo](https://github.com/XorTroll/emuiibo) and general help.

[N3evin](https://github.com/N3evin/) for [AmiiboAPI](https://github.com/N3evin/AmiiboAPI) which is used in Amiigo Store.

nlohmann for his [JSON library](https://github.com/nlohmann/json).

[Kronos2308](https://github.com/Kronos2308/) for maintaining Amiigo Mod.

Victoria_Borodinova for the [sombrero](https://pixabay.com/illustrations/sombrero-hat-mexico-mexican-4280389/) used in the logo.

Za for bringing pizzas.

All of the beta testers in the [CompSciOrBust Discord server](https://discord.gg/ZhRn3nn).

[Kim-Dewelski](https://github.com/Kim-Dewelski) for being a long time friend who has provided much useful programming advice.

## Usage

Use the "Amiigo Store" to generate new virtual Amiibos.

Use the "My Amiibo" list to emulate any of your existing virtual Amiibos.

D-Pad / analog sticks move between on screen buttons.

A selects an Amiibo from the list or click an on screen button.

X toggles emulation state.

B backs out of a category if one is selected.

Y / Long touchscreen tap opens the context menu for supported items.

Everything that can be done with physical buttons can also be done via touchscreen.

## Support

For support please use [the offical GBATemp thread](https://gbatemp.net/threads/amiigo-emuiibo-gui.549964/) unless you are reporting a bug in which case open a issue here on GitHub. Alternatively join [my Discord server](https://discord.gg/ZhRn3nn).

## Screenshots

<img  src="https://raw.githubusercontent.com/CompSciOrBust/Amiigo/master/Screenshots/Screenshot_1.jpg"  width="432"/><img  src="https://raw.githubusercontent.com/CompSciOrBust/Amiigo/master/Screenshots/Screenshot_2.jpg"  width="432"/><img  src="https://raw.githubusercontent.com/CompSciOrBust/Amiigo/master/Screenshots/Screenshot_3.jpg" width="432"/>

## Building

Install [LibNX](https://switchbrew.org/wiki/Setting_up_Development_Environment).

Recursively clone this repo.

Run (dkp-)pacman -S switch-glfw

Run (dkp-)pacman -S switch-glad

Run (dkp-)pacman -S switch-curl

Run Make

Optionally run "nxlink -s Amiigo.nro" after opening NetLoader to get debugging info.

## Donating
Before donating to someone who makes bad homebrew consider donating to a charity instead.
|Method|Info|How it will be used|
|--|--|--|
|BTC|1GUYKgask9u81MspethuF826iT8VCSg6XP|This is the "buy me a beer" option. I'll just spend it on whatever I want.|
|Deliveroo credit|Contact me on Twitter or Discord|I'll order pizza instead of cooking and use the time saved to write Homebrew.|
