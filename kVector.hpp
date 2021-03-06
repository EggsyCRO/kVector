#pragma once

template <typename T>
class kVector
{
private:

	T* Buffer;
	SIZE_T Count;
	SIZE_T PoolSize;

public:

	//
	// Initializes the instance
	//
	kVector()
	{
		this->Count = 0;
		this->PoolSize = 0;
		this->Buffer = NULL;
	}

	//
	// Destroys the instance
	//
	~kVector()
	{
		this->clear();
	}

	//
	// Returns the amount of entries in the array
	//
	SIZE_T size()
	{
		return this->Count;
	}

	//
	// Returns the size of the pool which contains the entries
	//
	SIZE_T pool_size()
	{
		return this->PoolSize;
	}

	//
	// Returns a reference to the entry at the specified index
	//
	T& at(SIZE_T Idx)
	{
		return this->Buffer[Idx];
	}

	//
	// Returns a reference to the first entry in the array
	//
	T& front()
	{
		return this->at(0);
	}

	//
	// Returns a reference to the last entry in the array
	//
	T& back()
	{
		return this->at(this->Count - 1);
	}

	//
	// Returns a pointer to the buffer containing the entries
	//
	T* data()
	{
		return this->Buffer;
	}

	//
	// Returns true if the array has no entries
	//
	bool empty()
	{
		return this->Count == 0;
	}

	//
	// Removes all entries from the array
	//
	void clear()
	{
		//
		// If the pool size is 0 there is nothing to clear
		//
		if (this->PoolSize == 0)
		{
			return;
		}

		//
		// Zero the entries if they exist and free the containing pool
		//
		if (this->Buffer != NULL)
		{
			RtlZeroMemory(this->Buffer, this->PoolSize);
			ExFreePool(this->Buffer);
		}

		this->Count = 0;
		this->PoolSize = 0;
		this->Buffer = NULL;
	}

	//
	// Inserts an entry at the end of the array
	//
	NTSTATUS push_back(T Entry)
	{
		return this->insert(this->Count, Entry);
	}

	//
	// Removes the last entry from the array
	//
	NTSTATUS pop_back()
	{
		return this->erase(this->Count - 1);
	}

	//
	// Swaps the given entries in the array
	//
	void swap(T& Left, T& Right)
	{
		T Temp = { };

		//
		// Exchange the memory of Left and Right using a temporary variable
		//
		RtlCopyMemory(&Temp, &Left, sizeof(T));
		RtlCopyMemory(&Left, &Right, sizeof(T));
		RtlCopyMemory(&Right, &Temp, sizeof(T));
	}

	//
	// Allocate a new pool with the given size and copy the old entries there
	//
	NTSTATUS resize(SIZE_T NewEntryCount)
	{
		//
		// If the new entry count is 0, clear the array
		//
		if (NewEntryCount == 0)
		{
			this->clear();

			return STATUS_SUCCESS;
		}

		//
		// If all entries won't fit in the new pool, remove the entries from the back
		//
		if (this->Count > NewEntryCount)
		{
			this->Count = NewEntryCount;
		}

		//
		// Calculate the new pool size
		//
		auto NewPoolSize = sizeof(T) * NewEntryCount;

		//
		// Allocate a NoExecute pool to store the buffer
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, NewPoolSize);

