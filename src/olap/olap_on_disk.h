/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include "olap/olap_io.h"

namespace lgraph_api {
namespace olap {

template <typename EdgeData>
class OlapOnDisk : public OlapBase<EdgeData> {
 protected:
    void Construct() {
        OlapBase<EdgeData>::Construct();
    }

 public:
    std::vector<std::string> mapped_to_origin_;
    cuckoohash_map<std::string, size_t> hash_list_;
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
            if (config.id_mapping) {
                auto & parse_line = config.parse_string_line;
                StringTextFileReader<EdgeData> reader(path, parse_line);
                this->edge_list_ = reader.edge_list;
                this->num_edges_ = reader.num_edges;
                this->num_vertices_ = reader.num_vertices;
                this->hash_list_.swap(reader.hash_list);
                this->mapped_to_origin_.swap(reader.mapped_to_origin);
                Construct();
            } else {
                auto & parse_line = config.parse_line;
                TextFileReader<EdgeData> reader(path, parse_line);
                this->edge_list_ = reader.edge_list;
                this->num_edges_ = reader.num_edges;
                this->num_vertices_ = GetMaxVertexId() + 1;
                Construct();
            }
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
            FileWriter<VertexData> fw(config.output_dir, array, array_size,
                      mapped_to_origin_, name, config.id_mapping, filter_output);
        }
    }
};

}  // namespace olap
}  // namespace lgraph_api
