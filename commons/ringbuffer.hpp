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

#include <string.h>
#include <iostream>

struct Ringbuffer_traits
{
    Ringbuffer_traits(unsigned int block, unsigned int size)
    : _block(block), _size(size), _reader(0), _writer(1) {};

    bool         traits_provide(      char *, const char *, unsigned int);
    unsigned int traits_consume(const char *,       char *, unsigned int);

    unsigned int traits_get(      char *, std::istream &, unsigned int);
    unsigned int traits_put(const char *, std::ostream &, unsigned int);

 protected:
    const unsigned int      _block;
    const unsigned int      _size;

    volatile unsigned int   _reader;
    volatile unsigned int   _writer;
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

        if (((reader - writer) == 1) || (reader == 0 && writer == _size))
            return false;

        unsigned int dest = writer - 1,
                     temp = writer + 1;

        _buffer[dest] = value;

        if (temp > _size) _writer = 1;
        else              _writer = temp;

        return true;
    }

    bool consume(T & value)
    {
        unsigned int reader = _reader,
                     writer = _writer;

        if ((writer - reader) == 1)
            return false;

        value = _buffer[reader];

        unsigned int temp = reader + 1;

        if (temp == _size) _reader = 0;
        else               _reader = temp;

        return true;
    }

    /* writes everything or nothing */
    inline bool provide(const T * value, unsigned int amount)
    {
        return traits_provide((char *)_buffer, (const char *) value, amount);
    }

    /* returns the number of itens that have been read */
    inline unsigned int consume(T * value, unsigned int amount)
    {
        return traits_consume((const char *)_buffer, (char *) value, amount);
    }

	/***** PARCIAL BUFFER FUNCTIONS (TWO-STAGE) *****/

	T & producer_start(void)
	{
        unsigned int reader = _reader,
                     writer = _writer;

        if (((reader - writer) == 1) || (reader == 0 && writer == _size))
            throw BufferFull();

        return _buffer[writer-1];
	}

	void producer_commit(void)
	{
        unsigned int temp = _writer + 1;

        if (temp > _size) _writer = 1;
        else              _writer = temp;
	}

	T & consumer_start(void)
	{
        unsigned int reader = _reader,
                     writer = _writer;

        if ((writer - reader) == 1)
            throw BufferEmpty();

        return _buffer[reader];
	}

	void consumer_commit(void)
	{
        unsigned int temp = _reader + 1;

        if (temp == _size) _reader = 0;
        else               _reader = temp;
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
