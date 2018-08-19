cc=g++
exe=ServerTest
obj=Utils.o ClientSocket.o ConfigHelp.o \
MemoryPool.o SelectClient_Thread.o SelectClient.o \
SelectServer_Thread.o SelectServer.o VarMemPool.o \
ServerTest.o Server01.o Server02.o
headerPath="/Volumes/[C] Windows 10/StudyServer/HelloSocket/CommonLib"

$(exe):$(obj)
	$(cc) -o $(exe) $(obj)
ServerTest.o:ServerTest.cpp
	$(cc) -c -std=c++11 -pthread ServerTest.cpp  -I $(headerPath)
Server01.o:Server01.cpp
	$(cc) -c -std=c++11 -pthread Server01.cpp  -I $(headerPath)
Server02.o:Server02.cpp
	$(cc) -c -std=c++11 -pthread Server02.cpp  -I $(headerPath)
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
	rm -rf ./obj/*.o ServerTest