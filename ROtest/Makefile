

TARGETS = ro_test

SNDFILE_LIB = ../src/.libs/libsndfile.a
SNDFILE_INC = ../src
SNDFILE_HEADER = $(SNDFILE_INC)/sndfile.h

all : $(TARGETS)


libsndfile.a : $(SNDFILE_LIB)
	cp -f $(SNDFILE_LIB) .

sndfile.h : $(SNDFILE_HEADER)
	cp -f $(SNDFILE_HEADER) .

ro_test: ro_test.c libsndfile.a sndfile.h
	gcc -Wall ro_test.c -static libsndfile.a -o ro_test

clean:
	rm -f $(TARGETS)

check: $(TARGETS) ro.data
	./ro_test ro.data

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: baac643d-a847-486d-aa90-444bcb2d433d
