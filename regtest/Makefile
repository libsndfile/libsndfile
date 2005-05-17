TARGETS = sndfile-regtest

CC = gcc

CFLAGS = -W -Wall -Werror -Wstrict-prototypes -Wshadow

SNDFILE_CFLAG = $(shell pkg-config --cflags sndfile)
SNDFILE_LIBS = $(shell pkg-config --libs sndfile)

SQL_LIBS = -lsqlite3


all: $(TARGETS)

clean :
	rm -rf $(TARGETS) *.o

sndfile-regtest : checksum.o database.o regtest.o
	$(CC) $+ $(SNDFILE_LIBS) $(SQL_LIBS) -o $@

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch
# revision control system.
#
# arch-tag: ac4a9626-49ce-4bb4-9fb6-e43de0a23432
