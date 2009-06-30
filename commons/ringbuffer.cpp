#include <ringbuffer.hpp>

/********** BUFFER FUNCTIONS **********/

/* writes everything or nothing */
bool Ringbuffer_traits::traits_provide(char * buffer, const char * value, unsigned int amount)
{
    unsigned int reader = _reader, /* avoid using different values */
                 writer = _writer;

    unsigned int dest = writer - 1;

    bool reader_less = reader < writer;

    /* should we go around the buffer for writing? */
    if (reader_less && ((writer + amount) > _size))
    {
        /* Documentation of the formula used in the 'if' below.
         *
         * [0|1|2|3|4|5|6|7] => size=8
         *      |   |
         *  reader  |
         *         writer
         *
         * => writer has places [5,6,7,0,1] to write (5 places).
         *
         * =>  8 - (4-2+1) = 8 - (2+1) = 8 - 3 = 5
         *
         * > writer goes 1 up, amount goes 1 down.
         * > reader goes 1 up, amount goes 1 up.
         * > size goes 1 down, amount goes 1 down.
         *
         */

        if ((_size - (writer - reader + 1)) <= amount)
            return false;

        unsigned int wr1 = _size - writer + 1; /* writer is already 1 position after */
        unsigned int wr2 = amount - wr1;
        
//        fprintf(stderr, "%p> partial write: (%d/%d) %d/%d [%d/%d]\n", this, wr1, wr2, amount, _size, reader, writer);
            
        /* two partial writes (one at the end, another at the beginning) */
        memcpy((void *) &(buffer[dest]), (const void *)  (value),      _block * wr1);
        memcpy((void *)  (buffer),       (const void *) &(value[wr1]), _block * wr2);

        _writer = ((dest + amount) % _size) + 1;
    }
    else
    {
        if (!reader_less && ((reader - writer) <= amount))
            return false;

//        fprintf(stderr, "%p> full write: %d/%d [%d/%d]\n", this, amount, _size, reader, writer);

        /* we are talking about buffers here, man! */
        memcpy((void *) &(buffer[dest]), (const void *) value, _block * amount);

        _writer = writer + amount;
    }

    return true;
}

/* returns the number of itens that have been read */
unsigned int Ringbuffer_traits::traits_consume(const char * buffer, char * value, unsigned int amount)
{
    unsigned int reader = _reader,
                 writer = _writer; /* avoid using different values */

    bool writer_less = writer < reader;

    /* should we go around the buffer for reading? */
    if (writer_less && (reader + amount >= _size))
    {
        unsigned int total = std::min(_size - (reader - writer + 1), amount);

        if (total == 0)
            return 0;

        unsigned int rd1 = _size - reader;
        unsigned int rd2 = total - rd1;

//        fprintf(stderr, "%p> partial read: (%d/%d) %d/%d [%d/%d]\n", this, rd1, rd2, total, _size, reader, writer);
            
        /* two partial consumes (one at the end, another at the beginning) */
        memcpy((void *)  (value),      (const void *) &(buffer[reader]), _block * rd1);
        memcpy((void *) &(value[rd1]), (const void *)  (buffer),         _block * rd2);

        /* jump the reader forward */
        _reader = (reader + total) % _size;
            
        return total;
    }
    else
    {
        unsigned int total = std::min((!writer_less ? writer - (reader + 1) : amount), amount);
            
        if (total == 0)
            return 0;

//        fprintf(stderr, "%p> full read: %d/%d [%d/%d]\n", this, total, _size, reader, writer);

        /* we are talking about buffers here, man! */
        memcpy((void *) value, (const void *) &(buffer[_reader]), _block * total);

        /* jump the reader forward */
        _reader = (reader + total);
            
        return total;
    }
}

/********** IO FUNCTIONS **********/

/* returns the number of items written to from buffer to stream */
unsigned int Ringbuffer_traits::traits_put(const char * buffer, std::ostream &fd, unsigned int amount)
{
    unsigned int reader = _reader,
                 writer = _writer; /* avoid using different values */

    bool writer_less = writer < reader;
        
    /* should we go around the buffer for reading? */
    if (writer_less && (reader + amount >= _size))
    {
        unsigned int total = std::min(_size - (reader - writer + 1), amount);

        if (total == 0)
            return 0;

        unsigned int rd1 = _size - reader;
        unsigned int rd2 = total - rd1;

//        fprintf(stderr, "%p> partial read: (%d/%d) %d/%d [%d/%d]\n", this, rd1, rd2, total, _size, reader, writer);
            
        /* two partial consumes (one at the end, another at the beginning) */
        fd.write((const char *) &(buffer[reader]), _block * rd1);
        fd.write((const char *)  (buffer),         _block * rd2);

        /* jump the reader forward */
        _reader = (reader + total) % _size;
            
        return total;
    }
    else
    {
        unsigned int total = std::min((!writer_less ? writer - (reader + 1) : amount), amount);

        if (total == 0)
            return 0;

//        fprintf(stderr, "%p> full read: %d/%d [%d/%d]\n", this, total, _size, reader, writer);

        /* we are talking about buffers here, man! */
        fd.write((const char *) &(buffer[_reader]), _block * total);

        /* jump the reader forward */
        _reader = (reader + total);
        
        return total;
    }
}

/* returns number of items read from stream to buffer */
unsigned int Ringbuffer_traits::traits_get(char * buffer, std::istream &fd, unsigned int amount)
{
    unsigned int reader = _reader, /* avoid using different values */
                 writer = _writer;

    unsigned int dest = writer - 1;

    bool reader_less = reader < writer;

    /* should we go around the buffer for writing? */
    if (reader_less && ((writer + amount) > _size))
    {
        /* Documentation of the formula used in the 'if' below.
         *
         * [0|1|2|3|4|5|6|7] => size=8
         *      |   |
         *  reader  |
         *         writer
         *
         * => writer has places [5,6,7,0,1] to write (5 places).
         *
         * =>  8 - (4-2+1) = 8 - (2+1) = 8 - 3 = 5
         *
         * > writer goes 1 up, amount goes 1 down.
         * > reader goes 1 up, amount goes 1 up.
         * > size goes 1 down, amount goes 1 down.
         *
         */

        if ((_size - (writer - reader + 1)) <= amount)
            return false;

        unsigned int wr1 = _size - writer + 1; /* writer is already 1 position after */
        unsigned int wr2 = amount - wr1;

//        fprintf(stderr, "%p> partial write: (%d/%d) %d/%d [%d/%d]\n", this, wr1, wr2, amount, _size, reader, writer);

        unsigned int char_amount = 0;

        /* one partial write on the buffer (at the end) */
        fd.read((char *) &(buffer[dest]), _block * wr1);
        char_amount += fd.gcount();

        if (fd.gcount() == (int)(_block * wr1))
        {
            /* another partial write on the buffer (at the beginning) */
            fd.read((char *) (buffer), _block * wr2);
            char_amount += fd.gcount();
        }

        unsigned int real_amount = char_amount / _block;

        _writer = ((dest + real_amount) % _size) + 1;
            
        return real_amount;
    }
    else
    {
        if (!reader_less && ((reader - writer) <= amount))
            return false;

//        fprintf(stderr, "%p> full write: %d/%d [%d/%d]\n", this, amount, _size, reader, writer);

        /* we are talking about buffers here, man! */
        fd.read((char *) &(buffer[dest]), _block * amount);

        unsigned int real_amount = fd.gcount() / _block;

        _writer = writer + real_amount;
        
        return real_amount;
    }
}
