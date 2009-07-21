all:
	g++ -I.. -o rbtest  ../ringbuffer.cpp   rbtest.cpp 
	g++ -I.. -o fmttest ../format.cpp      fmttest.cpp 
	g++ -I.. -o funtest functiontest.cpp 
	g++ -o verbosetest \
		-I .. -I /home/lang/SVN/SOFTPBX/chan_khomp/2.4/trunk/include/ \
		../verbose.cpp ../format.cpp ../strings.cpp verbosetest.cpp
	g++ -o monitortest -I .. -I /home/lang/SVN/SOFTPBX/chan_khomp/2.4/trunk/include/ \
		../format.cpp monitortest.cpp -lk3l
