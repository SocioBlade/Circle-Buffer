#include "ComLib.h"

ComLib::ComLib(const std::string& secret, const size_t& buffSize, TYPE type)
{
	this->mSize = buffSize;

	size_t mem = mSize + (3 * (sizeof(size_t)));

	hFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		mem,
		secret.c_str());
	
	cout << "filemap created" << endl;

	if (hFileMap == NULL)
	{ 
		cout << "FATAL ERROR" << endl;
		cout << "Last error " << GetLastError() << endl;
	}

	mData = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (mData == NULL)
	{
		cout << "Could not map view of file " << GetLastError() << endl;
		
		CloseHandle(hFileMap);
	}
	
	mDataStartPos = mData; //<--- Save the first assigned address to a const pointer
	
	head = mDataStartPos;
	head = static_cast<char*>(head) + mSize;
	
	tail = mDataStartPos;
	tail = static_cast<char*>(tail) + (mSize + sizeof(int));
	
	prodLoopPtr = mDataStartPos;
	prodLoopPtr = static_cast<char*>(prodLoopPtr) + (mSize + (sizeof(int) * 2));
	memcpy(&prodLoop, prodLoopPtr, sizeof(int));

	if (prodLoop == NULL)
	{
		prodLoop = 0;
		memcpy(prodLoopPtr, &prodLoop, sizeof(int));
	}
}

bool ComLib::send(const void* msg, const size_t length)
{ 

	while (checkMemStatus(length) != true)
	{
		memcpy(&headOffset, head, sizeof(int));
	}
	

	// Re:format the couts to be both cleaner and make more sense
	memcpy(mData, &length, sizeof(int));

	mData = static_cast<char*>(mData) + sizeof(int);
	memcpy(mData, msg, length);

	mData = static_cast<char*>(mData) + length;
	// Update head
	headOffset += length + sizeof(int);
	memcpy(head, &headOffset, sizeof(int));

	std::cout << "HEAD: " << headOffset << std::endl;
	std::cout << "TAIL: " << tailOffset << std::endl;
	return true;
}

bool ComLib::recv(char* msg, size_t& length)
{
	int testLen;
	int8_t msgStatus = 0;
	consumCheck = false;
	
	memcpy(&testLen, mData, sizeof(int));
	//Check if the consumer should read or not
	while (checkReadStatus() != true)
	{
		memcpy(&testLen, mData, sizeof(int));
	}
	
	memcpy(&testLen, mData, sizeof(int));

	mData = static_cast<char*>(mData) + (sizeof(int)); 

	memcpy(msg, mData, testLen);
	std::cout << msg << std::endl;

	mData = static_cast<char*>(mData) + testLen;


	tailOffset += testLen + sizeof(int);
	memcpy(tail, &tailOffset, sizeof(int));

	std::cout << "HEAD: " << headOffset << std::endl;
	std::cout << "TAIL: " << tailOffset << std::endl;
	return true;
}

size_t ComLib::nextSize()
{
	return 0;
}


bool ComLib::checkReadStatus()
{
	int m_length;
	bool read = false;
	
	memcpy(&headOffset, head, sizeof(int)); //<--- Update headOffset
	memcpy(&tailOffset, tail, sizeof(int));
	memcpy(&m_length, mData, sizeof(m_length));

	if (tailOffset != headOffset)
	{
		if (m_length == 0)
		{
			memcpy(&m_length, mData, sizeof(m_length));
		}
		else if (m_length != -1)
		{
			read = true;
		}
		else
		{
			prodLoop = 0;
			memcpy(prodLoopPtr, &prodLoop, sizeof(int));

			mData = mDataStartPos;

			while (headOffset == 0)
			{
				memcpy(&headOffset, head, sizeof(int));
			}

			tailOffset = 0;
			memcpy(tail, &tailOffset, sizeof(int)); //<--- Is this needed?

			memcpy(&m_length, mData, sizeof(m_length));

			read = true;
			std::cout << "=======LOOOOOOP========" << std::endl;
		}
	}
	return read;
}

bool ComLib::checkMemStatus(size_t length)
{
	bool isMemLeft = false;
	int mCase;

	//REFRESH UPDATE
	memcpy(&headOffset, head, sizeof(int));
	memcpy(&tailOffset, tail, sizeof(int));

	if (headOffset >= tailOffset) //<--- If head is in front or in lign with tail
		mCase = 0;
	if (tailOffset > headOffset) //<--- If Tail is in front of Head
		mCase = 1;

	if (mCase == 0)
	{
		int memLeft = mSize - headOffset;
		int totLength = length + (2 * sizeof(int)); //<--- Possible issue
		
		memcpy(&prodLoop, prodLoopPtr, sizeof(int));

		if (prodLoop == 0 && memLeft > totLength)
		{
			isMemLeft = true;
		}
		else
		{
			//Memory left is not enough
			int newLength = -1;
			memcpy(mData, &newLength, sizeof(newLength));

			//This while loop ensures that consumer must have started reading before
			//the loop actually can take place
			while (tailOffset == 0)
			{
				memcpy(&tailOffset, tail, sizeof(int)); //<--- Updates the tailOffset Variable
			}

			mData = static_cast<char*>(mData) + sizeof(int);

			mData = mDataStartPos; //<--- Reset mData position in memory
			//memLeft = mSize; //<--- Reset memLeft to allow for further checks
			mCase = 1;
			headOffset = 0; //<-- Reset headOffset
			memcpy(head, &headOffset, sizeof(int)); //<-- Write it to memeory
			
			prodLoop = 1; //<--- Producer is behind consumer
			memcpy(prodLoopPtr, &prodLoop, sizeof(int));

			std::cout << "=======LOOOOOOP========" << std::endl;
		}
	}
	if (mCase == 1)
	{
		int differnce = tailOffset - headOffset;

		// Only return true if producer has looped and the difference between 
		// head and tail allow for a new message
		if (differnce > length + (2 * sizeof(int)))
			isMemLeft = true; 
	} 

	return isMemLeft;
}


ComLib::~ComLib()
{
	cout << "Destroy the world" << endl;
	UnmapViewOfFile((LPCVOID)mData);
	CloseHandle(hFileMap);
}


