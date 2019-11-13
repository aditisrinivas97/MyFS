# MyFS
[![Open Source Love](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)]()
![Status](https://img.shields.io/badge/status-active-brightgreen.svg?style=flat)
[![License](https://img.shields.io/badge/license-mit-brightgreen.svg?style=flat)](https://github.com/aditisrinivas97/MyFS/blob/master/LICENSE)
![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)

A filesystem in user space built using FUSE, a software interface for Unix-like computer operating systems that lets non-privileged users create their own file systems without editing kernel code. This project was created as a part of the "Unix System Programming" course at PES University, 2018.

## Table of contents

- [Description](https://github.com/aditisrinivas97/MyFS#Description)
- [Features](https://github.com/aditisrinivas97/MyFS#Features)
- [Installing Dependencies](https://github.com/aditisrinivas97/MyFS#Installing-Dependencies)
- [Using this filesystem](https://github.com/aditisrinivas97/MyFS#Installing-Dependencies)
- [License](https://github.com/aditisrinivas97/MyFS#License)
- [Credits](https://github.com/aditisrinivas97/MyFS#Credits)


## Description

FUSE or File System in User Space module provides a "bridge" to the actual kernel interfaces by running file system code in user space. Creating a file system can be achieved by writing a [Virtual File System](https://en.wikipedia.org/wiki/Virtual_file_system) which is nothing but an abstraction layer above a more concrete file system to provide custom access to users.

## Features

The following operations are supported in this file system

1. Create a directory.
2. Remove a directory - both empty and non-empty.
3. Create a file using touch, nano, gedit etc.
4. Delete an existing file.
5. Appending to and truncating a file.
6. Change the permissions of a file or directory.
7. Access, modified and status change time updates.
8. Open and close a file.
9. Read and write to files.
10. Persistence of all aspects of the file system.

## Installing Dependencies

The file system was built over the Distro, Ubuntu 16.04 Xenial Xerus. The fuse version used for this project was FUSE 2.9.4.
To install libfuse-dev package on Ubuntu 16.04 (Xenial Xerus), run the following commands.
```
$ sudo apt-get update
$ sudo apt-get install libfuse-dev
```

## Using this File System

Clone this repository
```
$ git clone https://github.com/aditisrinivas97/MyFS MyFS
```

Cd into the folder 'MyFS' and mount the filesystem using the makefile
```
$ cd MyFS
$ make
```

To view the error logs of fuse and check for memory leaks / errors, run the following commands
```
$ cd MyFS
$ make debugrun
```

cd into the mountpoint which is present at ~/Desktop/mountpoint and use the filesystem!
```
$ cd ~/Desktop/mountpoint
```

To unmount the filesystem run the following commands
```
$ cd ~/Desktop
$ sudo umount mountpoint
```

## License

This project is made available under the [MIT License](http://www.opensource.org/licenses/mit-license.php).

## Credits

The project is created and maintained by [Aditi Srinivas](https://github.com/aditisrinivas97), [Aishwarya Manjunath](https://github.com/Aishwarya-Manjunath) and [Akanksha Somayaji](https://github.com/AkankshaSomayaji).
