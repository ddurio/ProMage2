Known Issues:
- No crashes / fatals
- I am experiencing some graphical artifacts when drawing debris objects only in release mode.

How To Use:
- Keyboard Key Bindings:
    * Left / Right Arrow Keys: Turn ship left / right
    * Up Arrow Key: Accelerate
    * Space Bar: Fire bullet

    * A: Spawn (A)steroid
    * N: Spawn (N)ew PlayerShip when dead
    * P: (P)ause / Unpause Game
    * T: Slow down (T)ime while held

    * F1: Toggle Debug Drawing Mode
        - Magenta is cosmetic radius
        - Cyan is physics radius
    * F8: Hard reset entire game
- Xbox Controller Key Bindings:
    * Left Joystick: Turn & Accelerate
    * A: Fire bullet

    * Start: Spawn New PlayerShip when dead
- List of Game Constants:
    A list of configurable game constants are stored here: ./Code/Game/GameCommon.hpp
    These configurable values are references throughout the code to simply basic changes.
    Examples include:
        * Max Number of Bullets
        * Max Number of Asteroids
        * Bullet Speed
        * All Cosmetic and Physics Radii
    For a full list, see GameCommon.hpp

Deep Learning (Gold):
    Because both SD1 and Programming for Commercial Game Engines have had a fairly hefty
    assignment due at the end of this week, I've been forcing myself to context switch
    between the two often over the past two weeks.  The realization I came to, as we
    touched on during today's impromptu Zen Topic, is the need to manage the amount
    of time I spend on each feature within the assignment.  All too often, I lose track
    of how long I've been drilling into a single topic, disregarding possibly more important
    features.  Going forward, I plan to set down a time constraint for each individual
    code chunk to make sure I'm always focusing on the top priorities.  As an added bonus,
    the addition of a time limit, so to speak, should be good practice for coding under
    time pressure at an industry job.

Deep Learning (Playable):
    After reading from multiple books about Rapid Game Development for TGP this week,
    I've come to the realization that I instinctively write more complicated code that takes
    longer to develop.  Usually, this is with regards to saving compute cycles or memory
    despite this being the first pass at the code.  Specifically, during this week's
    assignment, I spent a great deal of time trying to construct a way to not recreate
    an entity's vertices from scratch every frame.  I was optimizing for compute cycles despite
    the code not working yet and with a "SLEEP 16" still in the main loop...
    While I did enjoy the challenge, this was a complete waste of time that I cannot
    afford to repeat in the future.  In general, I need to keep your coding mantra of
    "Get it working. Make it pretty." in the back of my mind to avoid writing needlessly
    complicated code before it's useful. In doing so, I hope to make progress towards
    a truly Rapid development process going forward.
