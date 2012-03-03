/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
	File:		alac_encoder.h
*/

#ifndef ALAC_ENCODER_H
#define ALAC_ENCODER_H

#include <stdint.h>

#include "ALACAudioTypes.h"

typedef enum
{
	false = 0,
	true = 1
} bool ;

struct BitBuffer;

typedef struct alac_encoder
{
	// ALAC encoder parameters
	int16_t			mBitDepth;
	bool			mFastMode;

	// encoding state
	int16_t			mLastMixRes[kALACMaxChannels];

	// encoding buffers
	int32_t *		mMixBufferU;
	int32_t *		mMixBufferV;
	int32_t *		mPredictorU;
	int32_t *		mPredictorV;
	uint16_t *		mShiftBufferUV;

	uint8_t *		mWorkBuffer;

	// per-channel coefficients buffers
	int16_t			mCoefsU[kALACMaxChannels][kALACMaxSearches][kALACMaxCoefs];
	int16_t			mCoefsV[kALACMaxChannels][kALACMaxSearches][kALACMaxCoefs];

	// encoding statistics
	uint32_t		mTotalBytesGenerated;
	uint32_t		mAvgBitRate;
	uint32_t		mMaxFrameBytes;
	uint32_t		mFrameSize;
	uint32_t		mMaxOutputBytes;
	uint32_t		mNumChannels;
	uint32_t		mOutputSampleRate;
} alac_encoder ;

alac_encoder * alac_encoder_new (void);
void alac_encoder_delete (alac_encoder *);

int32_t	Encode(alac_encoder *p, AudioFormatDescription theInputFormat,
								   unsigned char * theReadBuffer, unsigned char * theWriteBuffer, int32_t * ioNumBytes);
int32_t	Finish(void);

static inline void
SetFastMode(alac_encoder * p, bool fast ) { p->mFastMode = fast; }

// this must be called *before* InitializeEncoder()
static inline void
SetFrameSize(alac_encoder *p, uint32_t frameSize ) { p->mFrameSize = frameSize; }

void	GetConfig(alac_encoder *p, ALACSpecificConfig * config );
uint32_t GetMagicCookieSize(uint32_t inNumChannels);
void	GetMagicCookie(alac_encoder *p, void * config, uint32_t * ioSize );

int32_t	InitializeEncoder(alac_encoder *p, AudioFormatDescription theOutputFormat);
void	GetSourceFormat(alac_encoder *p, const AudioFormatDescription * source, AudioFormatDescription * output );

int32_t	EncodeStereo(alac_encoder *p, struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t channelIndex, uint32_t numSamples );
int32_t	EncodeStereoFast(alac_encoder *p, struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t channelIndex, uint32_t numSamples );
int32_t	EncodeStereoEscape(alac_encoder *p, struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t numSamples );
int32_t	EncodeMono(alac_encoder *p, struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t channelIndex, uint32_t numSamples );

#endif
