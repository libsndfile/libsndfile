/*
** Copyright (C) 1999-2003 Erik de Castro Lopo <erikd@zip.com.au>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined (__linux__)
	#include 	<fcntl.h>
	#include 	<sys/ioctl.h>
	#include 	<sys/soundcard.h>

#elif (defined (__MACH__) && defined (__APPLE__))
	#include <Carbon.h>
	#include <CoreAudio/AudioHardware.h>

#elif (defined (sun) && defined (unix))
	#include <fcntl.h>
	#include <sys/ioctl.h>
	#include <sys/audioio.h>

#elif (defined (_WIN32) || defined (WIN32))
	#include <windows.h>
	#include <mmsystem.h>
	#include <mmreg.h>

#endif

#include	<sndfile.h>

#define	SIGNED_SIZEOF(x)	((int) sizeof (x))
#define	BUFFER_LEN			(2048)

/*------------------------------------------------------------------------------
**	Linux/OSS functions for playing a sound.
*/

#if defined (__linux__)

static	int	linux_open_dsp_device (int channels, int srate) ;

static void
linux_play (int argc, char *argv [])
{	static short buffer [BUFFER_LEN] ;
	SNDFILE *sndfile ;
	SF_INFO sfinfo ;
	int		k, audio_device, readcount, subformat ;

	for (k = 1 ; k < argc ; k++)
	{	memset (&sfinfo, 0, sizeof (sfinfo)) ;

		printf ("Playing %s\n", argv [k]) ;
		if (! (sndfile = sf_open (argv [k], SFM_READ, &sfinfo)))
		{	puts (sf_strerror (NULL)) ;
			continue ;
			} ;

		if (sfinfo.channels < 1 || sfinfo.channels > 2)
		{	printf ("Error : channels = %d.\n", sfinfo.channels) ;
			continue ;
			} ;

		audio_device = linux_open_dsp_device (sfinfo.channels, sfinfo.samplerate) ;

		subformat = sfinfo.format & SF_FORMAT_SUBMASK ;

		if (subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE)
		{	static float float_buffer [BUFFER_LEN] ;
			double	scale ;
			int 	m ;

			sf_command (sndfile, SFC_CALC_SIGNAL_MAX, &scale, sizeof (scale)) ;
			if (scale < 1e-10)
				scale = 1.0 ;
			else
				scale = 32700.0 / scale ;

			while ((readcount = sf_read_float (sndfile, float_buffer, BUFFER_LEN)))
			{	for (m = 0 ; m < readcount ; m++)
					buffer [m] = scale * float_buffer [m] ;
				write (audio_device, buffer, readcount * sizeof (short)) ;
				} ;
			}
		else
		{	while ((readcount = sf_read_short (sndfile, buffer, BUFFER_LEN)))
				write (audio_device, buffer, readcount * sizeof (short)) ;
			} ;

		sf_close (sndfile) ;

		close (audio_device) ;
		} ;

	return ;
} /* linux_play */

static int
linux_open_dsp_device (int channels, int srate)
{	int fd, stereo, temp, error ;

	if ((fd = open ("/dev/dsp", O_WRONLY, 0)) == -1 &&
		(fd = open ("/dev/sound/dsp", O_WRONLY, 0)) == -1)
	{	perror("linux_open_dsp_device : open ") ;
		exit (1) ;
		} ;

	stereo = 0 ;
	if (ioctl (fd, SNDCTL_DSP_STEREO, &stereo) == -1)
	{ 	/* Fatal error */
		perror("linux_open_dsp_device : stereo ") ;
		exit (1);
		} ;

	if (ioctl (fd, SNDCTL_DSP_RESET, 0))
	{	perror ("linux_open_dsp_device : reset ") ;
		exit (1) ;
		} ;

	temp = 16 ;
	if ((error = ioctl (fd, SOUND_PCM_WRITE_BITS, &temp)) != 0)
	{	perror ("linux_open_dsp_device : bitwidth ") ;
		exit (1) ;
		} ;

	if ((error = ioctl (fd, SOUND_PCM_WRITE_CHANNELS, &channels)) != 0)
	{	perror ("linux_open_dsp_device : channels ") ;
		exit (1) ;
		} ;

	if ((error = ioctl (fd, SOUND_PCM_WRITE_RATE, &srate)) != 0)
	{	perror ("linux_open_dsp_device : sample rate ") ;
		exit (1) ;
		} ;

	if ((error = ioctl (fd, SNDCTL_DSP_SYNC, 0)) != 0)
	{	perror ("linux_open_dsp_device : sync ") ;
		exit (1) ;
		} ;

	return 	fd ;
} /* linux_open_dsp_device */

