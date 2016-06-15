# Filename: src/Makefile
# Project: test-rdtsc
# Brief: Top-lovel Makefile for test-rdtsc -- build libraries and commands
#
# Copyright (C) 2016 Guy Shaw
# Written by Guy Shaw <gshaw@acm.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SRCS_H    := rdtsc.h
SRCS_C    := main.c test-rdtsc.c
SRCS      := $(SRCS_H) $(SRCS_C)
OBJS      := main.o test-rdtsc.o
LIBS      := libcscript/libcscript.a
CPP_FLAGS := -I.
OPT_FLAGS := -O3
DBG_FLAGS := -ggdb
WRN_FLAGS := -Wall -Wextra
CFLAGS    := $(WRN_FLAGS) $(DBG_FLAGS) $(OPT_FLAGS) $(CPP_FLAGS)
CC        := gcc

.PHONY: e ec eh default all dis run install clean-bld clean-run

default: test-rdtsc

all: default dis

e:
	vim $(SRCS)

ec:
	vim $(SRCS_C)

eh:
	vim $(SRCS_H)

test-rdtsc: $(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS)

libcscript/libcscript.a:
	( cd libcscript && make )

run:
	./test-rdtsc 100 ; echo $$?
	./test-rdtsc --show-statistics 100 ; echo $$?
	./test-rdtsc --verbose --show-samples --show-statistics 100 ; echo $$?
	./test-rdtsc --rdtscp --verbose --show-samples 100 ; echo $$?

install:
	cp -p test-rdtsc /usr/local/bin/test-rdtsc

dis: test-rdtsc
	gobjdump --disassemble --reloc $<  > tmp/$<.dis

clean: clean-bld clean-run

clean-bld:
	( cd libcscript && make clean )
	rm -f *.o test-rdtsc

clean-run:
	rm -f core vcore.*
	rm -rf tmp
	mkdir tmp

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
