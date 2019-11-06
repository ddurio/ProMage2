Known Issues:
- Intermittent Run-time fatal "Read Access Violation" (Release ONLY)
- Clicking on any "Custom Step" in the editor causes "recoverable" errors.. every frame..
	* Don't click on a step header named "Add Stairs" until next week when that's implemented

General:
- ProMage2 == 'PRO'cedural 'M'ap 'A'lgorithm 'GE'nerator '2'D
- Tilde (`) to open devConsole
- Execute 'quit' devConsole command, Alt-F4, or click Windows X to quit

Editor:
- Launch  ./Run/ProMage2_x64_Editor.exe
- Can be run from VS in either EditorDebug or EditorRelease build configuration
- Layout:
  * Map Window - (Top Left)
	** Mouse wheel scroll: Zoom in/out
	** Mouse right hold & Drag: Pan map camera
	** Mouse hover: red highlighted tiles will display tooltip of tile modifications in bottom right
  * XML Editor Window - (Right)
	** Mouse left click: Step header to change step index (only one step open at a time)
	** Mouse left click: GUI interactions (drop downs, buttons, etc.)
	** Mouse right click (on parameter label): Convert parameter between hard-coded value and motif variable
	** Control + Mouse left click: Converts any GUI element into free type field for easy entry
  * Step Media Window - (Bottom Left)
	** Buttons (left to right)
	** Jump to Start - Go to first step
	** Play - Automatically step through generation steps (1 step / sec)
	** Loop - Loop to start during Play
	** Jump to End - Go to last step
	** Step Index slider - Mouse left hold and drag to change step index

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
		** Map type alternates every ten levels (1-10, 11-20, ...)
		** 'goToFloor number=11' to see first example of Cave map
	
