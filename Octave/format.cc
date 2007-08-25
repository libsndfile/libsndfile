/*
** Copyright (C) 2007 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <string>
#include <vector>
#include <cctype>

#include "sndfile.h"

#include "format.h"

static void
str_split (const std::string & str, const std::string & delim, std::vector <std::string> & output)
{
    unsigned int offset = 0 ;
    unsigned int delim_index = 0 ;

    delim_index = str.find (delim, offset) ;

    while (delim_index != std::string::npos)
    {
        output.push_back (str.substr(offset, delim_index - offset)) ;
        offset += delim_index - offset + delim.length () ;
        delim_index = str.find (delim, offset) ;
    }

    output.push_back (str.substr (offset)) ;
} /* str_split */

static int
hash_of_str (const std::string & str)
{
	int hash = 0 ;

	for (unsigned k = 0 ; k < str.length () ; k++)
		hash = (hash * 3) + tolower (str [k]) ;

	return hash ;
} /* hash_of_str */

static int
major_format_of_hash (const std::string & str)
{	int hash ;

	hash = hash_of_str (str) ;

	switch (hash)
	{
		case 0x5c8 : /* 'wav' */ return SF_FORMAT_WAV ;
		case 0xf84 : /* 'aiff' */ return SF_FORMAT_AIFF ;
		case 0x198 : /* 'au' */ return SF_FORMAT_AU ;
		case 0x579 : /* 'paf' */ return SF_FORMAT_PAF ;
		case 0x5e5 : /* 'svx' */ return SF_FORMAT_SVX ;
		case 0x1118 : /* 'nist' */ return SF_FORMAT_NIST ;
		case 0x5d6 : /* 'voc' */ return SF_FORMAT_VOC ;
		case 0x324a : /* 'ircam' */ return SF_FORMAT_IRCAM ;
		case 0x505 : /* 'w64' */ return SF_FORMAT_W64 ;
		case 0x1078 : /* 'mat4' */ return SF_FORMAT_MAT4 ;
		case 0x1079 : /* 'mat5' */ return SF_FORMAT_MAT5 ;
		case 0x5b8 : /* 'pvf' */ return SF_FORMAT_PVF ;
		case 0x1d1 : /* 'xi' */ return SF_FORMAT_XI ;
		case 0x56f : /* 'htk' */ return SF_FORMAT_HTK ;
		case 0x5aa : /* 'sds' */ return SF_FORMAT_SDS ;
		case 0x53d : /* 'avr' */ return SF_FORMAT_AVR ;
		case 0x11d0 : /* 'wavx' */ return SF_FORMAT_WAVEX ;
		case 0x569 : /* 'sd2' */ return SF_FORMAT_SD2 ;
		case 0x1014 : /* 'flac' */ return SF_FORMAT_FLAC ;
		case 0x504 : /* 'caf' */ return SF_FORMAT_CAF ;
		case 0x5f6 : /* 'wve' */ return SF_FORMAT_WVE ;
		default : break ;
		} ;

	printf ("%s : hash '%s' -> 0x%x\n", __func__, str.c_str (), hash) ;

	return 0 ;
} /* major_format_of_hash */

static int
minor_format_of_hash (const std::string & str)
{	int hash ;

	hash = hash_of_str (str) ;

	switch (hash)
	{
		case 0x1085 : /* 'int8' */ return SF_FORMAT_PCM_S8 ;
		case 0x358a : /* 'uint8' */ return SF_FORMAT_PCM_U8 ;
		case 0x31b0 : /* 'int16' */ return SF_FORMAT_PCM_16 ;
		case 0x31b1 : /* 'int24' */ return SF_FORMAT_PCM_24 ;
		case 0x31b2 : /* 'int32' */ return SF_FORMAT_PCM_32 ;
		case 0x3128 : /* 'float' */ return SF_FORMAT_FLOAT ;
		case 0x937d : /* 'double' */ return SF_FORMAT_DOUBLE ;
		case 0x11bd : /* 'ulaw' */ return SF_FORMAT_ULAW ;
		case 0xfa1 : /* 'alaw' */ return SF_FORMAT_ALAW ;
		case 0xfc361 : /* 'ima_adpcm' */ return SF_FORMAT_IMA_ADPCM ;
		case 0x5739a : /* 'ms_adpcm' */ return SF_FORMAT_MS_ADPCM ;
		case 0x9450 : /* 'gsm610' */ return SF_FORMAT_GSM610 ;
		case 0x172a3 : /* 'g721_32' */ return SF_FORMAT_G721_32 ;
		case 0x172d8 : /* 'g723_24' */ return SF_FORMAT_G723_24 ;
		case 0x172da : /* 'g723_40' */ return SF_FORMAT_G723_40 ;
		default : break ;
		} ;

	printf ("%s : hash '%s' -> 0x%x\n", __func__, str.c_str (), hash) ;

	return 0 ;
} /* minor_format_of_hash */


