#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <functional>
#include <utility>

#define SEGMENT_NAME "MySharedMemory"
#define OBJECT_NAME  "MyMap"

using namespace boost::interprocess;

//Note that map<Key, MappedType>'s value_type is std::pair<const Key, MappedType>,
//so the allocator must allocate that pair.
typedef int    KeyType;
typedef float  MappedType;
typedef std::pair<const int, float> ValueType;

//Alias an STL compatible allocator of for the map.
//This allocator will allow to place containers
//in managed shared memory segments
typedef allocator<ValueType, managed_shared_memory::segment_manager> ShmemAllocator;

//Alias a map of ints that uses the previous STL-like allocator.
//Note that the third parameter argument is the ordering function
//of the map, just like with std::map, used to compare the keys.
typedef map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MyMap;

int main (int argc, char *argv[]) {
   if(argc == 1) { //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() { shared_memory_object::remove(SEGMENT_NAME); }
         ~shm_remove(){ shared_memory_object::remove(SEGMENT_NAME); }
      } remover;

      //Shared memory front-end that is able to construct objects
      //associated with a c-string. Erase previous shared memory with the name
      //to be used and create the memory segment at the specified address and initialize resources
      managed_shared_memory segment
         (create_only
         ,SEGMENT_NAME //segment name
         ,65536);          //segment size in bytes

      //Initialize the shared memory STL-compatible allocator
      ShmemAllocator alloc_inst (segment.get_segment_manager());

      //Construct a shared memory map.
      //Note that the first parameter is the comparison function,
      //and the second one the allocator.
      //This the same signature as std::map's constructor taking an allocator
      MyMap *mymap =
         segment.construct<MyMap>(OBJECT_NAME)      //object name
                                    (std::less<int>() //first  ctor parameter
                                    ,alloc_inst);     //second ctor parameter

      //Insert data in the map
      for(int i = 0; i < 100; ++i){
         mymap->insert(std::pair<const int, float>(i, (float)i));
      }

      //Launch child process
      std::string s(argv[0]); s += " child ";
      if(0 != std::system("/home/mehmet/vscode\\ workspaces/boost-interprocess/build/boost-interprocess child"))
         return 1;

      //Check child has destroyed the map
      if(segment.find<MyMap>(OBJECT_NAME).first)
         return 1;
   } else { //Child process
      //Open the managed segment
      managed_shared_memory segment(open_only, SEGMENT_NAME);

      //Find the map using the c-string name
      MyMap *mymap = segment.find<MyMap>(OBJECT_NAME).first;

      for (ValueType vt : *mymap) {
         std::cout << vt.first << " " << vt.second << std::endl;
      }

      //When done, destroy the map from the segment
      segment.destroy<MyMap>(OBJECT_NAME);
   }
   return 0;
}