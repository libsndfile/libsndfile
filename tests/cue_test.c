
#include "sfconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>


static int test_cues (const char *filename);
int test_cues (const char *filename)
{
    SNDFILE    *file;
    SF_INFO	sfinfo;

    int size, i;	/* long is UInt32 is 32bit, int might be 64bit */
    SF_CUE_INFO *info;
    int err;

    if ((file = sf_open(filename, SFM_READ, &sfinfo)) == NULL)
    {
	printf("can't open file '%s'\n", filename);
	return 0;
    }

    if ((err = sf_command(file, SFC_GET_CUE_SIZE, &size, sizeof (int))) == SF_FALSE)
    {
	printf("can't get cue info size for file '%s' (arg size %lu, err %d)\n", 
	       filename, sizeof(int), err);
	return 0;
    }

    if (!(info = malloc(size)))
	return 0;

    if (sf_command(file, SFC_GET_CUE_INFO, info, size) == SF_FALSE)
    {
	printf("can't get cue info of size %d for file '%s'\n", 
	       size, filename);
	return 0;
    }

    printf("number of cues %d  in file %s  sr %d\n", info->num, filename, sfinfo.samplerate);

    for (i = 0; i < info->num; i++)
    {
	int    pos = info->cue[i].position;
	double t   = (double) pos / sfinfo.samplerate;
	double expected = i < 10  ?  (double) i / 3.  :  11. / 3.;

	printf("cue %02d: markerID %02d  position %06d  (time %.3f  diff %f)  label '%s'\n", i, info->cue[i].markerID, pos, t, (double) abs(t - expected), info->cue[i].label);
    }
    
    sf_close(file);

    return 1;
}


int main (int argc, char **argv)
{
    test_cues("clickpluck24.wav");
    test_cues("clickpluck.wav");
    test_cues("clickpluck.aiff");
}
