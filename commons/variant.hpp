#ifndef _VARIANT_H_
#define _VARIANT_H_

struct VariantBaseType
{
    VariantBaseType() {}

    virtual ~VariantBaseType() {}

    virtual int which() = 0;
};

template < typename BaseType >
struct Variant
{
    struct InvalidType {};

    Variant(BaseType * value) : _value(value)
    {
       _count = new int(1);
    };

    Variant(const Variant & v) : _value(v._value), _count(v._count)
    {
       ++(*_count);
    }

    ~Variant()
    {
        --(*_count);

        if (_value && !(*_count))
        {
            delete _value;
            delete _count;
        }
    };

    template < typename ReturnType >
    ReturnType & get(void)
    {
        try
        {
            ReturnType & ret = dynamic_cast < ReturnType & > (*_value);
            return ret;
        }
        catch (std::bad_cast & e)
        {
            throw InvalidType();
        }

    };

    int which()
    {
        return _value->which();
    }

 protected:
    BaseType * _value;
    int      * _count;
};

#endif /* _VARIANT_H_ */

