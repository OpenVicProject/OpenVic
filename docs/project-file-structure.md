# Project Structure Guide

## CPP Structure

All C++ files are in the OpenVic2/extension/ directory

Please see the [C++ Styleguide](./styleguide-cpp.md) for more details

## Godot Structure

All godot files are in the OpenVic2/game directory

### Definitions

* root/, res/, r/ -> The root directory of the Godot project (OpenVic2/game/)
* user/, u/ -> The user directory used by the project (Platform specific)
* core -> related to the functionality of OpenVic2 without modification

> Windows u/ -> %APPDATA%/OpenVic2/
>
> MacOS u/ -> ~/Library/Application Support/OpenVic2/
>
> Linux u/ -> ~/.OpenVic2/

### When to use root/

Any files that are integral to the basic functionality of the project, with the condition
that these files are immutable. The application should only ever read from these files.

#### Examples

* Core art assets - good icons, 3D models etc.
* Core sound assets - soundtrack, sfx etc.
* Core simulation files - goods, pops etc.
* Core map files - terrain, provinces etc.

#### Rule of thumb

Files that are not modified by the running application should be in root/

### When to use user/

* Any file that has a mutable state within the game needs to be stored in user/ to avoid
permissions issues. I.e. if the application writes to these files in any capacity
* Any overloaded data through the use of mods should be stored in user/ to maintain project integrity.
* Game cache data should be written to user/
* Files generated at runtime by the application need to be stored in user/ to avoid permission issues.

#### Rule of thumb

Files that are modified by the running application should be in user/

### Scene, Scripts, and Resources Organization

No scene files should be in the root/src/ directory

Resources, scripts etc. that are used universally across the application should have a distinct subdirectory
within root/src/

Scene files should be stored in subdirectories of root/src/ organized by the relevancy to each other

Complex Scenes that make use of a top level management system should use further subdirectories
to organize their structure

Resources, scenes, scripts etc. used exclusively within that scene should be stored in the scenes subdirectory

Example:
```
root/
|- src/
    |- Theme/
    |   |- ButtonNormal.tres
    |   |- ButtonHover.tres
    |   |- ButtonDisabled.tres
    |- Scripts/
    |   |- Math.gd
    |- Menu/
    |   |- MenuManager.tscn
    |   |- MenuManager.gd
    |   |- MainMenu/
    |   |   |- MainMenu.tscn
    |   |   |- MainMenu.gd
    |   |   |- MainMenuFont.ttf
    |   |   |- MainMenuLabel.tres
    |   |- SettingsMenu/
    |   |   |- Settings.tscn
    |   |   |- Settings.gd
    |   |   |- Resolution/
    |   |   |   |- ResolutionSelector.gd
    |   |   |- Volume/
    |   |   |   |- VolumeGrid.tscn
    |   |   |   |- VolumeGrid.gd
```
