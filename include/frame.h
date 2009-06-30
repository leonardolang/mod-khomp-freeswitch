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

#ifndef _FRAME_HPP_
#define _FRAME_HPP_

#include <stdlib.h>
#include <sys/mman.h>

#include <ringbuffer.hpp>

#include "globals.h"

/* Internal frame array structure. */
struct FrameArray
{
	static const unsigned int frame_count;

    FrameArray(switch_codec_t *);
    ~FrameArray();

    // may throw Ringbuffer::BufferEmpty
	switch_frame_t * pick(void)
	{
        switch_frame & f = _audio.consumer_start();

        /* advance now */
        _audio.consumer_commit();

        return &f;
	}

    // may throw Ringbuffer::BufferFull
	bool give(const char * buf, unsigned int size)
	{
        switch_frame_t & f = _audio.producer_start();

        memcpy((char *)f.data, buf, size);

        _audio.producer_commit();
    }

    switch_frame_t * cng(void)
    {
        return &_cng_frame;
    }

    void clear()
    {
        _audio.clear();
    }

 protected:
    switch_frame_t                 _cng_frame;
    switch_frame_t               * _frames;
    char                         * _buffer;
    Ringbuffer < switch_frame_t >  _audio;
};

#endif /* _FRAME_HPP_ */
