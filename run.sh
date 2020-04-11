g++ -std=c++0x -mavx -march=native ./maincpp.cpp 

sleep 1

FILE=./a.exe
if test -f "$FILE"; then
	start ./a.exe
fi

sleep 2

./clean.sh
