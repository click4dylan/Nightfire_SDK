# Nightfire_SDK
Half-Life SDK updated to match Nightfire PC's engine and game/server and renderer functions and structs, as well as AMXModX, MetaMod, and MetaAudio ported to James Bond 007 Nightfire PC

NOTE: this is currently a work in progress and part of a complete overhaul.

Only the game's client/server launcher executable (located in dedicated_client), metamod and amxmodx are currently ported. Major game and client classes and structures are still actively being reversed and ported to this SDK. You may find changelogs to the updated structures in the commits made.

The game's launcher is where primary bugfixes are being added to the game, injected at runtime to aid in supporting as many versions of the game as possible (including any early builds that might show up in the future).

As of April, 2024, AMXModX is now fully compatible with this latest SDK.

REQUIREMENTS: create environment variable in Windows called HLSDK and point it to the hlsdk directory, create environment variable in Windows called METAMOD and point it to the metamod\metamod-hl1-master directory.

This SDK is only compatible with a stock 1.0 or 1.1 installation of the game, and is NOT compatible with the modified public build and community patch of the game available on the internet due to the fact that the community patch was built upon many years of hard-coded assembly and bad implementations that make compatibility impossible.

Minimum requirements to compile: Visual Studio 2010 (C++) with SP1 and KB2736182 update, Visual Studio 2015 (C++) with Update 3 and XP support selected in installation options, Windows SDK 8.1.
