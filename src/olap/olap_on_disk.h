/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "olap/olap_io.h"
#include "fma-common/text_dir_stream.h"
#include "fma-common/text_parser.h"

namespace lgraph_api {
namespace olap {

template <typename EdgeData>
class OlapOnDisk : public OlapBase<EdgeData> {
 protected:
    void Construct() {
        if (this->num_vertices_ == 0 || this->num_edges_ == 0) {
            throw std::runtime_error("Construct empty graph");
        }

        this->lock_array_.ReAlloc(this->num_vertices_);
        this->lock_array_.Resize(this->num_vertices_, false);

        bool dual = true;
        if (this->edge_direction_policy_ == MAKE_SYMMETRIC) {
            this->out_edges_.ReAlloc(this->num_edges_ * 2);
            this->out_edges_.Resize(this->num_edges_ * 2);
        } else {
            this->out_edges_.ReAlloc(this->num_edges_);
            this->out_edges_.Resize(this->num_edges_);
        }
        this->out_degree_.ReAlloc(this->num_vertices_);
        this->out_degree_.Resize(this->num_vertices_, (size_t)0);
        this->out_index_.ReAlloc(this->num_vertices_ + 1);
        this->out_index_.Resize(this->num_vertices_ + 1, (size_t)0);

        if (this->edge_direction_policy_ == DUAL_DIRECTION) {
            this->in_edges_.ReAlloc(this->num_edges_);
            this->in_edges_.Resize(this->num_edges_);
            this->in_degree_.ReAlloc(this->num_vertices_);
            this->in_degree_.Resize(this->num_vertices_, (size_t)0);
            this->in_index_.ReAlloc(this->num_vertices_ + 1);
            this->in_index_.Resize(this->num_vertices_ + 1, (size_t)0);
        }

        if (this->edge_direction_policy_ == DUAL_DIRECTION) {
            auto worker = Worker::SharedWorker();
            worker->Delegate([&]() {
#pragma omp parallel for default(none)
              for (size_t ei = 0; ei < this->num_edges_; ei++) {
                  size_t src = this->edge_list_[ei].src;
                  size_t dst = this->edge_list_[ei].dst;
                  __sync_fetch_and_add(&this->out_degree_[src], (size_t)1);
                  __sync_fetch_and_add(&this->in_degree_[dst], (size_t)1);
              }

              memcpy(this->out_index_.Data() + 1, this->out_degree_.Data(),
                                        sizeof(size_t) * this->num_vertices_);
              this->out_index_[0] = 0;
              if (dual) {
                  memcpy(this->in_index_.Data() + 1, this->in_degree_.Data(),
                                        sizeof(size_t) * this->num_vertices_);
                  this->in_index_[0] = 0;
              }

              for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                  this->out_index_[vi + 1] += this->out_index_[vi];
              }
              if (dual) {
                  for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                      this->in_index_[vi + 1] += this->in_index_[vi];
                  }
              }

#pragma omp parallel for
              for (size_t ei = 0; ei < this->num_edges_; ei++) {
                  size_t src = this->edge_list_[ei].src;
                  size_t dst = this->edge_list_[ei].dst;

                  size_t pos = __sync_fetch_and_add(&this->out_index_[src], (size_t)1);
                  this->out_edges_[pos].neighbour = dst;
                  if (this->edge_data_size_ != 0) {
                      this->out_edges_[pos].edge_data = this->edge_list_[ei].edge_data;
                  }

                  if (dual) {
                      pos = __sync_fetch_and_add(&this->in_index_[dst], (size_t)1);
                      this->in_edges_[pos].neighbour = src;
                      if (this->edge_data_size_ != 0) {
                          this->in_edges_[pos].edge_data = this->edge_list_[ei].edge_data;
                      }
                  }
              }

              memmove(this->out_index_.Data() + 1, this->out_index_.Data(),
                                                sizeof(size_t) * this->num_vertices_);
              this->out_index_[0] = 0;
              if (dual) {
                  memmove(this->in_index_.Data() + 1, this->in_index_.Data(),
                                                sizeof(size_t) * this->num_vertices_);
                  this->in_index_[0] = 0;
              }
            });
        } else {
            auto worker = Worker::SharedWorker();
            worker->Delegate([&]() {
#pragma omp parallel for default(none)
              for (size_t ei = 0; ei < this->num_edges_; ei++) {
                  size_t src = this->edge_list_[ei].src;
                  size_t dst = this->edge_list_[ei].dst;
                  __sync_fetch_and_add(&this->out_degree_[src], (size_t)1);
                  if (this->edge_direction_policy_ == MAKE_SYMMETRIC) {
                      __sync_fetch_and_add(&this->out_degree_[dst], (size_t)1);
                  }
              }

              memcpy(this->out_index_.Data() + 1, this->out_degree_.Data(),
                                                sizeof(size_t) * this->num_vertices_);
              this->out_index_[0] = 0;
              for (size_t vi = 0; vi < this->num_vertices_; vi++) {
                  this->out_index_[vi + 1] += this->out_index_[vi];
              }

#pragma omp parallel for
              for (size_t ei = 0; ei < this->num_edges_; ei++) {
                  size_t src = this->edge_list_[ei].src;
                  size_t dst = this->edge_list_[ei].dst;
                  size_t pos = __sync_fetch_and_add(&this->out_index_[src], (size_t)1);
                  this->out_edges_[pos].neighbour = dst;
                  if (this->edge_data_size_ != 0) {
                      this->out_edges_[pos].edge_data = this->edge_list_[ei].edge_data;
                  }

                  if (this->edge_direction_policy_ == MAKE_SYMMETRIC) {
                      pos = __sync_fetch_and_add(&this->out_index_[dst], (size_t)1);
                      this->out_edges_[pos].neighbour = src;
                      if (this->edge_data_size_ != 0) {
                          this->out_edges_[pos].edge_data = this->edge_list_[ei].edge_data;
                      }
                  }
              }

              memmove(this->out_index_.Data() + 1, this->out_index_.Data(),
                                                sizeof(size_t) * this->num_vertices_);
              this->out_index_[0] = 0;

              if (this->edge_direction_policy_ == MAKE_SYMMETRIC) {
                  this->num_edges_ *= 2;
              }
            });
        }
    }

