# C64maze
A 3D maze game written in C for the Commodore 64.

This project (distributed with the GPLv.3 license) is a very simple 3D maze for
the Commodore 64 computer. The source code is written in C (with some inline
assembly) and it is meant to be compiled with the cc65 compiler.

The goal of the game is to find the exit of the maze in the shortest possible amount of time. The entrance of the maze is changed randomly each time the game is played:

![Do you dare to enter The Maze?](https://github.com/DarwinNE/C64maze/raw/master/screenshots/step_in.png)

You can have a look at the maze map, but beware! Each time this is done, a penalty of 30s is applied:

![Maze map](https://github.com/DarwinNE/C64maze/raw/master/screenshots/mazeview.png)

You should explore the maze to find the exit:

![Hey! You found the exit!](https://github.com/DarwinNE/C64maze/raw/master/screenshots/exit.png)

And once you find your way through it, you will know how much time you needed:

![Game completed.](https://github.com/DarwinNE/C64maze/raw/master/screenshots/exit_s.png)

The music is a 3-part reduction for the SID of J.S. Bach's "little" fugue in G minor, BWV578. Hommage to Wendy Carlos.

Author: D. Bucci
