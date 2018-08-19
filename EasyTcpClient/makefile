cc=g++
exe=EasyTcpClient
obj=EasyTcpClient.o Utils.o
headerPath="/Volumes/[C] Windows 10/StudyServer/HelloSocket/CommonLib"

$(exe):$(obj)
	$(cc) -o $(exe) $(obj)
Utils.o:../CommonLib/Utils.cpp
	$(cc) -c -std=c++11 -pthread ../CommonLib/Utils.cpp -I $(headerPath)
EasyTcpClient.o:EasyTcpClient.cpp
	$(cc) -c -std=c++11 -pthread EasyTcpClient.cpp  -I $(headerPath)
clean:
	rm -rf ./obj/*.o EasyTcpClient