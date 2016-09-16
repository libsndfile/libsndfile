/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "gsm.h"

#ifdef	HAVE_STDLIB_H
#	include	<stdlib.h>
#else
#	ifdef	HAVE_MALLOC_H
#		include 	<malloc.h>
#	else
		extern void free () ;
#	endif
#endif

void gsm_destroy (gsm S)
{
	if (S)
		free ((char *) S) ;
}

