cc=g++
exe=HelloSocket
obj=HelloSocket.o Utils.o
headerPath="/Volumes/[C] Windows 10/StudyServer/HelloSocket/CommonLib"

$(exe):$(obj)
	$(cc) -o $(exe) $(obj)
Utils.o:../CommonLib/Utils.cpp
	$(cc) -c ../CommonLib/Utils.cpp -I $(headerPath)
HelloSocket.o:HelloSocket.cpp
	$(cc) -c HelloSocket.cpp  -I $(headerPath)
clean:
	rm -rf ./obj/*.o HelloSocket