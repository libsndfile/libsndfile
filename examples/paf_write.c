/* 
** Copyright (C) 1999-2003 Erik de Castro Lopo <erikd@mega-nerd.com> 
**   
** This program is free software; you can redistribute it and/or modify 
** it under the terms of the GNU General Public License as published by 
** the Free Software Foundation; either version 2 of the License, or 
** (at your option) any later version. 
**  
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of 
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
** GNU General Public License for more details. 
**  
** You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software  
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
*/ 


#include	<stdio.h> 
#include	<string.h> 
#include	<math.h> 

#include	<sndfile.h> 

#define		SAMPLE_RATE			8000 
#define		SAMPLE_COUNT		100 

int 
main (void) 
{	SNDFILE	*file ; 
	SF_INFO	sfinfo ; 
	char	*filename ;
	int		k, buffer [SAMPLE_COUNT] ; 
	 
	for (k = 0 ; k < SAMPLE_COUNT ; k++)
		buffer [k] = ((3 * k + 2) << 24) + ((3 * k + 1) << 16) + ((3 * k) << 8) ; 

	/* Big endian first. */
	sfinfo.samplerate  = SAMPLE_RATE ; 
	sfinfo.frames     = SAMPLE_COUNT ; 
	sfinfo.channels	   = 1 ; 
	sfinfo.format      = (SF_ENDIAN_BIG | SF_FORMAT_PAF | SF_FORMAT_PCM_24) ;
	
	filename = "be-pcm24.paf" ;
	
	if (! (file = sf_open (filename, SFM_WRITE, &sfinfo))) 
	{	printf ("Error : Not able to open output file.\n") ; 
		return 1 ; 
		} ; 

	printf ("Writing data to '%s'\n", filename) ;

	if (sf_write_int (file, buffer, SAMPLE_COUNT) != SAMPLE_COUNT) 
		puts (sf_strerror (file)) ; 

	sf_close (file) ; 

	/* Little endian first. */
	sfinfo.samplerate  = SAMPLE_RATE ; 
	sfinfo.frames     = SAMPLE_COUNT ; 
	sfinfo.channels	   = 1 ; 
	sfinfo.format      = (SF_ENDIAN_LITTLE | SF_FORMAT_PAF | SF_FORMAT_PCM_24) ;
	
	filename = "le-pcm24.paf" ;
	
	if (! (file = sf_open (filename, SFM_WRITE, &sfinfo))) 
	{	printf ("Error : Not able to open output file.\n") ; 
		return 1 ; 
		} ; 

	printf ("Writing data to '%s'\n", filename) ;

	if (sf_write_int (file, buffer, SAMPLE_COUNT) != SAMPLE_COUNT) 
		puts (sf_strerror (file)) ; 

	sf_close (file) ; 

	return	 0 ; 
} /* main */ 

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 62025b88-1f3f-46fb-994a-8220bf915772
*/
