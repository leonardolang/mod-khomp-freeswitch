/*  
    KHOMP endpoint driver for FreeSWITCH library.
    Copyright (C) 2007-2009 Khomp Ind. & Com.  
  
  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.
  
*/

#include "frame.h"

#define ALLOC(T,s) ((T*)calloc(1,s))

/* Internal frame manager structure. */
FrameStorage::FrameStorage(switch_codec_t * codec, int packet_size)
:  _frames(ALLOC(switch_frame_t, frame_count * sizeof(switch_frame_t))),
   _buffer(ALLOC(          char, audio_count * packet_size)),
   _index(0)
{
	for (unsigned int i = 0; i < frame_count; i++)
	{
		_frames[i].codec      = codec;
		_frames[i].source     = "mod_khomp";

		_frames[i].packet     = 0;
		_frames[i].packetlen  = 0;
		_frames[i].extra_data = 0;

		_frames[i].data       = (char *)0;
		_frames[i].datalen    = packet_size;
		_frames[i].buflen     = packet_size;

		_frames[i].samples    = packet_size; // packet_duration * 8
		_frames[i].rate       = 8000;
		_frames[i].payload    = 0;

		_frames[i].timestamp  = 0u;

		_frames[i].seq        = 0u;
		_frames[i].ssrc       = 0u;
		_frames[i].m          = SWITCH_FALSE;
		_frames[i].flags      = SFF_NONE;
	}

    _cng_frame.codec      = codec;
    _cng_frame.source     = "mod_khomp";
	_cng_frame.packet     = 0;
	_cng_frame.packetlen  = 0;
	_cng_frame.extra_data = 0;

	_cng_frame.data       = (void*)"A";
	_cng_frame.datalen    = 2;
	_cng_frame.buflen     = 2;

	_cng_frame.samples    = packet_size;
	_cng_frame.rate       = 8000;
	_cng_frame.payload    = 0;

	_cng_frame.timestamp  = 0u;

	_cng_frame.seq        = 0u;
	_cng_frame.ssrc       = 0u;
	_cng_frame.m          = SWITCH_FALSE;
	_cng_frame.flags      = SFF_CNG;

//	if (mlock(&_frames, frames_size) < 0)
//	{
//		DBG(CONF, F("Unable to lock ast_frame buffer memory in RAM: %s") % strerror(errno));
//		std::cerr << "chan_khomp: Unable to lock ast_frame buffer memory in RAM: "
//			<< strerror(errno) << ". "
//				<< "This is not a catastrophic failure, but may cause unpredictable "
//				<< "audio delay under extreme load conditions." << std::endl;
//	}
//
//	if (mlock(&_buffer, buffer_size) < 0)
//	{
//		DBG(CONF, F("Unable to lock audio buffer memory in RAM: %s") % strerror(errno));
//		std::cerr << "chan_khomp: Unable to lock temporary audio buffer memory in RAM: "
//				<< strerror(errno) << ". "
//					<< "This is not a catastrophic failure, but may cause unpredictable "
//					<< "audio delay under extreme load conditions." << std::endl;
//	}
};

FrameStorage::~FrameStorage()
{
    free(_frames);
    free(_buffer);
}

