/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software ; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation ; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY ; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program ; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "sfconfig.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <vorbis/codec.h>

#include "sndfile.h"
#include "sfendian.h"
#include "common.h"

static int ogg_read_header(SF_PRIVATE *psf);
static int ogg_close(SF_PRIVATE *psf);
static int ogg_command (SF_PRIVATE *psf, int command, void *data, int datasize) ;
static int ogg_ulaw_init (SF_PRIVATE *psf) ;
static int ogg_pcm_init (SF_PRIVATE *psf) ;
static int ogg_alaw_init (SF_PRIVATE *psf) ;
static int ogg_float32_init (SF_PRIVATE *psf) ;
static int ogg_double64_init (SF_PRIVATE *psf) ;
static int ogg_g72x_init (SF_PRIVATE *psf) ;

typedef struct {
    ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
    ogg_stream_state os; /* take physical pages, weld into a logical
                            stream of packets */
    ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
    ogg_packet       op; /* one raw packet of data for decode */
    
    vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                            settings */
    vorbis_comment   vc; /* struct that stores all the bitstream user comments */
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block     vb; /* local working space for packet->PCM decode */
    int              eos;
} OGG_DATA;

void foo(void);

static int ogg_read_header(SF_PRIVATE *psf)
{
    char *buffer;
    int  bytes;
    int i, nn;
    OGG_DATA *data = (OGG_DATA*)psf->container_data;

    ogg_sync_init(&(data->oy)); /* Now we can read pages */

    /* grab some data at the head of the stream.  We want the first page
       (which is guaranteed to be small and only contain the Vorbis
       stream initial header) We need the first page to get the stream
       serialno. */
 
    buffer = ogg_sync_buffer(&data->oy,4096L); /* Expose the buffer */
    /* submit a 4k block to libvorbis' Ogg layer */
    /* need to patch up guess type stuff */
    memcpy(buffer, "OggS\0\0\0\0\0\0\0\0", 12);
    buffer[5]=2;
    bytes=psf_fread(buffer+12,1,4096-12,psf);
    ogg_sync_wrote(&(data->oy),bytes+12);
   
    /* Get the first page. */
    if ((nn=ogg_sync_pageout(&(data->oy),&(data->og)))!=1) {
      /* have we simply run out of data?  If so, we're done. */
      if (bytes<4096) return 0;
        
      /* error case.  Must not be Vorbis data */
      psf_log_printf(psf,"Input does not appear to be an Ogg bitstream.\n");
      return SFE_MALFORMED_FILE;
    }
  
   /* Get the serial number and set up the rest of decode. */
    /* serialno first; use it to set up a logical stream */
    ogg_stream_init(&data->os,ogg_page_serialno(&data->og));
   
    /* extract the initial header from the first page and verify that the
       Ogg bitstream is in fact Vorbis data */
    
    /* I handle the initial header first instead of just having the code
       read all three Vorbis headers at once because reading the initial
       header is an easy way to identify a Vorbis bitstream and it's
       useful to see that functionality seperated out. */
    
    vorbis_info_init(&data->vi);
    vorbis_comment_init(&data->vc);
    if (ogg_stream_pagein(&data->os,&data->og)<0) { 
      /* error; stream version mismatch perhaps */
      psf_log_printf(psf,"Error reading first page of Ogg bitstream data\n");
      return SFE_MALFORMED_FILE;
    }
    
    if (ogg_stream_packetout(&data->os,&data->op)!=1) { 
      /* no page? must not be vorbis */
      psf_log_printf(psf,"Error reading initial header packet.\n");
      return SFE_MALFORMED_FILE;
    }
   
    if (vorbis_synthesis_headerin(&data->vi,&data->vc,&data->op)<0) { 
      /* error case; not a vorbis header */
      psf_log_printf(psf,"This Ogg bitstream does not contain Vorbis "
              "audio data->\n");
      return SFE_MALFORMED_FILE;
    }
    psf_log_printf(psf,"JPff: a vorbis file (%d)\n", __LINE__);
    
    /* At this point, we're sure we're Vorbis.  We've set up the logical
       (Ogg) bitstream decoder.  Get the comment and codebook headers and
       set up the Vorbis decoder */
    
    /* The next two packets in order are the comment and codebook headers.
       They're likely large and may span multiple pages.  Thus we reead
       and submit data until we get our two pacakets, watching that no
       pages are missing.  If a page is missing, error out; losing a
       header page is the only place where missing data is fatal. */
    
    i=0;                        /* Count of number of packets read */
    while (i<2) {
      int result = ogg_sync_pageout(&data->oy,&data->og);
      if (result==0) {
        /* Need more data */
        buffer = ogg_sync_buffer(&data->oy,4096);
        bytes = psf_fread(buffer,1,4096,psf);
        if (bytes==0 && i<2) {
          psf_log_printf(psf,"End of file before finding all Vorbis headers!\n");
          return SFE_MALFORMED_FILE;
        }
        nn=ogg_sync_wrote(&data->oy,bytes);
      }
      /* Don't complain about missing or corrupt data yet.  We'll
         catch it at the packet output phase */
      else if (result==1) {
         nn=ogg_stream_pagein(&data->os,&data->og); /* we can ignore any errors here
                                                   as they'll also become apparent
                                                   at packetout */
        while (i<2) {
          result=ogg_stream_packetout(&data->os,&data->op);
          if (result==0) break;
          if (result<0) {
            /* Uh oh; data at some point was corrupted or missing!
               We can't tolerate that in a header.  Die. */
            psf_log_printf(psf,"Corrupt secondary header.  Exiting.\n");
            return SFE_MALFORMED_FILE;
          }
          vorbis_synthesis_headerin(&data->vi,&data->vc,&data->op);
          i++;
         }
      }
      else {
        /*       fprintf(stderr, "JPff:ignoring result=%d\n", result); */
      }
    }

    /* Throw the comments plus a few lines about the bitstream we're
       decoding */
    {
      char **ptr=data->vc.user_comments;
      while(*ptr){
        psf_log_printf(psf,"%s\n",*ptr);
        ++ptr;
      }
      psf_log_printf(psf,"\nBitstream is %d channel, %ldHz\n",
              data->vi.channels,data->vi.rate);
      psf_log_printf(psf,"Encoded by: %s\n\n",data->vc.vendor);
    }
    psf->sf.samplerate	= data->vi.rate;
    psf->sf.channels 	= data->vi.channels;
    //    psf->sf.format      |= SF_FORMAT_VORBIS;

    /* OK, got and parsed all three headers. Initialize the Vorbis
       packet->PCM decoder. */
    vorbis_synthesis_init(&data->vd,&data->vi); /* central decode state */
    vorbis_block_init(&data->vd,&data->vb);     /* Local state for most of the 
                                                   decode so multiple block
                                                   decodes can proceed in parallel.
                                                   We could init multiple
                                                   vorbis_block structures for
                                                   vd here */

    return 0;
}

