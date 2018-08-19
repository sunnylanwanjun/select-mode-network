cc=g++
exe=EasyTcpServer
obj=EasyTcpServer.o Utils.o
headerPath="/Volumes/[C] Windows 10/StudyServer/HelloSocket/CommonLib"

$(exe):$(obj)
	$(cc) -o $(exe) $(obj)
Utils.o:../CommonLib/Utils.cpp
	$(cc) -c -std=c++11 -pthread  ../CommonLib/Utils.cpp -I $(headerPath)
EasyTcpServer.o:EasyTcpServer.cpp
	$(cc) -c -std=c++11 -pthread  EasyTcpServer.cpp  -I $(headerPath)
clean:
	rm -rf ./obj/*.o EasyTcpServer