
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

    unsigned int i, err, size;
    uint32_t count = 0;
    SF_CUES_VAR(0) *info;

    if ((file = sf_open(filename, SFM_READ, &sfinfo)) == NULL)
    {
	printf("can't open file '%s'\n", filename);
	return 0;
    }

    printf("\n---- get cues of file '%s'\n", filename);

    if ((err = sf_command(file, SFC_GET_CUE_COUNT, &count, sizeof(uint32_t))) == SF_FALSE)
    {
	if (sf_error(file))
	    printf("can't get cue info size for file '%s' (arg size %lu), err %s\n", 
		   filename, sizeof(uint32_t), sf_strerror(file));
	else
	    printf("no cue info for file '%s'\n", filename);
	return 0;
    }
	
    size = sizeof(*info) + count * sizeof(SF_CUE_POINT);
    printf("num. cues in file '%s': %d  info struct size %d\n", filename, count, size);

    if (!(info = malloc(size)))
	return 0;

    if (sf_command(file, SFC_GET_CUE, info, size) == SF_FALSE)
    {
	printf("can't get cue info of size %d for file '%s' error %s\n", 
	       size, filename, sf_strerror(file));
	return 0;
    }

    printf("number of cues %d  in struct\n", info->cue_count);

    for (i = 0; i < info->cue_count; i++)
    {
	int    pos = info->cue_points[i].position;
	double t   = (double) pos / sfinfo.samplerate;
	double expected = i < 8  ?  (double) i / 3.  :  10. / 3.;

	printf("cue %02d: markerID %02d  position %06d  (time %.3f  expected %.3f  diff %f)  label '%s'\n",
	       i, info->cue_points[i].indx, pos, t, expected, (double) fabs(t - expected), info->cue_points[i].name);
    }
    
    sf_close(file);

    return 1;
}


int main ()
{
    test_cues("clickpluck24.wav");
    test_cues("clickpluck.wav");
    test_cues("clickpluck.aiff");
}
