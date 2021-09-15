# Console-ASCII-RPG
A game demo of an old-school RPG I've written as a personal C++ coding project. [*Fully* text-based](https://en.wikipedia.org/wiki/Text-based_game) - inspired by [Zork](https://en.wikipedia.org/wiki/Zork), [Guilded Youth](https://jimmunroe.net/games/guilded-youth-a-text-game-with-ascii-animation.html), [Sanctuary RPG](https://store.steampowered.com/app/328760/SanctuaryRPG_Black_Edition/). Hardly scalable, as it started as a minor highschool project, and turned into spaghetti code fast.

# How to Run:
1. Download ZIP of the repository
2. Open '/Release/ASCIIEngine.exe' (in cmd or in explorer)

Source code is located in the root and the '/engine' folder.

# Stuff I finished:
- ASCII rendering - featuring colors, animations, and speed optimizations (DSU subarray-painting)
- ASCII-based GUI system
- ASCII-based particle systems
- FMOD-based audio system
- Engine debugging tools (game pause, error messages, debug logs)
- Turn-based combat system
- Minimal custom mark-up language for game scripts, GUI objects, visual objects, and animations - editing game scenarios and assets is done completely outside source code (see '/Release/resources' folder for examples)
> script.txt file 
```
!bop_dog
?scroll
~bop nose$do not bop nose$
What will you do?
/
!bop_dog_yes
The dog is happy and thanks you with an innocent smile.
!bop_dog_no
You regret not bopping the dog's nose, as it wimpers.
%
```
> script_links.txt file
```
&bop_dog
2
bop_dog_yes
bop_dog_no
%
```

# Stuff I wish I had the time to work on:
- Code refactoring
- Lenghtier game demo 
- More complex combat encounters
- Smarter enemy AI
- Encoding all text-based game resources
- Releasing a Linux version

