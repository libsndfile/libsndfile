#include <stdlib.h>

#include <sndfile.h>

#include "utils.h"

int main (void) {
	SF_INFO info ;
	SNDFILE * f ;
	short shorts [1024] ;
	sf_count_t n_read = 0 ;

	print_test_name ("mp3", "read") ;
	f = sf_open ("<example file goes here>", SFM_READ, &info) ;
	if (f == NULL)
		exit (1) ;
	else
	{	for (unsigned i = 0 ; i < 5 ; i++)
		{	n_read = sf_readf_short (f, shorts, 512) ;
			if (n_read != 1024)
				exit (1) ;
		}
		sf_close (f) ;
		puts ("ok") ;
	}
}
