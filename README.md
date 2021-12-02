xinputproxy
===========
xinputproxy is a proxy dll for XInput 1.4 or 9

Although it could be used for all manner of filtering of controller messages (e.g. disable Vibration if the game does not make it configurable, looking at you LOTR : War in the North), I have personally used it in the game Darkwood to be able to disable running.

In Darkwood, the PC is wieved top-down and their movement is controlled with the right stick. When the stick is pushed to the edge, the PC starts running; this was rather annoying in a fight, when instead of winding-up for an attack and waiting for the best moment to strike, the character would just start running away if, in the rush of the moment, I had pushed the stick too far.

A proxydll for the XInput driver could be used to artificially restrict the stick movement: even when the stick is close to the edge, always a lower value would be reported to the application/game; this prevents the PC from running. The posibility of running can be restored by pressing the stick (configurable) which restores the full range of movement; another press will re-enable the restriction.

Other notes
-----------
The file should be placed in the same directory as the game executable, and renamed to one of the following:
- XInput1_1.dll
- XInput1_2.dll
- XInput1_3.dll
- XInput1_4.dll
- XInput9_1_0.dll

The name depends on which dll the game uses. Either create all 5 files and don't bother, or use ProcessExplorer and check to see which one is loaded by the game. When a dll is loaded it will create a companion .log file; just check to see which log file lists controller status being accesed; you could then remove the other dlls.

Disclamer of liability
---------
Some Anti-Cheat solutions might flag this as cheating, so consider yourself warned. I personally try to avoid games with online coponent.

That being said, if you decide to use this tool, you are doing so at your own risk. I, and any persons involved in the development of this tool, cannot be held accountable for any damages caused by the use of this tool. I do not offer any advice on how anyone should use it or where to use it.

Copyright
---------
xinputproxy is Copyright 2021 Mircea-Dacian Munteanu under the New BSD License (BSD 3-clause), see included LICENSE file.

xinputproxy contains the following third-party libraries:

- [inih](https://github.com/benhoyt/inih), which is copyright © 2009 – 2020 Ben Hoyt and used under a BSD 3-clause license.

