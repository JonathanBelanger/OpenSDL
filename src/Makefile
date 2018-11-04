#
# Copyright (C) Jonathan D. Belanger 2018.
#
#  OpenSDL is free software: you can redistribute it and/or modify it under
#  the terms of the GNU General Public License as published by the Free
#  Software Foundation, either version 3 of the License, or (at your option)
#  any later version.
#
#  OpenSDL is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
#  details.
#
#  You should have received a copy of the GNU General Public License along
#  with OpenSDL.  If not, see <https://www.gnu.org/licenses/>.
#
# Description:
#
#		This make file is used to build the executable for the SDL project.
#
#	Revision History:
#
#	V01.000		18-AUG-2018	Jonathan D. Belanger
#	Initially written.
#
#	V01.001		06-SEP-2018	Jonathan D. Belanger
#	Updated the copyright to be GNUGPL V3 compliant.
#
#	V01.002		08-SEP-2018	Jonathan D. Belanger
#	Change the parent folder name to OpenSDL.
#
HOME		= /cygdrive/g/git/OpenSDL
SRC			= $(HOME)/src
BIN			= $(HOME)/bin
EXE			= $(HOME)/exe
TST			= $(HOME)/test

CFLAGS		= -m64 -std=gnu99 -Wall -I$(SRC) -I$(BIN)
LFLAGS		=
YFLAGS		=
LDFLAGS		= 
SDLDEFS		= $(SRC)/opensdl_defs.h		\
			  $(BIN)/opensdl_lexical.h	\
			  $(BIN)/opensdl_parser.h
OBJS		= $(BIN)/opensdl_actions.o 	\
			  $(BIN)/opensdl_utility.o	\
			  $(BIN)/opensdl_lang_c.o 	\
			  $(BIN)/opensdl_parser.o 	\
			  $(BIN)/opensdl_lexical.o

debug: CFLAGS += -g -O0
debug: LFLAGS += -d
debug: YFLAGS += --debug
debug: all

all: $(EXE)/opensdl.exe $(EXE)/copyright.sdl $(EXE)/struct_test.exe

$(BIN)/opensdl_parser.c $(BIN)/opensdl_parser.h : $(SRC)/opensdl_parser.y
	bison $(YFLAGS) -v --report=lookahead -d -Lc -o $@ $<
#	rm -f $(BIN)/*.output

$(BIN)/opensdl_parser.o : $(BIN)/opensdl_parser.c $(BIN)/opensdl_parser.h $(BIN)/opensdl_lexical.h $(SDLDEFS)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN)/opensdl_lexical.c $(BIN)/opensdl_lexical.h : $(SRC)/opensdl_lexical.l
	flex $(LFLAGS) -o $(BIN)/opensdl_lexical.c --header-file=$(BIN)/opensdl_lexical.h $<

$(BIN)/opensdl_lexical.o : $(BIN)/opensdl_lexical.c $(BIN)/opensdl_lexical.h
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN)/opensdl_actions.o : $(SRC)/opensdl_actions.c $(SDLDEFS)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN)/opensdl_utility.o : $(SRC)/opensdl_utility.c $(SDLDEFS)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN)/opensdl_lang_c.o : $(SRC)/opensdl_lang_c.c $(SDLDEFS)
	$(CC) -c $(CFLAGS) -o $@ $<

$(EXE)/opensdl.exe : $(SRC)/opensdl_main.c $(OBJS) $(BIN)/opensdl_parser.h $(BIN)/opensdl_lexical.h $(SDLDEFS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LDFLAGS)

$(EXE)/copyright.sdl : $(SRC)/copyright.sdl
	cp $< $@

$(EXE)/struct_test.exe : $(TST)/struct_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(BIN)/* $(EXE)/* $(TST)/*.h $(HOME)/*/*.stackdump
