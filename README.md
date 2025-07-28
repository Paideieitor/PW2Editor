# PW2Editor
Poorly coded Pokémon White 2 Editor  
PW2Editor attaches to your CTRMap project and allows you to modify relevant narcs. aswell as adding modern features like Fairy type and a Battle Engine expansion

## Features
### Pokémon Data Editor
Change all the relevant information of any Pokémon, from the type, to the learnset, to the Pokédex description
### Item Data Editor
Change the information of the game items from the use data, to the name and description
### Move Data Editor
Change moves' information from the damage dealt, to it's effects and text descriptions
### Location and Encounter Data Editor
Change encounters in any route, you can also rename locations and assign encounters to diferent location names
If you install the patch with the ``REMOVE_LOCATION_LIMIT`` option, you can also create new locations
### Trainer Data Editor
Change trainer and their teams' information
If you install the patch with the ``PERFECT_TRAINERS`` option, you get access to the fully customizable trainer editor
### Patcher
Requires PMC to be installed on your CTRMap project
The Patcher tool can install the most up-to-date features from PW2Code, this features are fully customizable so if you don't want some of them you can just disable them

## How to install
- Install [Java](https://www.java.com/es/download/manual.jsp)
- This editor attaches to your CTRMap project, if you don't already have one you will need to set up that first:  
``WARNING: If you want to use the patcher features, your CTRMap project should be created from a Clean USA Pokémon White 2 ROM``  
  - Install [CTRMap-CE](https://github.com/ds-pokemon-hacking/CTRMap-CE/releases)
  - Install [CTRMapV](https://github.com/ds-pokemon-hacking/CTRMapV/releases)
  - Create your CTRMap project
  - Install [PMC](https://github.com/ds-pokemon-hacking/PMC/releases) on to your project (you can follow this [guide](https://ds-pokemon-hacking.github.io/docs/generation-v/guides/bw_b2w2-code_injection/#setting-up-the-environment))
- Download the latest [release of PW2Editor](https://github.com/Paideieitor/PW2Editor/releases)
- Now to attach to a CTRMap project:
  - Open PW2Editor
  - Click *Create Project*
  - Using the file explorer navigate to your CTRMap project and select the project file

## How to use the Patcher
- Go to ``Patcher``->``Options``
- Enable or disable the features as you wish
- Click ``Patcher``->``Build``
A command prompt will open with current information about the build process, when it finishes the editor will shut down so that the new features can be loaded on restart
