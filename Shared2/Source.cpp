// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <Windows.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <ctime>
#include "ComLib.h"
#pragma comment(lib, "user32.lib")
using namespace std;

#define MEGABYTE 1024

bool isDigits(const string& str);
void gen_random(char* s, const int len);
int randomizeSize(const size_t buffsize);


int main(int argc, char* argv[])
{
	if (argc == 6)
	{
		//Random seeding
		srand(int(time(0))); 
		
		//Argument handling
		string firstArg = argv[1];				// Producer || Conumer
		short delay = stoi(argv[2]);			// Delay
		const size_t buffsize = stoi(argv[3]);	// Memory Size in megabytes
		int nrOfMessages = stoi(argv[4]);		// Number of messages
		string secondArg = argv[5];				// Random or set mesgSize
		
		const string secret = "ComLibSecret";
		char* msgPtr = nullptr;


		cout << "number of messages = " << nrOfMessages << endl;

		char* randomMsg;

		randomMsg = new char[((buffsize * MEGABYTE) / 2) - 2 * sizeof(int)]; // message size can at most be the half of total memory size. 

		msgPtr = randomMsg;
		int messageSize;

		if (firstArg.compare("producer") == 0) //If the program should act as producer, create filemap
		{
			ComLib comLib(secret, buffsize * MEGABYTE, ComLib::PRODUCER);

			for (int i = 0; i < nrOfMessages; i++)
			{
				Sleep(delay);

				if (secondArg.compare("random") == 0)
				{
					messageSize = randomizeSize(buffsize);
					gen_random(randomMsg, messageSize);
					

					comLib.send(randomMsg, messageSize);

				} 
				else if (isDigits(secondArg))
				{
					int fixedSize = *argv[5];
					gen_random(randomMsg, fixedSize);

					comLib.send(randomMsg, fixedSize);
				}

				//Print the message and the message number, Remove before python test 
				std::cout << randomMsg << std::endl;
				std::cout << "msg number: [" << i + 1 << "]" << std::endl;
			}
	
		}
		else if (firstArg.compare("consumer") == 0)
		{
			size_t readMsgSize = (buffsize * MEGABYTE / 2);
			ComLib comLib(secret, buffsize * MEGABYTE, ComLib::CONSUMER);
			for(int i = 0; i < nrOfMessages; i++)
			{ 
				Sleep(delay);
				comLib.recv(msgPtr, (size_t&)readMsgSize);
				std::cout << "msg number: [" << i + 1 << "]"<< std::endl;
			}
		}
		else
			std::cout << "ERROR: FIRST INPUT IS NOT \"producer\" OR \"consumer\"" << std::endl;;


	}
	else
	{
		if (argc > 6)
			cout << "Too many arguments" << endl;
		if (argc < 6)
			cout << "Too few arguments" << endl;
	}
	//Remove this before python script
	system("pause");
	return 0;
}

bool isDigits(const string& str)
{
	return str.find_first_not_of("0123456789") == string::npos;
}

// Random character generation of "len" bytes.
// the pointer s must point to a big enough buffer to hold "len" bytes.
void gen_random(char* s, const int len)
{
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (auto i = 0; i < len; i++)
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	s[len - 1] = 0;
}

int randomizeSize(const size_t buffsize)
{
	int	size = rand() % (buffsize * MEGABYTE) + 1;
	//Make it so the size is always a multiple of 64
	
	if (size > (buffsize * MEGABYTE / 2))
		size = (buffsize * MEGABYTE / 2);

	if (size >= 64)
	{
		int rest = size % 64;
		size -= rest;
	} 
	else if (size < 64)
	{
		size = 64;
	}
	return size;
}