		if (NewPool == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//
		// Zero the new pool
		//
		RtlZeroMemory(NewPool, NewPoolSize);

		//
		// If there are already entries, copy them to the new pool
		//
		if (this->Buffer != NULL)
		{
			RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * this->Count);
		}

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
			RtlZeroMemory(OldPool, this->PoolSize);
			ExFreePool(OldPool);
		}

		//
		// Update the pool size
		//
		this->PoolSize = NewPoolSize;

		return STATUS_SUCCESS;
	}

	//
	// Allocate a new pool with the given size and copy the old entries there,
	// the remaining space in the pool is filled with NewEntries
	//
	NTSTATUS resize(SIZE_T NewEntryCount, T NewEntries)
	{	
		//
		// If the new entry count is 0, clear the array
		//
		if (NewEntryCount == 0)
		{
			this->clear();

			return STATUS_SUCCESS;
		}

		//
		// If all entries won't fit in the new pool, remove the entries from the back
		//
		if (this->Count > NewEntryCount)
		{
			this->Count = NewEntryCount;
		}

		//
		// Calculate the new pool size
		//
		auto NewPoolSize = sizeof(T) * NewEntryCount;

		//
		// Allocate a NoExecute pool to store the buffer
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, NewPoolSize);

		if (NewPool == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//
		// Zero the new pool
		//
		RtlZeroMemory(NewPool, NewPoolSize);	

		//
		// If there are already entries, copy them to the new pool
		//
		if (this->Buffer != NULL)
		{
			RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * this->Count);
		}

		//
		// While there is remaining space in the pool, fill it with the new entries and increment the count
		//
		while (NewEntryCount > this->Count)
		{
			RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * this->Count++), &NewEntries, sizeof(T));
		}

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
			RtlZeroMemory(OldPool, this->PoolSize);
			ExFreePool(OldPool);
		}

		//
		// Update the pool size
		//
		this->PoolSize = NewPoolSize;

		return STATUS_SUCCESS;
	}
	
	//
	// Inserts an entry at the given index, moving all
	// entries at that index or higher 1 index higher
	//
	NTSTATUS insert(SIZE_T Idx, T Entry)
	{
		//
		// Are you retarded?
		//
		if (Idx > this->Count)
		{
			return STATUS_ARRAY_BOUNDS_EXCEEDED;
		}

		//
		// Check if the existing pool is big enough to store the new entry
		//
		if (this->PoolSize >= sizeof(T) * (this->Count + 1))
		{
			//
			// Shift the entries after the entry we're inserting 1 index higher if they exist
			//
			if (Idx < this->Count)
			{
				RtlCopyMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * (Idx + 1)), (PVOID) ((ULONG64) this->Buffer + sizeof(T) * Idx), sizeof(T) * (this->Count - Idx));
			}

			//
			// Copy the new entry in the array
			//
			RtlCopyMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * Idx), &Entry, sizeof(T));

			//
			// Increment the entry count
			//
			this->Count++;

			return STATUS_SUCCESS;
		}

		//
		// Calculate the required pool size
		//
		auto NewPoolSize = sizeof(T) * (this->Count + 1);

		//
		// Allocate a NoExecute pool to store the new buffer
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, NewPoolSize);

		if (NewPool == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//
		// Zero the new pool
		//
		RtlZeroMemory(NewPool, NewPoolSize);

		//
		// Check if there are old entries before the entry we're inserting
		//
		if (Idx != 0)
		{
			//
			// Copy the old entries before the new entry to the new pool
			//
			RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * Idx);
		}

		//
		// Copy the new entry to the new pool
		//
		RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * Idx), &Entry, sizeof(T));	

		if (Idx < this->Count)
		{
			//
			// Copy the old entries after the new entry to the new pool and increment the entry count
			//
			RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * (Idx + 1)), (PVOID) ((ULONG64) this->Buffer + sizeof(T) * Idx), sizeof(T) * (this->Count - Idx));
		}

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
			RtlZeroMemory(OldPool, this->PoolSize);
			ExFreePool(OldPool);
		}

		//
		// Update the pool size and increment the entry count
		//
		this->PoolSize = NewPoolSize;
		this->Count++;

		return STATUS_SUCCESS;
	}

	//
	// Ereses an entry at the given index, moving all
	// entries higher than that index 1 index lower
	//
	NTSTATUS erase(SIZE_T Idx)
	{
		//
		// If the entry count is 0, there is nothing to remove
		//
		if (this->Count == 0)
		{
			return STATUS_SUCCESS;
		}

		//
		// Are you retarded?
		//
		if (Idx >= this->Count)
		{
			return STATUS_ARRAY_BOUNDS_EXCEEDED;
		}

		//
		// Calculate the required pool size
		//
		auto NewPoolSize = sizeof(T) * (this->Count - 1);

		//
		// Allocate a NoExecute pool to store the new buffer
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, NewPoolSize);

		if (NewPool == NULL)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//
		// Zero the new pool
		//
		RtlZeroMemory(NewPool, NewPoolSize);

		//
		// Check if there are old entries before the entry we're erasing
		//
		if (Idx != 0)
		{
			//
			// Copy the old entries before the given index to the new pool
			//
			RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * Idx);
		}

		//
		// Check if there are old entries after the entry we're erasing
		//
		if (Idx < this->Count - 1)
		{
			//
			// Copy the old entries after the given index to the new pool
			//
			RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * Idx), (PVOID) ((ULONG64) this->Buffer + sizeof(T) * (Idx + 1)), sizeof(T) * (this->Count - (Idx + 1)));
		}

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
			RtlZeroMemory(OldPool, sizeof(T) * (this->Count + 1));
			ExFreePool(OldPool);
		}

		//
		// Update the pool size and decrement the entry count
		//
		this->PoolSize = NewPoolSize;
		this->Count--;

		return STATUS_SUCCESS;
	}

};
