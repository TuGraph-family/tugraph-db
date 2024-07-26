// VectorIndexManager.cpp  
#include "vector_index_manager.h"

namespace lgraph {

VectorIndexManager::VectorIndexManager(size_t count)  
    : count_(count),  indexed_(false){}

VectorIndexManager::VectorIndexManager(const VectorIndexManager& rhs)
    : count_(rhs.count_),
      indexed_(rhs.indexed_) {}


void VectorIndexManager::addCount() {
    count_++;
}

bool VectorIndexManager::MakeVectorIndex() { 
    indexed_ = true;
    return true; 
}

bool VectorIndexManager::UpdateCount(size_t DataSize) {
    count_ = count_ + DataSize;
    return true;
} 

bool VectorIndexManager::WhetherUpdate() {
    if (count_ >= 10000) {
        count_ = 0;
        return true;
    } else {
        return false;
    }
}

}