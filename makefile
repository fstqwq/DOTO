all: main.out

main.out: makefile main.o logic.h logic.o const.h playerAI.h playerAI.cpp  jsoncpp/json/json-forwards.h jsoncpp/json/json.h jsoncpp/jsoncpp.cpp geometry.o geometry.h
ifeq ($(OS),Windows_NT)
	g++ main.o logic.o playerAI.cpp jsoncpp/jsoncpp.cpp geometry.o -o main.exe -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -lwsock32 -std=c++11
else
	g++ main.o logic.o playerAI.cpp jsoncpp/jsoncpp.cpp geometry.o -o main.out -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -pthread -Wshadow -O1
endif

logic.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h logic.cpp 
	g++ -c logic.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -O1

geometry.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h geometry.cpp
	g++ -c geometry.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -O1

gamemap.h: makefile map.info preset.h
	g++ precal.cpp -o precal.out -std=c++11 -O1
	./precal.out

main.o: makefile gamemap.h const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h main.cpp
	g++ -c main.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -O1

.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	-del -r *.exe *.o gamemap.h
else
	-rm -r *.out *.o gamemap.h
endif
