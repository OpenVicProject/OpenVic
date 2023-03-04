# Project Structure Guide

## CPP Structure

All C++ files are in the OpenVic2/extension/ directory

Please see the [C++ Styleguide](./styleguide-cpp.md) for more details

## Godot Structure

All godot files are in the OpenVic2/game directory

### Directories

* r/, root/, res/, res://
    * The root directory of the Godot Project
* u/, user/, user://
    * The user directory used by the project (Platform specific)
    * Windows u/ - %APPDATA%/OpenVic2/
    * MacOS u/ - ~/Library/Application Support/OpenVic2/
    * Linux u/ - ~/.local/share/OpenVic2/
* d/, data/, common/ - The data directory used to store data files
    * can be a subdirectory of root/ - root/common/
    * can be a subdirectory of user/ - user/common/
    * files in this directory should not be included in the godot export, but as external files in the same directory as the executable

### Definitions

* core - OpenVic2 without modification
* mod - OpenVic2 with changes in files to modify the gameplay experience
* scene - godot engine scene object
* script - godot engine gdscript object
* resource - godot engine resource object
* asset - static file generally used as is within the project - art, music etc.
* data file - static file that holds data or information that needs to be processed to be used - economy good information, province names etc.

### OpenVic2 File Extensions

* ov2d - OpenVic2 data file
* ov2b - OpenVic2 binary file

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

### When to use data/

* Any data file that gets read in by the program
* Any data file that expresses game conditions
* Can be used within both root/ and user/ - see above rules to know when

#### Rule of thumb

Any sort of modifiable data file should be in data/, core files will be in root/ and mod files will be in data/

### Scene, Scripts, and Resources Organization

No scene files should be in the root/src/ directory

Resources, scripts etc. that are used universally across the application should have a distinct subdirectory
within root/src/

Scene files should be stored in subdirectories of root/src/ organized by the relevancy to each other

Complex Scenes that make use of a top level management system should use further subdirectories
to organize their structure

Resources, scenes, scripts etc. used exclusively within that scene should be stored in the scenes subdirectory

A scene and its script should always be together within the directory i.e. no /scripts/ and /scenes/ directories should exist within
the project somewhere.

Example:
```
root/
|- src/
    |- Theme/
    |   |- ButtonNormal.tres
    |   |- ButtonHover.tres
    |   |- ButtonDisabled.tres
    |- Utility/
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

#### Naming Convention

Subdirectories for Godot files should be PascalCase.
```
root/src/GameSession/
```

File names should be PascalCase.
```
root/src/GameSession/Map.tscn
```

### Asset Organization

Note: Asset files should not be exported with Godot in a pck file. They should be downloaded via an installer or handled in a similar
fashion to the data files and their exclusion from the export

Asset files should be organized within a relevant category from root/
```
root/images/
root/audio/
root/models/
```

Asset files should be organized in a subdirectory that specifies it's relevancy within the application.
```
root/art/economy/
root/art/military/
root/audio/sfx/
root/audio/music/
```


#### Naming Convention

Asset file subdirectories should be snake_case.
```
root/art/icons/goods/
```

Asset files should be PascalCase for their name.
```
TheCrown.mp3
Coal.png
```


### Data File Organization

All data files should be in the root/common/ directory.

Data files should be organized by category.
```
root/common/map/
root/common/technology/
root/common/pop/
```

Data files that are plain text should have the ov2d file extension.
```
root/common/pop/soldier.ov2d
```

Data files that are complex data like a bitmap should make use of the standard file extension for that data
```
root/common/map/provinces.bmp
```

Data files can be further organized by subdirectory.
```
root/common/economy/goods/Coal.ov2d
```

#### Naming Convention

Data file subdirectories should be snake_case.
```
root/common/economy/technology/
```

Data files should be PascalCase.
```
root/common/economy/goods/Coal.txt
```
