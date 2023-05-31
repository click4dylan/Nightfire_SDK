# Nightfire_SDK
Half-Life SDK and AMXModX, etc port to James Bond 007 Nightfire PC

NOTE: this is currently a work in progress and part of an overhaul

Only the game's executable (located in dedicated_client), metamod and amxmodx are currently ported

amxmodx is currently not compilable as it is being transitioned from using an older fork of metamod for nightfire

REQUIREMENTS: create environment variable in Windows called HLSDK and point it to the hlsdk directory, create environment variable in Windows called METAMOD and point it to the metamod\metamod-hl1-master directory

Minimum requirements to compile: Visual Studio 2010 (C++) with SP1 and KB2736182 update, Visual Studio 2015 (C++) with Update 3 and XP support selected in installation options, Windows SDK 8.1
Visual Studio 2019 and up is not supported because of lack of support for Windows XP in >14.26
