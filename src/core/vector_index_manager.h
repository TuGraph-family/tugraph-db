// VectorIndexManager.h  

#include <unordered_map>  
#include <mutex>  
#include <memory>  
#include <string>  
#include <vector>  
#include "vector_index_layer.h" 
#include "db/galaxy.h"
#include "core/index_manager.h"
#include "core/lightning_graph.h"

namespace lgraph {

class VectorIndexManager {
private:
   size_t dataSize_;
   size_t threshold_ = 100000;
   std::string label_;
   std::string field_;
   bool indexed_ = false;

   size_t getDataSize() const { return dataSize_; }
   bool isIndexed() const { return indexed_; }

public:
   VectorIndexManager() {}

   VectorIndexManager(const std::string& label, const std::string& field, size_t dataSize)  
    : dataSize_(dataSize), threshold_(100000), label_(label), field_(field), indexed_(true) {}

   bool CheckdataSizeWhenCreate(KvTransaction& txn);

   bool CheckdataSizeWhenUpdate(KvTransaction& txn);

};
}