Extended / External Plugin for TF2
----------------------------------

by CasualX

Intro:
======

As Valve shown clearly they do not like us bypassing their stupid plugin checks (whose only purpose serves to stop modding) an alternative needed to be made. While a straight up plugin would be far more convenient (for both user & developer) it isn't the only option available. The reason this wasn't done earlier this way is because this is much harder and more limited, but I'll see what I can do.

Windows-only, as it heavily relies on Windows APIs it is unlikely you'll see this for any other platform soon.

Features:
=========

* Allow mat_picmip in range [-10,6]
* Unlocks viewmodel_fov_demo
* Run a console command in the context of the game

HowTo:
======

For convenience, double click 'launch.bat' when the game is at its main menu. A more detailed overview below.

Launch ExtPlugin.exe with parameters specifying the features you wish to use.
Warning! Do not run the program before the main menu has appeared, it may lead to the game crashing!

First, you must specify the game process:
/pid:<process-id>      Work on a specific process id.
/process:<exe-name>    Work on all processes with this exe filename.
/window:<window-title> Work on the process which created a window with this name.

With the /verbose switch you enable more extensive output of feedback.

Allow more extensive mat_picmip settings (and some more) with /picmip.

With /cvars it'll run the code to modify certain console variables (not configurable, hard-coded in the source code).

A command is executed in the context of the game with /command:"echo Hello World!"

FAQ:
====

Q. It says I'm missing MSVCR100.DLL, how do I fix?  
A. Install the visual 2010 redistributable (x86) package [here](GO FIND URL)

Q. Will I get VAC banned?  
A. Very unlikely. There is no .dll loaded in the game, the plugin itself will run for less than a second while you're not in a server so VAC shouldn't even be running.

Q. But... Hacks?  
A. If you manage to make cheats out of this thing then consider yourself skilled enough to go do something useful in real life.
