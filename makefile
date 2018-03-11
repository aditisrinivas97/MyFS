username = $(shell whoami)
files = fsmain.c fsoperations.c fstree.c bitmap.c fsdisk.c
opflag = -o fs
flags = `pkg-config fuse --cflags --libs` -DFUSE_USE_VERSION=25 -lm -g

all: run

run: compile
	./fs -f /home/$(username)/Desktop/mountpoint7

debugrun: dcompile
	./fs -d -f -s /home/$(username)/Desktop/mountpoint7

compile: checkdir
	gcc -Wall $(files) $(opflag) $(flags)

dcompile: checkdir	
	gcc -Wall -g $(files) $(opflag) $(flags)

checkdir:
	if [ -d "/home/$(username)/Desktop/mountpoint7" ]; then echo "mountpoint7 exists"; else mkdir /home/$(username)/Desktop/mountpoint7; fi