int
format_of_str (const std::string & fmt)
{
	std::vector <std::string> split ;

	str_split (fmt, "-", split) ;

	if (split.size () != 2)
		return 0 ;

	int major_fmt = major_format_of_hash (split.at (0)) ;
	if (major_fmt == 0)
		return 0 ;

	int minor_fmt = minor_format_of_hash (split.at (1)) ;
	if (minor_fmt == 0)
		return 0 ;

	return major_fmt | minor_fmt ;
} /* format_of_str */

static const char *
string_of_major_format (int format)
{
	switch (format & SF_FORMAT_TYPEMASK)
	{
		case SF_FORMAT_WAV : return "wav" ;
		case SF_FORMAT_AIFF : return "aiff" ;
		case SF_FORMAT_AU : return "au" ;
		case SF_FORMAT_PAF : return "paf" ;
		case SF_FORMAT_SVX : return "svx" ;
		case SF_FORMAT_NIST : return "nist" ;
		case SF_FORMAT_VOC : return "voc" ;
		case SF_FORMAT_IRCAM : return "ircam" ;
		case SF_FORMAT_W64 : return "w64" ;
		case SF_FORMAT_MAT4 : return "mat4" ;
		case SF_FORMAT_MAT5 : return "mat5" ;
		case SF_FORMAT_PVF : return "pvf" ;
		case SF_FORMAT_XI : return "xi" ;
		case SF_FORMAT_HTK : return "htk" ;
		case SF_FORMAT_SDS : return "sds" ;
		case SF_FORMAT_AVR : return "avr" ;
		case SF_FORMAT_WAVEX : return "wavx" ;
		case SF_FORMAT_SD2 : return "sd2" ;
		case SF_FORMAT_FLAC : return "flac" ;
		case SF_FORMAT_CAF : return "caf" ;
		case SF_FORMAT_WVE : return "wfe" ;
		default : break ;
		} ;

	return "unknown" ;
} /* string_of_major_format */

static const char *
string_of_minor_format (int format)
{
	switch (format & SF_FORMAT_SUBMASK)
	{
		case SF_FORMAT_PCM_S8 : return "int8" ;
		case SF_FORMAT_PCM_U8 : return "uint8" ;
		case SF_FORMAT_PCM_16 : return "int16" ;
		case SF_FORMAT_PCM_24 : return "int24" ;
		case SF_FORMAT_PCM_32 : return "int32" ;
		case SF_FORMAT_FLOAT : return "float" ;
		case SF_FORMAT_DOUBLE : return "double" ;
		case SF_FORMAT_ULAW : return "ulaw" ;
		case SF_FORMAT_ALAW : return "alaw" ;
		case SF_FORMAT_IMA_ADPCM : return "ima_adpcm" ;
		case SF_FORMAT_MS_ADPCM : return "ms_adpcm" ;
		case SF_FORMAT_GSM610 : return "gsm610" ;
		case SF_FORMAT_G721_32 : return "g721_32" ;
		case SF_FORMAT_G723_24 : return "g723_24" ;
		case SF_FORMAT_G723_40 : return "g723_40" ;
		default : break ;
		} ;
	
	return "unknown" ;
} /* string_of_minor_format */

void
string_of_format (std::string & fmt, int format)
{
	char buffer [64] ;

	snprintf (buffer, sizeof (buffer), "%s-%s", string_of_major_format (format), string_of_minor_format (format)) ;

	fmt = buffer ;

	return ;
} /* string_of_format */
