


#include "sfconfig.h"

#include <stdio.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include "common.h"
#include "sfendian.h"



int
audio_detect (AUDIO_DETECT *ad, unsigned char * data, int datalen)
{
	if (ad != NULL && data [datalen - 1] == 0)
		return 0 ;

	return 1 ;
} /* data_detect */
