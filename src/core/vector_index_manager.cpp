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
    
// std::vector<std::vector<float>> VectorIndexManager::getData(KvTransaction& txn, Schema schema) {
//     std::vector<std::vector<float>> floatvector;
//     const _detail::FieldExtractor* extractor = schema.GetFieldExtractor(getName());
//     auto kv_iter = schema.GetPropertyTable().GetIterator(txn);
//     for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
//         auto prop = kv_iter->GetValue();
//         if (extractor->GetIsNull(prop)) {
//             continue;
//         }
//         floatvector.emplace_back(prop.AsType<std::vector<float>>());
//     }
//     return floatvector;
// }

bool VectorIndexManager::WhetherUpdate() {
    auto count = getCount();
    if (count >= 1000000) {
        count = 0;
        UpdateCount(count);
        return true;
    } else {
        return false;
    }
}

}