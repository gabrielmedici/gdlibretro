Uhm, aksually, youre supposed to use scons to build gdextension projects...\
I. don't. care.\
For all i know scons sucks ass. I couldn't even build the example gdext project.\
You'll have to deal with CMake. at least it works

# Things you should know
- This is a hobby project, i have the right to not give a damn about it
- Still, i'll try to help if you ask nicely
- If launching with the provided debug dosbox pure dlls expect a "stack destroyed" exception. **Just ignore it and move on**

# Building
- Windows only for now
- Have msvc or try another compiler, idk, do what you want (the debugging configured as is will only work if you have a full msvc installation)
- Have CMake
- Pull the submodules
- Compile

# Debugging (VSCode)
- Go into .vscode/launch.json
- Find the godot path on the "Windows Launch" task
- Change it for your godot's installation path

# Launching
- Open the project on godot and launch it
- Click the debug button on vscode, it'll automatically launch the godot project in play mode and allow you to debug it

# Changing the core
- Put the core you want to launch inside the "libretro-cores" folder on the godot project directory
- Modifiy the argument on node3d.gd function call of RetroHost.load_core to the name of the core dll **without the .dll**

# Exported projects
It should work with exported projects since outside the editor it loads dlls the windows way ([first on the executable directory, then on the system directory and so on](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order))\
You just need to have your core's dll on one of those places and it should work.
