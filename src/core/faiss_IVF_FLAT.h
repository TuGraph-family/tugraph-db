#include "string.h"
#include "vector_index_layer.h"


namespace lgraph {

template <typename T>
class Faiss_IVFFLAT : public VectorIndex { 
 private: 
    std::string distance_type_;
    int vec_dimension_;
    int nlist_;
    int nprobe_;
    int num_of_vector_;
    faiss::Index* index_;

 public: 
    Faiss_IVFFLAT(int d, int n, int nv) {
        vec_dimension_ = d;
        nlist_ = n;
        nprobe_ = 1;
        num_of_vector_ = nv;
        // TODO
        /* eg
        faiss::IndexFlatL2 quantizer(vec_dimension); //创建量化器
        faiss::IndexIVFFlat index_(&quantizer,vec_dimension,nlist,distance_type)    
        */
    }

    // add vector to index 
    void Add(const std::vector<std::vector<T>>& vectors, size_t num_vectors) override { 
        // TODO 
    } 

    // build index
    void Build() override { 
        // TODO
    } 

    // serialize index
    std::vector<uint8_t> Save() override {
        // TODO 
    }

    // load index form serialization
    void Load(std::vector<uint8_t>& idx_bytes) override {
        // TODO
    }

    // search vector in index
    std::vector<std::vector<T>> Search(const std::vector<float>& query, size_t num_results) override { 
        // TODO 
    } 


    ~Faiss_IVFFLAT() { 
        if (index_) { 
            delete index_; 
        } 
    } 
};
}
