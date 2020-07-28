#pragma once

template <typename T>
class kVector
{
private:

	T* Buffer;
	SIZE_T Count;

public:

	//
	// Initializes the array
	//
	kVector()
	{
		this->Count = 0;
		this->Buffer = NULL;
	}

	//
	// Destroys the array
	//
	~kVector()
	{
		this->clear();
	}

	//
	// Returns the amount of members in the array
	//
	SIZE_T size()
	{
		return this->Count;
	}

	//
	// Returns a reference to the entry at the specified index
	//
	T& at(SIZE_T Idx)
	{
		return this->Buffer[Idx];
	}

	//
	// Removes all entries from the array
	//
	void clear()
	{
		//
		// If the count is 0 there is nothing to clear
		//
		if (this->Count == 0)
		{
			return;
		}

		//
		// Zero the entries if they exist and free the containing pool
		//
		if (this->Buffer != NULL)
		{
			RtlZeroMemory(this->Buffer, sizeof(T) * this->Count);
			ExFreePool(this->Buffer);
		}

		this->Count = 0;
		this->Buffer = NULL;
	}

	//
	// Adds an entry to the array
	//
	NTSTATUS push_back(T Entry)
	{
		//
		// Allocate a NoExecute pool to store the new buffer
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, sizeof(T) * (this->Count + 1));

		if (NewPool == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//
		// If there are already entries, copy them to the new pool
		//
		if (this->Buffer != NULL)
		{
			RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * this->Count);
		}

		//
		// Copy the new entry to the new pool and increment the count
		//
		RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * this->Count++), &Entry, sizeof(T));
		
		//
		// Set the pointer to the buffer as the address
		// of the new pool and save the old pointer
		//
		auto OldPool = InterlockedExchangePointer((volatile PVOID*) &this->Buffer, NewPool);

		//
		// If there was alredy a pool containing entries, zero it and free it
		//
		if (OldPool != NULL)
		{
			RtlZeroMemory(OldPool, sizeof(T) * (this->Count - 1));
			ExFreePool(OldPool);
		}

		return STATUS_SUCCESS;
	}

	//
	// Removes the last entry from the array
	//
	NTSTATUS pop_back()
	{
		//
		// If the count is 0, there is nothing to remove
		//
		if (this->Count == 0)
		{
			return STATUS_SUCCESS;
		}

		//
		// Allocate a NoExecute pool to store the modified buffer without the last entry
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, sizeof(T) * (this->Count - 1));

		//
		// If the allocation failed, just zero the last entry and decrement the count
		//
		if (NewPool == NULL)
		{
			RtlZeroMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * --this->Count), sizeof(T));
			return STATUS_SUCCESS;
		}

		//
		// Copy the buffer without the last entry to the new pool and decremtent the count
		//
		RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * (--this->Count));

		//
		// Set the pointer to the buffer as the address
		// of the new pool and save the old pointer
		//
		auto OldPool = InterlockedExchangePointer((volatile PVOID*) &this->Buffer, NewPool);

		//
		// If there was alredy a pool containing entries, zero it free it
		//
		if (OldPool != NULL)
		{
			RtlZeroMemory(OldPool, sizeof(T) * (this->Count + 1));
			ExFreePool(OldPool);
		}

		return STATUS_SUCCESS;
	}

	//
	// Swaps the given entries in the array
	//
	void swap(T& Left, T& Right)
	{
		T Temp = Left;
		Left = Right;
		Right = Temp;
	}

};
