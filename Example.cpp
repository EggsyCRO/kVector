// Example code:

#define PrintListInfo(List, PrintAllEntries) \
if (PrintAllEntries) \
{ \
	for (SIZE_T j = 0; j < List.size(); j++) \
	{ \
		Debug("List.at(%llu) %d", j, List.at(j)); \
	} \
} \
Debug("List.size() %llu", List.size()); \
Debug("List.pool_size() %llu", List.pool_size());

//-------------------------------------//

kVector<int> List;

Debug("Created the list");
PrintListInfo(List, false);

// ..

for (SIZE_T i = 0; i < 10; i++)
{
	List.push_back(i);
}

Debug("Added entries");
PrintListInfo(List, true);

// ..

List.resize(5);

Debug("Resized the pool");
PrintListInfo(List, true);

// ..

List.resize(10, 1337);

Debug("Resized the pool with new entries");
PrintListInfo(List, true);

// ..

List.pop_back();

Debug("Removed the last entry");
PrintListInfo(List, true);

// ..

for (SIZE_T j = 0; j < List.size(); j++)
{
	List.at(j) += 10;
}

Debug("Increased every entry by 10");
PrintListInfo(List, true);

// ..

List.swap(List.front(), List.back());

Debug("Swapped the first and last entry");
PrintListInfo(List, true);

// ..

List.erase(2);

Debug("Erased the entry at index 2");
PrintListInfo(List, true);

// ..

List.insert(0, 333);
Debug("Inserted 333 in the first entry");
PrintListInfo(List, true);

// ..

List.clear();

Debug("Cleared the list");
PrintListInfo(List, false);

//-------------------------------------//

// Debug output:

// > Created the list
// > List.size() 0
// > List.pool_size() 0
// > Added entries
// > List.at(0) 0
// > List.at(1) 1
// > List.at(2) 2
// > List.at(3) 3
// > List.at(4) 4
// > List.at(5) 5
// > List.at(6) 6
// > List.at(7) 7
// > List.at(8) 8
// > List.at(9) 9
// > List.size() 10
// > List.pool_size() 40
// > Resized the pool
// > List.at(0) 0
// > List.at(1) 1
// > List.at(2) 2
// > List.at(3) 3
// > List.at(4) 4
// > List.size() 5
// > List.pool_size() 20
// > Resized the pool with new entries
// > List.at(0) 0
// > List.at(1) 1
// > List.at(2) 2
// > List.at(3) 3
// > List.at(4) 4
// > List.at(5) 1337
// > List.at(6) 1337
// > List.at(7) 1337
// > List.at(8) 1337
// > List.at(9) 1337
// > List.size() 10
// > List.pool_size() 40
// > Removed the last entry
// > List.at(0) 0
// > List.at(1) 1
// > List.at(2) 2
// > List.at(3) 3
// > List.at(4) 4
// > List.at(5) 1337
// > List.at(6) 1337
// > List.at(7) 1337
// > List.at(8) 1337
// > List.size() 9
// > List.pool_size() 36
// > Increased every entry by 10
// > List.at(0) 10
// > List.at(1) 11
// > List.at(2) 12
// > List.at(3) 13
// > List.at(4) 14
// > List.at(5) 1347
// > List.at(6) 1347
// > List.at(7) 1347
// > List.at(8) 1347
// > List.size() 9
// > List.pool_size() 36
// > Swapped the first and last entry
// > List.at(0) 1347
// > List.at(1) 11
// > List.at(2) 12
// > List.at(3) 13
// > List.at(4) 14
// > List.at(5) 1347
// > List.at(6) 1347
// > List.at(7) 1347
// > List.at(8) 10
// > List.size() 9
// > List.pool_size() 36
// > Erased the entry at index 2
// > List.at(0) 1347
// > List.at(1) 11
// > List.at(2) 13
// > List.at(3) 14
// > List.at(4) 1347
// > List.at(5) 1347
// > List.at(6) 1347
// > List.at(7) 10
// > List.size() 8
// > List.pool_size() 32
// > Inserted 333 in the first entry
// > List.at(0) 333
// > List.at(1) 1347
// > List.at(2) 11
// > List.at(3) 13
// > List.at(4) 14
// > List.at(5) 1347
// > List.at(6) 1347
// > List.at(7) 1347
// > List.at(8) 10
// > List.size() 9
// > List.pool_size() 36
// > Cleared the list
// > List.size() 0
// > List.pool_size() 0
