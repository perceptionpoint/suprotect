# suprotect
A Proof-Of-Concept Linux kernel module for changing memory protection in an arbitrary process (x64 only).

## Instructions
To compile the kernel module and the user-mode utility, run `make`. To clean the project directory, run `make clean`.

To load the kernel module, run `sudo insmod suprotect.ko`. To unload the kernel module, run `sudo rmmod suprotect.ko`.

To change memory protection, run the uesr-mode utility `suprotect-cli` with this usage (while the kernel module is loaded, of course):
```
Usage: ./suprotect-cli <pid> <addr> <len> <prot>
Note: all the parameters are in hex, except the pid.
```

## Example
Let's run python with an infinite loop:
```
ron@ron-ubuntu:~/suprotect$ python -c "while True: pass" &
[1] 4504
```

First, check its memory mapping:
```
ron@ron-ubuntu:~/suprotect$ sudo cat /proc/4504/maps
56143b00d000-56143b315000 r-xp 00000000 08:01 1579941                    /usr/bin/python2.7
56143b515000-56143b517000 r--p 00308000 08:01 1579941                    /usr/bin/python2.7
56143b517000-56143b58d000 rw-p 0030a000 08:01 1579941                    /usr/bin/python2.7
...
```

Remove the execute access for the entire executable region (prot=1 is read only):
```
ron@ron-ubuntu:~/suprotect$ sudo ./suprotect-cli 4504 0x56143b00d000 0x308000 1
```

Immediately, the python process is killed due to segmentation fault:
```
[1]+  Segmentation fault      (core dumped) python -c "while True: pass"
```