#endif /* __linux__ */

/*------------------------------------------------------------------------------
**	Mac OS X functions for playing a sound.
*/

#if (defined (__MACH__) && defined (__APPLE__)) /* MacOSX */

typedef struct
{	AudioStreamBasicDescription		format;

	UInt32 			buf_size;
	AudioDeviceID 	device;

	SNDFILE 		*sndfile ;
	SF_INFO 		sfinfo ;

	int 			done_playing ;
} MacOSXAudioData ;

#include <math.h>

static OSStatus
macosx_audio_out_callback (AudioDeviceID device, const AudioTimeStamp* current_time,
	const AudioBufferList* data_in, const AudioTimeStamp* time_in,
	AudioBufferList*	data_out, const AudioTimeStamp* time_out,
	void* client_data)
{	MacOSXAudioData	*audio_data ;
	int		size, sample_count, read_count ;
	float	*buffer ;

	/* Prevent compiler warnings. */
	device = device ;
	current_time = current_time ;
	data_in = data _in ;
	time_in = time_in ;
	time_out = time_out ;

	audio_data = (MacOSXAudioData*) client_data ;

	size = data_out->mBuffers[0].mDataByteSize ;
	sample_count = size / sizeof(float) ;

	buffer = (float*) data_out->mBuffers [0].mData ;

	read_count = sf_read_float (audio_data->sndfile, buffer, sample_count) ;
	
	if (read_count < sample_count)
	{	memset (&(buffer [read_count]), 0, (sample_count - read_count) * sizeof (float)) ;
		/* Tell the main application to terminate. */
		audio_data->done_playing = SF_TRUE ;
		} ;
	
	return noErr ;
} /* macosx_audio_out_callback */

static void
macosx_play (int argc, char *argv [])
{	MacOSXAudioData 	audio_data ;
	OSStatus	err ;
	UInt32		count, buffer_size ;
	int 		k ;

	audio_data.device = kAudioDeviceUnknown ;

	/*  get the default output device for the HAL */
	count = sizeof (AudioDeviceID) ;
	if ((err = AudioHardwareGetProperty (kAudioHardwarePropertyDefaultOutputDevice,
				&count, (void *) &(audio_data.device))) != noErr)
	{	printf ("AudioHardwareGetProperty failed.\n") ;
		return ;
		} ;

	/*  get the buffersize that the default device uses for IO */
	count = sizeof (UInt32) ;
	if ((err = AudioDeviceGetProperty (audio_data.device, 0, false, kAudioDevicePropertyBufferSize,
				&count, &buffer_size)) != noErr)
	{	printf ("AudioDeviceGetProperty (AudioDeviceGetProperty) failed.\n") ;
		return ;
		} ;

	/*  get a description of the data format used by the default device */
	count = sizeof (AudioStreamBasicDescription) ;
	if ((err = AudioDeviceGetProperty (audio_data.device, 0, false, kAudioDevicePropertyStreamFormat,
				&count, &(audio_data.format))) != noErr)
	{	printf ("AudioDeviceGetProperty (kAudioDevicePropertyStreamFormat) failed.\n") ;
		return ;
		} ;

	/* Base setup completed. Now play files. */
	for (k = 1 ; k < argc ; k++)
	{	printf ("Playing %s\n", argv [k]) ;
		if (! (audio_data.sndfile = sf_open (argv [k], SFM_READ, &(audio_data.sfinfo))))
		{	puts (sf_strerror (NULL)) ;
			continue ;
			} ;

		if (audio_data.sfinfo.channels < 1 || audio_data.sfinfo.channels > 2)
		{	printf ("Error : channels = %d.\n", audio_data.sfinfo.channels) ;
			continue ;
			} ;


		audio_data.format.mSampleRate = audio_data.sfinfo.samplerate ;
		audio_data.format.mChannelsPerFrame = audio_data.sfinfo.channels ;
		
		if ((err = AudioDeviceSetProperty (audio_data.device, NULL, 0, false, kAudioDevicePropertyStreamFormat,
					sizeof (AudioStreamBasicDescription), &(audio_data.format))) != noErr)
		{	printf ("AudioDeviceSetProperty (kAudioDevicePropertyStreamFormat) failed.\n") ;
			return ;
			} ;

		/*  we want linear pcm */
		if (audio_data.format.mFormatID != kAudioFormatLinearPCM)
			return ;

		/* Fire off the device. */
		if ((err = AudioDeviceAddIOProc (audio_data.device, macosx_audio_out_callback, 
				(void *) &audio_data)) != noErr)
		{	printf ("AudioDeviceAddIOProc failed.\n") ;
			return ;
			} ;

		err = AudioDeviceStart (audio_data.device, macosx_audio_out_callback) ;
		if	(err != noErr)
			return ;

		audio_data.done_playing = SF_FALSE ;

		while (audio_data.done_playing == SF_FALSE)
			usleep (10 * 1000) ; /* 10 000 milliseconds. */

		if ((err = AudioDeviceStop (audio_data.device, macosx_audio_out_callback)) != noErr)
		{	printf ("AudioDeviceStop failed.\n") ;
			return ;
			} ;
    
		err = AudioDeviceRemoveIOProc (audio_data.device, macosx_audio_out_callback) ;
        if (err != noErr)
		{	printf ("AudioDeviceRemoveIOProc failed.\n") ;
			return ;
			} ;

		sf_close (audio_data.sndfile) ;
		} ;

	return ;
} /* macosx_play */

