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
	// Returns the amount of members in the array
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
	// Adds an entry to the array
	//
	NTSTATUS push_back(T Entry)
	{
		//
		// Check if the existing pool is big enough to store the new entry
		//
		if (this->PoolSize >= sizeof(T) * (this->Count + 1))
		{
			//
			// If it is, just add it after the last entry and increment the entry count
			//
			RtlCopyMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * this->Count++), &Entry, sizeof(T));

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
		// If there are already entries, copy them to the new pool
		//
		if (this->Buffer != NULL)
		{
			RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * this->Count);
		}

		//
		// Copy the new entry to the new pool and increment the entry count
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
	// Removes the last entry from the array
	//
	NTSTATUS pop_back()
	{
		//
		// If the entry count is 0, there is nothing to remove
		//
		if (this->Count == 0)
		{
			return STATUS_SUCCESS;
		}

		//
		// If there is only one entry, clear the array
		//
		if (this->Count == 1)
		{
			this->clear();

			return STATUS_SUCCESS;
		}

		//
		// Calculate the required pool size
		//
		auto NewPoolSize = sizeof(T) * (this->Count - 1);

		//
		// Allocate a NoExecute pool to store the modified buffer without the last entry
		//
		auto NewPool = ExAllocatePool(NonPagedPoolNx, NewPoolSize);

		//
		// If the allocation failed, just zero the last entry and decrement the entry count
		//
		if (NewPool == NULL)
		{
			RtlZeroMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * --this->Count), sizeof(T));
			return STATUS_SUCCESS;
		}

		//
		// Zero the new pool
		//
		RtlZeroMemory(NewPool, NewPoolSize);

		//
		// Copy the buffer without the last entry to the new pool and decremtent the entry count
		//
		RtlCopyMemory(NewPool, this->Buffer, sizeof(T) * (--this->Count));

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
			return STATUS_INVALID_PARAMETER_1;
		}

		//
		// If the given index is equal to the current amount of 
		// entries, just use push_back to add it at the end
		//
		if (Idx == this->Count)
		{
			return this->push_back(Entry);
		}

		//
		// Check if the existing pool is big enough to store the new entry
		//
		if (this->PoolSize >= sizeof(T) * (this->Count + 1))
		{
			//
			// Shift the entries after the entry we're inserting 1 index higher and increment the entry count
			//
			RtlCopyMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * (Idx + 1)), (PVOID) ((ULONG64) this->Buffer + sizeof(T) * Idx), sizeof(T) * (this->Count++ - Idx));

			//
			// Copy the new entry in the array
			//
			RtlCopyMemory((PVOID) ((ULONG64) this->Buffer + sizeof(T) * Idx), &Entry, sizeof(T));

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
		
		//
		// Copy the old entries after the new entry to the new pool and increment the entry count
		//
		RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * (Idx + 1)), (PVOID) ((ULONG64) this->Buffer + sizeof(T) * Idx), sizeof(T) * (this->Count++ - Idx));

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
	// Ereses an entry at the given index, moving all
	// entries higher than that index 1 index lower
	//
	NTSTATUS erase(SIZE_T Idx)
	{
		//
		// Are you retarded?
		//
		if (Idx >= this->Count)
		{
			return STATUS_ARRAY_BOUNDS_EXCEEDED;
		}

		//
		// If the given index is equal to the index of 
		// the last entry, just remove the last entry
		//
		if (Idx == this->Count - 1)
		{
			return this->pop_back();
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
		// Copy the old entries after the given index to the new pool and decrement the entry count
		//
		RtlCopyMemory((PVOID) ((ULONG64) NewPool + sizeof(T) * Idx), (PVOID) ((ULONG64) this->Buffer + sizeof(T) * (Idx + 1)), sizeof(T) * (--this->Count - Idx));

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
		// Update the pool size
		//
		this->PoolSize = NewPoolSize;

		return STATUS_SUCCESS;
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

};
