#include "umpcrawler.h"
#include<iostream>


using std::string;
class Windows
{
public:
	Windows();
	~Windows();
	int LoadFromFile(std::string filepath);
private:
	void Windows::CleanWorkSpace();
	void Windows::RemoteDataReceived();
	void Windows::ShowSnapshot(CrawledMemorySnapshot* crawled);
	StartAppProcess *startAppProcess_;
};