#endif /* MacOSX */


/*------------------------------------------------------------------------------
**	Win32 functions for playing a sound.
**
**	This API sucks. Its needlessly complicated and is *WAY* too loose with
**	passing pointers arounf in integers and and using char* pointers to
**  point to data instead of short*. It plain sucks!
*/

#if (defined (_WIN32) || defined (WIN32))

#define	WIN32_BUFFER_LEN	(1<<15)

typedef struct
{	HWAVEOUT		hwave ;
	WAVEHDR			whdr [2] ;
	
	HANDLE			Event ;
	
	short			buffer [WIN32_BUFFER_LEN / sizeof (short)] ;
	int				current, bufferlen ;

	SNDFILE 		*sndfile ;
	SF_INFO 		sfinfo ;

	sf_count_t		remaining ;	
} Win32_Audio_Data ;


static void
win32_play_data (Win32_Audio_Data *audio_data)
{	int thisread, readcount ;
		
	readcount = (audio_data->remaining > audio_data->bufferlen) ? audio_data->bufferlen : (int) audio_data->remaining ;

	thisread = (int) sf_read_short (audio_data->sndfile, (short *) (audio_data->whdr [audio_data->current].lpData), readcount) ;

	audio_data->remaining -= thisread ;

	if (thisread > 0)
	{	/* Fix buffer length is only a partial block. */
		if (thisread * SIGNED_SIZEOF (short) < audio_data->bufferlen)
			audio_data->whdr [audio_data->current].dwBufferLength = thisread * sizeof (short) ;

		/* Queue the WAVEHDR */
		waveOutWrite (audio_data->hwave, (LPWAVEHDR) &(audio_data->whdr [audio_data->current]), sizeof (WAVEHDR)) ;
		}
	else
	{	/* Stop playback */
		waveOutPause (audio_data->hwave) ;

		SetEvent (audio_data->Event) ;
		} ;

	audio_data->current = (audio_data->current + 1) % 2 ;

} /* win32_play_data */

static DWORD CALLBACK
win32_audio_out_callback (HWAVEOUT hwave, UINT msg, DWORD data, DWORD param1, DWORD param2)
{	Win32_Audio_Data	*audio_data ;

	if (! data)
		return 1 ;

	/* 
	** I consider this technique of passing a pointer via an integer as 
	** fundamentally broken but thats the way microsoft has defined the 
	** interface.
	*/
	audio_data = (Win32_Audio_Data*) data ;
	
	if (msg == MM_WOM_DONE)
		win32_play_data (audio_data) ;
		
  return 0 ;
} /* win32_audio_out_callback */

