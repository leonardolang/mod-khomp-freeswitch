/*  
    KHOMP generic endpoint/channel library.
    Copyright (C) 2007-2009 Khomp Ind. & Com.  
  
  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

  Alternatively, the contents of this file may be used under the terms of the
  "GNU Lesser General Public License 2.1" license (the “LGPL" License), in which
  case the provisions of "LGPL License" are applicable instead of those above.

  If you wish to allow use of your version of this file only under the terms of
  the LGPL License and not to allow others to use your version of this file under
  the MPL, indicate your decision by deleting the provisions above and replace them
  with the notice and other provisions required by the LGPL License. If you do not
  delete the provisions above, a recipient may use your version of this file under
  either the MPL or the LGPL License.

  The LGPL header follows below:

    This library is free software; you can redistribute it and/or  
    modify it under the terms of the GNU Lesser General Public  
    License as published by the Free Software Foundation; either  
    version 2.1 of the License, or (at your option) any later version.  
  
    This library is distributed in the hope that it will be useful,  
    but WITHOUT ANY WARRANTY; without even the implied warranty of  
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
    Lesser General Public License for more details.  
  
    You should have received a copy of the GNU Lesser General Public License
    along with this library; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
  
*/

/* WARNING: This is a generic ringbuffer abstraction, which works for single-sized elements,
            partial elements, single/multi-elements read/writes. It is not wise to mix some
            functions (partial element write / full element write), since it was not designed
            with this use in mind.

            Also, it works only for single-reader + single-writer, since it does not depends
            on external mutex functions.
 */

#include <string.h>

#include <cmath>
#include <iostream>

struct Ringbuffer_traits
{
    Ringbuffer_traits(unsigned int block, unsigned int size)
    : _block(block), _size(size), _reader(0), _writer(1),
      _reader_partial(0), _writer_partial(1)
    {
//        fprintf(stderr, "===> creating buffer of block %d, size %d\n", _block, _size);
    };

    bool         traits_provide(      char *, const char *, unsigned int);
    unsigned int traits_consume(const char *,       char *, unsigned int);

    bool         traits_provide_partial(      char *, const char *, unsigned int);
    unsigned int traits_consume_partial(const char *,       char *, unsigned int);

    unsigned int traits_get(      char *, std::istream &, unsigned int);
    unsigned int traits_put(const char *, std::ostream &, unsigned int);

 protected:
    const unsigned int      _block;
    const unsigned int      _size;

    volatile unsigned int   _reader;
    volatile unsigned int   _writer;

    volatile unsigned int   _reader_partial;
    volatile unsigned int   _writer_partial;
};

template <typename T>
struct Ringbuffer: protected Ringbuffer_traits
{
    struct BufferFull  {};
    struct BufferEmpty {};

    Ringbuffer(unsigned int size)
    : Ringbuffer_traits(sizeof(T), size)
    {
        _buffer = new T[_size];
        _malloc = true;
    };

    Ringbuffer(unsigned int size, T * buffer)
    : Ringbuffer_traits(sizeof(T), size)
    {
        _buffer = buffer;
        _malloc = false;
    };

    ~Ringbuffer()
    {
        if (_malloc)
          delete[] _buffer;
    }

    /***** BUFFER FUNCTIONS *****/

    bool provide(const T & value)
    {
        unsigned int reader = _reader,
                     writer = _writer;

//		fprintf(stderr, "%p> provide %d/%d!\n", this, reader, writer);

        if (((reader - writer) == 1) || (reader == 0 && writer == _size))
            return false;

        unsigned int dest = writer - 1,
                     temp = writer + 1;

        _buffer[dest] = value;

        if (temp > _size) writer = 1;
        else              writer = temp;

        _writer         = writer;
        _writer_partial = writer * sizeof(T);

//		fprintf(stderr, "%p> write: %d/%d [%d/%d]\n", this, _reader, _writer, _reader_partial, _writer_partial);

        return true;
    }

    bool consume(T & value)
    {
        unsigned int reader = _reader,
                     writer = _writer;

//		fprintf(stderr, "%p> consume %d/%d!\n", this, reader, writer);

        if ((writer - reader) == 1)
            return false;

        value = _buffer[reader];

        unsigned int temp = reader + 1;

        if (temp == _size) reader = 0;
        else               reader = temp;

        _reader         = reader;
        _reader_partial = reader * sizeof(T);

//		fprintf(stderr, "%p> read: %d/%d [%d/%d]\n", this, _reader, _writer, _reader_partial, _writer_partial);

        return true;
    }

    /* writes everything or nothing */
    inline bool provide(const T * value, unsigned int amount)
    {
        return traits_provide((char *)_buffer, (const char *) value, amount);
    }

    /* returns the number of items that have been read */
    inline unsigned int consume(T * value, unsigned int amount)
    {
        return traits_consume((const char *)_buffer, (char *) value, amount);
    }

	/***** PARCIAL BUFFER FUNCTIONS (TWO-STAGE) *****/

	T & provider_start(void)
	{
        unsigned int reader = _reader,
                     writer = _writer;

//		fprintf(stderr, "%p> provider start %d/%d!\n", this, reader, writer);

        if (((reader - writer) == 1) || (reader == 0 && writer == _size))
            throw BufferFull();

        return _buffer[writer-1];
	}

	void provider_commit(void)
	{
        unsigned int temp = _writer + 1;

//		fprintf(stderr, "%p> provider commit %d!\n", this, temp);

        if (temp > _size)
            temp = 1;

        _writer         = temp;
        _writer_partial = temp * sizeof(T);

//		fprintf(stderr, "%p> write: %d/%d [%d/%d]\n", this, _reader, _writer, _reader_partial, _writer_partial);
	}

	T & consumer_start(void)
	{
        unsigned int reader = _reader,
                     writer = _writer;

//		fprintf(stderr, "%p> consumer start %d/%d!\n", this, reader, writer);

        if ((writer - reader) == 1)
            throw BufferEmpty();

        return _buffer[reader];
	}

	void consumer_commit(void)
	{
        unsigned int temp = _reader + 1;

//		fprintf(stderr, "%p> consumer commit %d!\n", this, temp);

        if (temp == _size)
            temp = 0;

        _reader         = temp;
        _reader_partial = temp * sizeof(T);

//		fprintf(stderr, "%p> read: %d/%d [%d/%d]\n", this, _reader, _writer, _reader_partial, _writer_partial);
	}

    /* writes everything or nothing, but works on bytes (may write incomplete elements) */
	/* WARNING: do not mix this with full element provider */
    inline bool provider_partial(const char *buffer, unsigned int amount)
    {
        return traits_provide_partial((char *)_buffer, buffer, amount);
    }

    /* returns the number of bytes that have been read (may read incomplete elements) */
	/* WARNING: do not mix this with full element consumer */
    inline unsigned int consumer_partial(char *buffer, unsigned int amount)
    {
        return traits_consume_partial((const char *)_buffer, buffer, amount);
    }

    /***** IO FUNCTIONS *****/

    /* returns the number of items written to from buffer to stream */
    inline unsigned int put(std::ostream &fd, unsigned int amount)
    {
        return traits_put((char *)_buffer, fd, amount);
    }

    /* returns number of items read from stream to buffer */
    inline unsigned int get(std::istream &fd, unsigned int amount)
    {
        return traits_get((const char *)_buffer, fd, amount);
    }

    void clear()
    {
        _reader = 0;
        _writer = 1;
    }

 protected:
    T *  _buffer;
    bool _malloc;
};
