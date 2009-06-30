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
    Ringbuffer(unsigned int size)
    : ringbuffer_traits(sizeof(T), size)
    {
        _buffer = new T[_size];
    };

    Ringbuffer(unsigned int size, T * buffer)
    : ringbuffer_traits(sizeof(T), size)
    {
        _buffer = buffer;
    };

    ~Ringbuffer()
    {
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
    T * _buffer;
};
