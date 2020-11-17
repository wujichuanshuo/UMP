#include <iostream>
#include"umpcrawler.h"

int main(int argc, char** argv) {
    printf("begin!\n");
	std::string filePath = argv[1];
	std::string outpath = argv[2];
	//std::string filePath = "C:\\Users\\Administrator\\1.rawsnapshot";
	//std::string outpath = "C:\\Users\\Administrator\\Desktop\\123\\1.json";
	//std::cout << filePath;
	Windows tmp = Windows();
	if (tmp.LoadFromFile(filePath, outpath))
		printf("ok");
	else
		printf("false");
	return 0;
}
