#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <iostream>

using namespace boost::interprocess;

//Typedefs of allocators and containers
typedef managed_shared_memory::segment_manager                                               segment_manager_t;
typedef allocator<void, segment_manager_t>                                                   void_allocator;
typedef std::uint32_t                                                                        metric_key_t;
typedef std::uint64_t                                                                        metric_value_t;
typedef std::pair<const metric_key_t, metric_value_t>                                        metrics_map_value_t;
typedef allocator<metrics_map_value_t, segment_manager_t>                                    metrics_map_allocator;
typedef map<metric_key_t, metric_value_t, std::less<metric_key_t>, metrics_map_allocator>    metrics_map;

class metrics_map_data {
   public:
      metrics_map metrics_map_;
      metrics_map_data(const void_allocator& void_allocator_instance)
         : metrics_map_(void_allocator_instance)
      {}
};

//Definition of the <host,metrics> map holding a string as key and complex_data as mapped type
typedef std::uint32_t                                                                  host_key_t;
typedef std::pair<const host_key_t, metrics_map_data>                                  host_map_value_t;
typedef allocator<host_map_value_t, segment_manager_t>                                 host_map_allocator;
typedef map<host_key_t, metrics_map_data, std::less<host_key_t>, host_map_allocator>   host_map;

int main () {
   //Remove shared memory on construction and destruction
   struct shm_remove
   {
      shm_remove() { shared_memory_object::remove("MySharedMemory"); }
      ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
   } remover;

   //Create shared memory
   managed_shared_memory segment(create_only,"MySharedMemory", 65536);
   void_allocator void_allocator_instance(segment.get_segment_manager());
   host_map *host_map_instance = segment.construct<host_map>("host_map")(std::less<host_key_t>(), void_allocator_instance);
   metrics_map_data mapped_metrics_map(void_allocator_instance);
   mapped_metrics_map.metrics_map_.insert({123,456});
   host_map_instance->insert({999,mapped_metrics_map});
   for (auto host_entry : *host_map_instance) {
      std::cout << "Host map content for key=" << host_entry.first << std::endl;
      for (auto metrics_entry : host_entry.second.metrics_map_) {
         std::cout << "\tMetrics entry: " << "<" << metrics_entry.first << "," << metrics_entry.second << ">" << std::endl;
      }
   }
   return 0;
}