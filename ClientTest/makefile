cc=g++
exe=ClientTest
obj=Utils.o ClientSocket.o ConfigHelp.o \
MemoryPool.o SelectClient_Thread.o SelectClient.o \
SelectServer_Thread.o SelectServer.o VarMemPool.o \
ClientTest.o Client01.o Client02.o Client03.o Client04.o
headerPath="/Volumes/[C] Windows 10/StudyServer/HelloSocket/CommonLib"

$(exe):$(obj)
	$(cc) -o $(exe) $(obj)
ClientTest.o:ClientTest.cpp
	$(cc) -c -std=c++11 -pthread ClientTest.cpp  -I $(headerPath)
Client01.o:Client01.cpp
	$(cc) -c -std=c++11 -pthread Client01.cpp  -I $(headerPath)
Client02.o:Client02.cpp
	$(cc) -c -std=c++11 -pthread Client02.cpp  -I $(headerPath)
Client03.o:Client03.cpp
	$(cc) -c -std=c++11 -pthread Client03.cpp  -I $(headerPath)
Client04.o:Client04.cpp
	$(cc) -c -std=c++11 -pthread Client04.cpp  -I $(headerPath)
Utils.o:../CommonLib/Utils.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/Utils.cpp -I $(headerPath)
ClientSocket.o:../CommonLib/ClientSocket.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/ClientSocket.cpp -I $(headerPath)
ConfigHelp.o:../CommonLib/ConfigHelp.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/ConfigHelp.cpp -I $(headerPath)
MemoryPool.o:../CommonLib/MemoryPool.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/MemoryPool.cpp -I $(headerPath)
SelectClient_Thread.o:../CommonLib/SelectClient_Thread.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/SelectClient_Thread.cpp -I $(headerPath)
SelectClient.o:../CommonLib/SelectClient.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/SelectClient.cpp -I $(headerPath)
SelectServer_Thread.o:../CommonLib/SelectServer_Thread.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/SelectServer_Thread.cpp -I $(headerPath)
SelectServer.o:../CommonLib/SelectServer.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/SelectServer.cpp -I $(headerPath)
VarMemPool.o:../CommonLib/VarMemPool.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/VarMemPool.cpp -I $(headerPath)
clean:
	rm -rf ./obj/*.o ClientTest