static int ogg_close(SF_PRIVATE *psf)
{
    OGG_DATA *data = (OGG_DATA*)psf->container_data;
    /* clean up this logical bitstream; before exit we see if we're
       followed by another [chained] */

    ogg_stream_clear(&data->os);
  
    /* ogg_page and ogg_packet structs always point to storage in
       libvorbis.  They're never freed or manipulated directly */
    
    vorbis_block_clear(&data->vb);
    vorbis_dsp_clear(&data->vd);
    vorbis_comment_clear(&data->vc);
    vorbis_info_clear(&data->vi);  /* must be called last */
    /* shoudl look here to reopen if chained */

    /* OK, clean up the framer */
    ogg_sync_clear(&data->oy);
    return 0 ;
}

int
ogg_open	(SF_PRIVATE *psf)
{		int		error = 0 ;
	int		subformat ;

        psf->container_data = malloc(sizeof(OGG_DATA));
        if (psf->mode == SFM_RDWR)
          return SFE_BAD_MODE_RW;
        if (psf->mode == SFM_READ)
          {	if ((error = ogg_read_header (psf)))
              return error ;
          } ;
        if ((psf->sf.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_OGG)
          return	SFE_BAD_OPEN_FORMAT ;

        subformat = psf->sf.format & SF_FORMAT_SUBMASK ;
        psf->container_close = ogg_close ;
        if (psf->mode == SFM_WRITE)
          return SFE_UNIMPLEMENTED ;
    
        psf->blockwidth = psf->bytewidth * psf->sf.channels ;

	psf->command = ogg_command ;
	switch (subformat)
	{	case SF_FORMAT_PCM_S8 :	/* 8-bit linear PCM. */
				error = error = ogg_pcm_init (psf) ;
				break ;

		case SF_FORMAT_PCM_16 :	/* 16-bit linear PCM. */
		case SF_FORMAT_PCM_24 :	/* 24-bit linear PCM */
		case SF_FORMAT_PCM_32 :	/* 32-bit linear PCM. */
				error = ogg_pcm_init (psf) ;
				break ;

		/* Lite remove start */
		case SF_FORMAT_FLOAT :	/* 32-bit floats. */
				error = ogg_float32_init (psf) ;
				break ;

		case SF_FORMAT_DOUBLE :	/* 64-bit double precision floats. */
				error = ogg_double64_init (psf) ;
				break ;

		/* Lite remove end */

		default :	break ;
		} ;

	return error ;
} /* ogg_open */

static int ogg_ulaw_init (SF_PRIVATE *psf) { return 0;}
static int ogg_pcm_init (SF_PRIVATE *psf) { return 0;}
static int ogg_alaw_init (SF_PRIVATE *psf) { return 0;}
static int ogg_float32_init (SF_PRIVATE *psf) { return 0;}
static int ogg_double64_init (SF_PRIVATE *psf) { return 0;}
static int ogg_g72x_init (SF_PRIVATE *psf) { return 0;}

static float **ogg_read_buffer(SF_PRIVATE *psf)
{
    OGG_DATA *data = (OGG_DATA*)psf->container_data;
    char *buffer;
    int bytes;
    float **pcm;

 top:
    {
      int result = ogg_sync_pageout(&data->oy,&data->og);
      if (result==0) {
        /* need more data */
        buffer=ogg_sync_buffer(&data->oy,4096);
        bytes=psf_fread(buffer,1,4096,psf);
        ogg_sync_wrote(&data->oy,bytes);
        if (bytes==0) data->eos=1;
        goto top;
      }
      if (result<0) { /* missing or corrupt data at this page position */
        fprintf(stderr,"Corrupt or missing data in bitstream; "
                "continuing...\n");
        goto top;
      }
      else {
        ogg_stream_pagein(&data->os,&data->og); /* can safely ignore errors at
                                       this point */
        result=ogg_stream_packetout(&data->os,&data->op);
        
        if (result==0) {
          /* need more data */
          buffer=ogg_sync_buffer(&data->oy,4096);
          bytes=psf_fread(buffer,1,4096,psf);
          ogg_sync_wrote(&data->oy,bytes);
          if (bytes==0) data->eos=1;
          goto top;
        }
        if (result>0) {
          /* we have a packet.  Return it */
          int samples;
	  
          if (vorbis_synthesis(&data->vb,&data->op)==0) /* test for success! */
            vorbis_synthesis_blockin(&data->vd,&data->vb);
          /* 
             
          **pcm is a multichannel float vector.  In stereo, for
          example, pcm[0] is left, and pcm[1] is right.  samples is
          the size of each channel.  Convert the float values
          (-1.<=range<=1.) to whatever PCM format and write it out */
	  
          if ((samples=vorbis_synthesis_pcmout(&data->vd,&pcm))>0) {
            vorbis_synthesis_read(&data->vd,samples/psf->sf.channels); /* tell libvorbis how
                                                many samples we
                                                actually consumed */
          }
        }
        else  /* missing or corrupt data at this page position */
          /* no reason to complain; already complained above */
          goto top;
        if (ogg_page_eos(&data->og)) data->eos=1;
        return pcm;
      }
    }
}

static int ogg_command (SF_PRIVATE *psf, int command, void *data, int datasize)
{
    return 0;
}

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 9ff1fe9c-629e-4e9c-9ef5-3d0eb1e427a0
*/
