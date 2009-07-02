#include <ringbuffer.hpp>

typedef char mypacket[10];

typedef Ringbuffer < mypacket > mybuffer;

int main ()
{
	mypacket * packbuf = new mypacket[4];

	mybuffer buf(4, packbuf);

	// TEST 1
	if (!buf.provider_partial("testetesteteste", 15))
		std::cerr << "provider 1 FAILED!" << std::endl;

	if (!buf.provider_partial("abcdefghijklmno", 15))
		std::cerr << "provider 2 FAILED!" << std::endl;

	// TEST 2
	for (unsigned int n = 0; n < 3; n++)
	{
		try
		{
			mypacket & elt = buf.consumer_start();

			std::cerr << "consumed '" << n << "' OK! consumed: ";

			for (unsigned int i = 0; i < 10; i++)
				std::cerr << elt[i];

			std::cerr << std::endl;

			buf.consumer_commit();
		}
		catch (mybuffer::BufferEmpty & e)
		{
			std::cerr << "consume '" << n << "' FAILED!" << std::endl;
		}
	}


}
