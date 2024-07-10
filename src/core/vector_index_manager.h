// VectorIndexManager.h  

#pragma once

#include <unordered_map>  
#include <mutex>  
#include <memory>  
#include <string>  
#include <vector>  

#include "/root/tugraph-db/include/tools/lgraph_log.h"


namespace lgraph {

class VectorIndexManager {
   std::string label_;
   std::string name_;
   size_t count_;
   bool indexed_;

public:

   VectorIndexManager(size_t count, std::string label, std::string name);

   VectorIndexManager(const VectorIndexManager& rhs);

   VectorIndexManager(VectorIndexManager&& rhs) = delete;

   VectorIndexManager& operator=(const VectorIndexManager& rhs) = delete;

   VectorIndexManager& operator=(VectorIndexManager&& rhs) = delete;

   size_t getCount() { return count_; }

   std::string getLabel() { return label_; }

   std::string getName() { return name_; }

   bool isIndexed() { return indexed_; }

   void addCount();

   bool MakeVectorIndex();

   bool UpdateCount(size_t DataSize);

   bool WhetherUpdate();

};
}