static void
win32_play (int argc, char *argv [])
{	Win32_Audio_Data	audio_data ;
	
	WAVEFORMATEX wf ;
	int	k, error ;

	audio_data.sndfile = NULL ;
	audio_data.hwave = 0 ;

	for (k = 1 ; k < argc ; k++)
	{	printf ("Playing %s\n", argv [k]) ;

		if (! (audio_data.sndfile = sf_open (argv [k], SFM_READ, &(audio_data.sfinfo))))
		{	puts (sf_strerror (NULL)) ;
			continue ;
			} ;
			
		audio_data.remaining = audio_data.sfinfo.frames ;
		audio_data.current = 0 ;
		
		audio_data.Event = CreateEvent (0, FALSE, FALSE, 0) ;
			
		wf.nChannels = audio_data.sfinfo.channels ;
		wf.wFormatTag = WAVE_FORMAT_PCM ;
		wf.cbSize = 0 ;
		wf.wBitsPerSample = 16 ;

		wf.nSamplesPerSec = audio_data.sfinfo.samplerate ;
		
		wf.nBlockAlign = audio_data.sfinfo.channels * sizeof (short) ;
		
		wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec ;

		error = waveOutOpen (&(audio_data.hwave), WAVE_MAPPER, &wf, (DWORD) win32_audio_out_callback, 
							(DWORD) &audio_data, CALLBACK_FUNCTION) ;
		if (error)
		{	puts ("waveOutOpen failed.") ;
			audio_data.hwave = 0 ;
			continue ;
			} ;

		waveOutPause (audio_data.hwave) ;

		audio_data.whdr [0].lpData = (char*) audio_data.buffer ;
		audio_data.whdr [1].lpData = ((char*) audio_data.buffer) + sizeof (audio_data.buffer) / 2 ;
		
		audio_data.whdr [0].dwBufferLength = sizeof (audio_data.buffer) / 2 ;
		audio_data.whdr [1].dwBufferLength = sizeof (audio_data.buffer) / 2 ;
		
		audio_data.bufferlen = sizeof (audio_data.buffer) / 2 / sizeof (short) ; 		

		/* Prepare the WAVEHDRs */
		if ((error = waveOutPrepareHeader (audio_data.hwave, &(audio_data.whdr [0]), sizeof (WAVEHDR))))
		{	printf("waveOutPrepareHeader [0] failed : %08X\n", error) ;
			waveOutClose (audio_data.hwave) ;
			continue ;
			} ;

		if ((error = waveOutPrepareHeader (audio_data.hwave, &(audio_data.whdr [1]), sizeof (WAVEHDR))))
		{	printf("waveOutPrepareHeader [1] failed : %08X\n", error) ;
			waveOutUnprepareHeader (audio_data.hwave, &(audio_data.whdr [0]), sizeof(WAVEHDR)) ;
			waveOutClose (audio_data.hwave) ;
			continue ;
			} ;

		waveOutRestart (audio_data.hwave) ;

		/* Need to call this twice to queue up enough audio. */
		win32_play_data (&audio_data) ;
		win32_play_data (&audio_data) ;

		/* Wait for playback to finish. My callback notifies me when all wave data has been played */
		WaitForSingleObject (audio_data.Event, INFINITE);

		waveOutPause (audio_data.hwave) ;
		waveOutReset (audio_data.hwave);

		waveOutUnprepareHeader (audio_data.hwave, &(audio_data.whdr [0]), sizeof(WAVEHDR)) ;
		waveOutUnprepareHeader (audio_data.hwave, &(audio_data.whdr [1]), sizeof(WAVEHDR)) ;

		waveOutClose (audio_data.hwave) ;  
		audio_data.hwave = 0 ;

		sf_close (audio_data.sndfile) ;
		} ;

} /* win32_play */

#endif /* Win32 */

/*------------------------------------------------------------------------------
**	Solaris.
*/

#if (defined (sun) && defined (unix)) /* ie Solaris */

