# $Id: Makefile,v 1.1.1.1 2007-08-06 10:04:43 root Exp $
CFLAGS = -Wall -g

all:	testobsdrdr testgetifstats

testobsdrdr:	testobsdrdr.o obsdrdr.o

obsdrdr.o:	obsdrdr.c obsdrdr.h

testobsdrdr.o:	testobsdrdr.c obsdrdr.h

testgetifstats:	testgetifstats.o getifstats.o -lkvm

