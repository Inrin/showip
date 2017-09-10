# showip
Simple tool for displaying current interfaces IP-addresses

# Usage
```
showip [-46glu] [interface]
```

Current options/filters are:
* **-4** display IPv4 addresses
* **-6** display IPv6 addresses (Same as -glu)
* **-g** display GUAs (Global Unicast Addresses)
* **-l** display link local addresses
* **-t** display temporary addresses
* **-T** filter temporary addresses out
* **-u** display ULAs (Unique Local addresses)
* **interface** display results for *interface* only

# Building
Just run your preferred compiler:
```
gcc -o showip showip.c
clang -o showip showip.c
```

# Installing
Copy the binary to some directory in your `$PATH` or make an alias/function in your shell.