 public:
    bool CheckKillThisTask() {return false;}

    size_t GetMaxVertexId() {
        auto worker = Worker::SharedWorker();
        size_t max_vertex_id = 0;
        worker->Delegate([&]() {
            thread_local size_t local_max_vertex_id = 0;
#pragma omp parallel
            {
                int thread_id = omp_get_thread_num();
                int num_threads = omp_get_num_threads();
                size_t thread_length = this->num_edges_ / num_threads;
                size_t start_index = thread_length * thread_id;
                size_t end_index = (thread_id == num_threads - 1) ?
                            this->num_edges_ : (start_index + thread_length);
                for (size_t ei = start_index; ei < end_index; ei++) {
                    local_max_vertex_id = std::max(local_max_vertex_id, this->edge_list_[ei].src);
                    local_max_vertex_id = std::max(local_max_vertex_id, this->edge_list_[ei].dst);
                }
            }
#pragma omp parallel
            {
#pragma omp critical
                max_vertex_id = std::max(max_vertex_id, local_max_vertex_id);
            };
        });

        return max_vertex_id;
    }

    /**
     * 
     * @brief   Load edge_list from txt file and construct graph.
     *
     * @param   config              The ConfigBase of graph.
     * @param   edge_direction_policy   Edge direction policy of graph.
     */
    void Load(ConfigBase<EdgeData> config,
                    EdgeDirectionPolicy edge_direction_policy = DUAL_DIRECTION) {
        if (this->num_vertices_ != 0 || this->num_edges_ != 0) {
            throw std::runtime_error("Graph should not be loaded twice!");
        }

        this->edge_direction_policy_ = edge_direction_policy;

        double cost = -get_time();
        if (config.GetType() == TEXT_FILE) {
            auto & path = config.input_dir;
            auto & parse_line = config.parse_line;
            TextFileReader<EdgeData> reader(path, parse_line);
            this->edge_list_ = reader.edge_list;
            this->num_edges_ = reader.num_edges;
            this->num_vertices_ = GetMaxVertexId() + 1;
            Construct();
        } else if (config.GetType() == BINARY_FILE) {
            auto & path = config.input_dir;
            BinaryFileReader<EdgeData> reader(path);
            this->edge_list_ = reader.edge_list;
            this->num_edges_ = reader.num_edges;
            this->num_vertices_ = GetMaxVertexId() + 1;
            Construct();
        }

        cost += get_time();
        std::printf("  single pass read cost: %.2lf(s)\n", cost);
    }

    /**
     * @brief   Load the vertex-data pairs from the txt file into an array in the order of vertex ids.
     * 
     * @param[in,out]   array       Array of data to read.
     * @param           prefix      The path of read file.
     * @param           parse_line  Text data parsing functions.
     * 
     */
    template <typename VertexData>
    void LoadVertexArrayTxt(std::vector<VertexData> &array, std::string prefix,
        std::function<std::tuple<size_t, bool>(const char *,
                            const char *, VertexData&)> parse_line) {
        double load_time = 0;
        load_time -= get_time();
        std::vector<VertexData> read_vertex_buffer;
        {
        fma_common::InputTextDirStream fin(prefix, 1, 0, CHUNKSIZE, 2);  // i.e. 8 threads
        fma_common::TextParser< VertexData, std::function<std::tuple<size_t, bool>(
                            const char *, const char *, VertexData &)> >
        parser(
            fin,
            parse_line,
            CHUNKSIZE,
            1);
        while (parser.ReadBlock(read_vertex_buffer)) {
            for (auto& ele : read_vertex_buffer) {
                array.push_back(ele);
                }
            }
        }
        load_time += get_time();
        printf("loading vertex data used %.2lf seconds\n", load_time);
    }

    /**
     * @brief   
     * 
     * @param           config          The ConfigBase of graph.
     * @param[in]       array           The array to be written.
     * @param           array_size      Array_size.
     * @param           name            Algorithm name.
     * @param[in,out]   filter_output   Write data rule function.
     * 
     */
    template <typename VertexData>
    void Write(ConfigBase<EdgeData> & config, ParallelVector<VertexData>& array,
                                        size_t array_size, std::string name,
             std::function<bool(VertexData &)> filter_output = filter_output_default<VertexData&>) {
        if (config.GetType() == TEXT_FILE) {
            if (config.output_dir == "") return;
            FileWriter<VertexData> fw(config.output_dir, array, array_size, name, filter_output);
        }
    }
};

}  // namespace olap
}  // namespace lgraph_api
