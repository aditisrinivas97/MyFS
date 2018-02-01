# MyFS

A filesystem in user space built using FUSE, a software interface for Unix-like computer operating systems that lets non-privileged users create their own file systems without editing kernel code. This project was created as a part of the "Unix System Programming" course at PES University, 2018.
This is a naive implementation as the persistence of data does not pertain to any disk, rather the data is retained upon unmount by writing to and reading from files.

## Table of contents

- [Description](https://github.com/aditisrinivas97/MyFS#Description)
- [Features](https://github.com/aditisrinivas97/MyFS#Features)
- [Installing Dependencies](https://github.com/aditisrinivas97/MyFS#Installing-Dependencies)

## Description

FUSE or File System in User Space module provides a "bridge" to the actual kernel interfaces by running file system code in user space. Creating a file system can be achieved by writing a [Virtual File System](https://en.wikipedia.org/wiki/Virtual_file_system) which is nothing but an abstraction layer above a more concrete file system to provide custom access to users. Once the VFS is mounted, the handler is registered with the kernel. If a user then requests for read, write etc over this newly mounted FS, the kernel forwards these requests to the handler and then sends the handler's response back to the user. The handler, thus, manages the system calls. The idea behind such a concept is to add a layer of abstraction such that the user need not look into the smallest of details.

## Features

The following features are supported in this File System.

1. mkdir    -   Create a directory.
2. rmdir    -   Delete an empty directory.
3. getattr  -   Return file attributes.
4. readdir  -   Return one or more directory entries (struct dirent) to the caller. 
5. mknod    -   Make a special (device) file, FIFO, or socket. 
6. open     -   Open a file.
7. read     -   Read size bytes from the given file into the buffer buf, beginning offset bytes into the file.
8. write    -   Read size bytes from the given buffer buf to the file, beginning offset bytes into the file.
9. unlink   -   Remove (delete) the given file.
10. chmod   -   Change the mode (permissions) of the given object to the given new permissions.
11. utimens -   Update the last access time of the given object.

## Installing Dependencies

The file system was built over the Distro, Ubuntu 16.04 Xenial Xerus. The fuse version used for this project was FUSE 2.9.4.
To install libfuse-dev package on Ubuntu 16.04 (Xenial Xerus), run the following commands.
```
$ sudo apt-get update
$ sudo apt-get install libfuse-dev
```

## License

This project is made available under the [MIT License](http://www.opensource.org/licenses/mit-license.php).

## Credits

The project is created and maintained by [Aditi Srinivas](https://github.com/aditisrinivas97), [Aishwarya Manjunath](https://github.com/Aishwarya-Manjunath) and [Akanksha Somayaji](https://github.com/AkankshaSomayaji).
