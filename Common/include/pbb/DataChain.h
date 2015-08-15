#ifndef __PBB_DATA_CHAIN_H__
#define __PBB_DATA_CHAIN_H__
#include <pbb/pbb.h>
#include <stdlib.h>
#include <string.h> // memcpy

namespace pbb
{
	class DataChain
	{
	public:
		/**
		  Allocate a buffer
		  @param capacity  Total capacity of the buffer
		  @param before    How many bytes to reserve BEFORE the start.  Reduces your starting capacity
		 */
		DataChain(uint32_t capacity, uint32_t before=0)
		{
			mBuffer = new uint8_t[capacity];
			mStart = &mBuffer[before];
			mLength = 0;
			mCapacity = capacity - before;
			mBefore = before;
		}

		void Reset()
		{
			mCapacity = (mStart + mCapacity) - mBuffer - mBefore;
			mStart = &mBuffer[mBefore];
			mLength = 0;			
		}

		uint32_t AddTail(const void* buff, uint32_t len)
		{
			// Put as much as we can
			uint32_t avail = Available();
			if (avail < len) len = avail;

			memcpy(&mStart[mLength], buff, len);
			mLength += len;
			return len;
		}

		uint32_t AddHead(const void* buff, uint32_t len)
		{
			// Put as much as we can
			uint32_t avail = AvailableHead();
			if (avail < len) len = avail;

			memcpy(mStart-len, buff, len);
			mStart -= len;
			mLength += len;
			mCapacity += len;
			return len;
		}

		template<typename T>
		uint32_t AddHead(T t)
		{
			return AddHead(&t, sizeof(T));
		}

		/**
		  Total you can add to tail
		 */
		uint32_t Available()
		{
			return &mBuffer[mCapacity] - &mStart[mLength];
		}

		uint32_t AvailableHead()
		{
			return mStart - mBuffer;
		}

		template<typename T=uint8_t>
		T* GetBuffer()
		{
			return (T*)mStart;
		}

		/**
		Remove bytes from front of buffer
		@param len  Number of bytes to shift off front of buffer
		*/
		void Shift(void* dest, uint32_t len)
		{
			if (len <= mLength)
			{
				memcpy(dest, mStart, len);
				mLength -= len;
				memmove(mStart, &mStart[len], mLength);
			}
		}

		/**
		  Remove bytes from front of buffer
		  @param len  Number of bytes to shift off front of buffer
		 */
		void Shift(uint32_t len)
		{
			if (len <= mLength)
			{
				mLength -= len;
				memmove(mStart, &mStart[len], mLength);
			}
		}
		/**
		  Number of bytes in buffer
		 */
		uint32_t Size() { return mLength; }

		/**
		  Total number of bytes buffer can hold
		 */
		uint32_t Capacity() { return mCapacity; }
	protected:

		// Pointer to actual buffer
		uint8_t* mBuffer;   
		// Pointer to start of data.  This may be different from mBuffer to allow
		// data to be added to start of buffer
		uint8_t* mStart;
		uint32_t mCapacity;  // Total suze of mBuffer in bytes
		uint32_t mLength;   // Total number of bytes

		uint32_t mBefore;   // used in Reset
	private:
	};
}
#endif
