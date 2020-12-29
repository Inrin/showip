# showip
Simple tool for displaying current interfaces IP-addresses

# Usage
```
showip [-h] [-46gltTu] [interface]
```

Current options/filters are:
* **-h** display help
* **-4** display IPv4 addresses
* **-6** display IPv6 addresses (Same as -glut)
* **-g** display GUAs (Global Unicast Addresses)
* **-l** display LLAs (Link Local Addresses
* **-t** display temporary addresses
* **-T** filter temporary addresses out
* **-u** display ULAs (Unique Local Addresses)
* **interface** display results for *interface* only

# Building
Just run your preferred compiler:
```
gcc -o showip showip.c
clang -o showip showip.c
```

# Installing
Copy the binary to some directory in your `$PATH` or make an alias/function in your shell.

# Tested Distributions
* Arch Linux (29.12.2020)
* Arch Linux ARM (29.12.2020 on Raspberry Pi 3)
* Scientific Linux 7.8
* Ubuntu 18.04.5
* Ubuntu 20.04.1
