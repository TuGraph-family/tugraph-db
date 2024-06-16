// VectorIndexManager.h  

#pragma once

#include <unordered_map>  
#include <mutex>  
#include <memory>  
#include <string>  
#include <vector>  


namespace lgraph {

class VectorIndexManager {
private:
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

   void addCount() { count_++; }

   bool MakeVectorIndex() { indexed_ = true; return indexed_; }

   //std::vector<std::vector<float>> getData(KvTransaction& txn, Schema schema);

   bool UpdateCount(size_t& DataSize) { count_ = DataSize;  return true; } 

   bool WhetherUpdate();

};
}