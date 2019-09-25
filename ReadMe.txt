Known Issues:
- No crashes / fatals
- DistanceField steps do not highlight modified tiles
- RoomsAndPath steps do not highlight modified "path" tiles
- EdgedTiles step does not highlight modified tiles

General:
- ProMage2 == 'PRO'cedural 'M'ap 'A'lgorithm 'GE'nerator '2'D
- Tilde (`) to open devConsole
- Execute 'quit' devConsole command, Alt-F4, or click Windows X to quit

Editor:
- Launch  ./Run/ProMage2_x64_Editor.exe
- Can be run from VS in either EditorDebug or EditorRelease build configuration
- Controls:
	* Mouse left click: buttons along botton to change MapGenStep
	* Mouse hover: red highlighted tiles will display tooltip of tile modifications in bottom right

Game:
- Launch  ./Run/ProMage2_x64.exe
- Can be run from VS in either GameDebug or GameRelease build configuration
- Controls:
	* WASD to move character
	* F to interact with stairs (within one tile radius)

    * F1: Toggle Debug Drawing Mode
		- Displays physics collider outlines
	* F2: CHEAT -- move to next floor
	* F3: CHEAT -- move to previous floor
	* F6: CHEAT -- use full map camera
    * F8: Hard reset entire game
- DevConsole Command:
	* 'goToFloor number=##'
		** Case insensitive, but no spaces around equal sign allowed
		** Map type alternates ever ten levels (1-10, 11-20, ...)
		** 'goToFloor number=11' to see first example of Cave map
	
