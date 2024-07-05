// VectorIndexManager.cpp  
#include "vector_index_manager.h"

namespace lgraph {

VectorIndexManager::VectorIndexManager(size_t count, std::string label, std::string name)  
    : label_(label), name_(name),
      count_(count),  indexed_(false){}

VectorIndexManager::VectorIndexManager(const VectorIndexManager& rhs)
    : label_(rhs.label_), 
      name_(rhs.name_),
      count_(rhs.count_),
      indexed_(rhs.indexed_) {}

bool VectorIndexManager::WhetherUpdate() {
    auto count = getCount();
    if (count >= 10000) {
        count = 0;
        UpdateCount(count);
        return true;
    } else {
        return false;
    }
}

}