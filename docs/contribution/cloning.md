# Cloning

If you do not have Git installed foremost you need to install it at https://git-scm.com/download.

Once installed you need to clone OpenVic:
```sh
git clone https://github.com/OpenVicProject/OpenVic.git
```
After cloning you must initialize and update the submodules:
```sh
cd OpenVic
git submodule update --init --recursive
```
Updating the submodules is necessary every time the dependencies are updated, if you're unsure, on every `git pull` you should also call `git submodule update --init --recursive`

To update the repo you must pull the repo:
```sh
git pull
```
Git may install a gui with it, this gui can be a useful external tool for doing things related to git, (especially commits) you can open it via `git gui`.

Further tutorials and documentation on git can be found at https://git-scm.com/book.