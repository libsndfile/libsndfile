#include <stdio.h>

#include "sndfile.h"


typedef struct
{	char filename [1024] ;
	char filetype [1024] ;
	
	sf_count_t	frames ;
	int			channels ;
	
	int 		data_hash ;

} RO_TEST_DATA ;

int
main (int argc, char *argv [])
{	
	if (argc != 2)
	{	puts ("Error : expect a filename argument.") ;
		exit (1) ;
		}
	puts ("\nRead Only Test.\n---------------\n") ;
		
	
	return 0 ;
} /* main */
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 1a24e55b-bbce-47dc-b895-26ef12ffe4dc
*/

