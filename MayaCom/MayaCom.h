#ifndef _MAYACOM_H
#define _MAYACOM_H

#include <Windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <queue>
#include <sstream>
#include <map>
#include <unordered_map>
#include <mutex>

namespace MayaComLib
{
	using namespace std;

	class MayaCom
	{
	public:
		// buffSize is in MEGABYTES (multiple of 1<<20)
		MayaCom(const size_t& buffSize = 1);
		~MayaCom();

		// returns "true" if data was sent successfully.
		// false if for ANY reason the data could not be sent.
		// we will not implement an "error handling" mechanism, so we will assume
		// that false means that there was no space in the buffer to put the message.
		// msg is a void pointer to the data.
		// length is the amount of bytes of the message to Write.
		void Write(const void* msg, const size_t length);

		bool CanRead(const size_t length);
		bool CanWrite(const size_t length);


		// returns: "true" if a message was received.
		// false if there was nothing to read.
		// "msg" is expected to have enough space for the message.
		// use "NextSize()" to check whether our temporary buffer is big enough
		// to hold the next message.
		// @length returns the size of the message just read.
		// @msg contains the actual message read.
		bool Read(char* msg, const size_t length);

		// return the length of the next message
		// return 0 if no message is available.
		size_t NextSize();

		size_t GetAvailableMemory();

		// Create the shared memory FileMap
		bool CreateFileMap();
		bool OpenFileMap();
		void* GetMap();
		bool CanClose();

		void UnmapView();
		void CloseFile();



	private:

		

		// 1) define variables (global or in a struct/class)
		HANDLE hFileMap;
		void* mData;
		bool exists;

		size_t mSize;
		size_t availableMem;

		int maxCharOffset;

		int headOffset;
		int tailOffset;
		int loopedOffset;
		int rsOffset;

		int head;
		int tail;

		int looped;

		int writeState;
		int readState;



	};
}
#endif