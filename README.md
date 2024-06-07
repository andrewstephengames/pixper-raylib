# Introduction to Pixper

**Pixper** is a 2D graphical game in which the objective is to collect all apples generated on a board resembling flatlands, while avoiding dangerous obstacles and entities.

The game is controlled entirely via WASD, although you can also use keys such as Q to quit, P for pausing or Escape for going back. The project was made in C using `raylib` and `sqlite3`.

# Screenshots


<img src="./docs/images/menu1.png" alt="Initial menu" width="640" height="360"/>

The game's initial menu

<img src="./docs/images/gameplay.png" alt="Gameplay" width="640" height="360"/>

The gameplay

<img src="./docs/images/win.png" alt="You won!" width="640" height="360"/>

The end of the game

<img src="./docs/images/menu2.png" alt="3 in 1" width="1920" height="360"/>

Other menus (which took me quite a bit of time to code)

# How do I play?

The game is available on the Web, <a href="https://andrewstephen.xyz/pixper">click here</a> to play.

The game is also available for Windows and Linux. Check the <a href="https://github.com/andrewstephengames/pixper-raylib/releases">releases</a> page.

# Building on Linux

* Compile/install raylib from the official documentation: https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux
* Open your favorite terminal emulator
* `sudo apt install libsqlite3-dev` (Debian/Ubuntu - for other distros look up _distroname_ install pip)
* `git clone https://github.com/andrewstephengames/pixper-raylib`
* `cd pixper-raylib`
* `./build-linux`
* **Enjoy!**
