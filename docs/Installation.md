# Installation

Good luck lol


## Installing ANTLR4

Install Java

```
sudo apt install open-jdk
```

Execute the script by running

```bash
./install-antlr.sh
```

If you get a permission error, this is fine, you can fix it by running: 

```bash 
chmod 755 ./install-antlr.sh
```

and trying to run the script again.

You may be running into issues with missing packages I may be missing some but here are some fixes

### Missing a uuid package

```bash 
sudo apt install uuid-dev
```

### Some git thing error not authorizing a certain protocol (probably https)

I think it's related to Github not liking https cloning anymore.
```
git config --global url."https://github.com/".insteadOf git://github.com/
```

### Some java error
fix your java, idk...


## Compiling the code

Navigate to the compiler and run make. you may need to install make.

```
cd compiler
make
```

If make still doesn't work, try running the runmake script that corresponds for your OS

On Ubuntu: 

```bash
./runmake_ubuntu.sh
```

On macOS: 

```bash
./runmake_mac.sh
```

On fedora: 

```bash
./runmake_fedora.sh
```

If you get permission errors you can run a `chmod` on it:
 
```bash 
chmod 755 ./runmake_<YOUR OS HERE>.sh
```

hope that works!