static void
solaris_play (int argc, char *argv [])
{	static short 	buffer [BUFFER_LEN] ;
	audio_info_t	audio_info ;
	SNDFILE			*sndfile ;
	SF_INFO			sfinfo ;
	unsigned long	delay_time ;
	long			k, start_count, output_count, write_count, read_count ;
	int				audio_fd, error, done ;
	
	for (k = 1 ; k < argc ; k++)
	{	printf ("Playing %s\n", argv [k]) ;
		if (! (sndfile = sf_open (argv [k], SFM_READ, &sfinfo)))
		{	puts (sf_strerror (NULL)) ;
			continue ;
			} ;

		if (sfinfo.channels < 1 || sfinfo.channels > 2)
		{	printf ("Error : channels = %d.\n", sfinfo.channels) ;
			continue ;
			} ;

		/* open the audio device - write only, non-blocking */
		if ((audio_fd = open ("/dev/audio", O_WRONLY | O_NONBLOCK)) < 0)
		{	perror ("open (/dev/audio) failed") ;
			return ;
			} ;
	
		/*	Retrive standard values. */
		AUDIO_INITINFO (&audio_info) ;
	
		audio_info.play.sample_rate = sfinfo.samplerate ;
		audio_info.play.channels = sfinfo.channels ;
		audio_info.play.precision = 16 ;
		audio_info.play.encoding = AUDIO_ENCODING_LINEAR ;
		audio_info.play.gain = AUDIO_MAX_GAIN ;
		audio_info.play.balance = AUDIO_MID_BALANCE ;
		
		if ((error = ioctl (audio_fd, AUDIO_SETINFO, &audio_info)))
		{	perror ("ioctl (AUDIO_SETINFO) failed") ;
			return ;
			} ;
	
		/* Delay time equal to 1/4 of a buffer in microseconds. */
		delay_time = (BUFFER_LEN * 1000000) / (audio_info.play.sample_rate * 4) ;
					
		done = 0 ;
		while (! done)
		{	read_count = sf_read_short (sndfile, buffer, BUFFER_LEN) ;
			if (read_count < BUFFER_LEN)
			{	memset (&(buffer [read_count]), 0, (BUFFER_LEN - read_count) * sizeof (short)) ;
				/* Tell the main application to terminate. */
				done = SF_TRUE ;
				} ;

			start_count = 0 ;
			output_count = BUFFER_LEN * sizeof (short) ;
	
			while (output_count > 0)
			{	/* write as much data as possible */
				write_count = write (audio_fd, &(buffer [start_count]), output_count) ;
				if (write_count > 0)
				{	output_count -= write_count ;
					start_count += write_count ;
					}
				else
				{	/*	Give the audio output time to catch up. */
					usleep (delay_time) ;
					} ;
				} ; /* while (outpur_count > 0) */
			} ; /* while (! done) */
	
		close (audio_fd) ;
		} ;

	return ;
} /* solaris_play */

#endif /* Solaris */

/*==============================================================================
**	Main function.
*/

int
main (int argc, char *argv [])
{
	if (argc < 2)
	{	
		printf ("\nUsage : %s <input sound file>\n\n", argv [0]) ;
#if (defined (_WIN32) || defined (WIN32))
		printf ("This is a Unix style command line application which\n"
				"should be run in a MSDOS box or Command Shell window.\n\n") ;
		printf ("Sleeping for 5 seconds before exiting.\n\n") ;

		/* This is the officially blessed by microsoft way but I can't get
		** it to link.
		**     Sleep (15) ;
		** Instead, use this:
		*/
		_sleep (5 * 1000) ;
#endif
		return 1 ;
		} ;

#if defined (__linux__)
	#if (defined (HAVE_ALSA) && HAVE_ALSA)
		if (access ("/proc/asound/cards", R_OK) == 0)
			alsa_play (argc, argv) ;
	#else
		linux_play (argc, argv) ;
	#endif
#elif (defined (__MACH__) && defined (__APPLE__))
	macosx_play (argc, argv) ;
#elif (defined (sun) && defined (unix))
	solaris_play (argc, argv) ;
#elif (defined (_WIN32) || defined (WIN32))
	win32_play (argc, argv) ;
#elif defined (__BEOS__)
	printf ("This program cannot be compiled on BeOS.\n") ;
	printf ("Instead, compile the file sfplay_beos.cpp.\n") ;
	return 1 ;
#else
	#warning "*** Playing sound not yet supported on this platform."
	#warning "*** Please feel free to submit a patch."
	printf ("Error : Playing sound not yet supported on this platform.\n") ;
	return 1 ;
#endif

	return 0 ;
} /* main */
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 8fc4110d-6cec-4e03-91df-0f384cabedac
*/
