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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "olap/olap_config.h"

static constexpr size_t CHUNKSIZE = 1048576*64;

namespace lgraph_api {
namespace olap {

template <typename VertexData>
bool filter_output_default(VertexData & val) {
    return true;
}

template <typename EdgeData>
class BlockReader {
 public:
    virtual void Reset() = 0;
    virtual bool ReadBlock(std::vector<EdgeUnit<EdgeData>>& data) = 0;
};

template <typename EdgeData>
class TextFileReader: public BlockReader<EdgeData> {
 public:
    TextFileReader(
            std::string path,
            std::function<std::tuple<size_t, bool>(const char *,
                    const char *, EdgeUnit<EdgeData> &)> parse_line) {
        double cost = - get_time();
        edge_unit_size = sizeof(size_t) * 2 +
                (std::is_same<EdgeData, Empty>::value ? 0 : sizeof(EdgeData));
        assert(edge_unit_size == sizeof(EdgeUnit<EdgeData>));

        edge_list = (EdgeUnit<EdgeData> *)alloc_buffer(edge_unit_size * MAX_NUM_EDGES);
        num_edges = 0;
        if (fma_common::FileSystem::GetFileSystem(path).IsDir(path))
            InitFromDir(path, parse_line);
        else
            InitFromFile(path, parse_line);
        cost += get_time();
        printf("  text read cost: %.2lf(s)\n", cost);
        this->Reset();
    }

    void InitFromDir(
            std::string path,
            std::function<std::tuple<size_t, bool>(const char *, const char *,
                            EdgeUnit<EdgeData> &)> parse_line) {
        auto &fs = fma_common::FileSystem::GetFileSystem(path);
        std::vector<size_t> *sizes = nullptr;
        auto file_splits = fs.ListFiles(path, sizes);
        std::sort(file_splits.begin(), file_splits.end());
        int splits = file_splits.size();

        int global_split_id = 0;
#pragma omp parallel
        {
            std::vector<EdgeUnit<EdgeData>> read_edge_buffer;
            while (true) {
                int split_id = __sync_fetch_and_add(&global_split_id, 1);
                if (split_id >= splits)
                    break;
                fma_common::InputFmaStream fin(file_splits[split_id]);
                fma_common::TextParser<EdgeUnit<EdgeData>, decltype(parse_line)> parser(
                        fin,
                        parse_line,
                        CHUNKSIZE,
                        1);
                while (parser.ReadBlock(read_edge_buffer)) {
                    size_t pos = __sync_fetch_and_add(&num_edges, read_edge_buffer.size());
                    memcpy(edge_list + pos, read_edge_buffer.data(),
                            edge_unit_size * read_edge_buffer.size());
                }
            }
        }
    }

    void InitFromFile(
            std::string file,
            std::function<std::tuple<size_t, bool>(const char *, const char *,
                            EdgeUnit<EdgeData> &)> parse_line) {
        fma_common::InputFmaStream fin(file);
        fma_common::TextParser<EdgeUnit<EdgeData>, decltype(parse_line)> parser(
                fin,
                parse_line,
                CHUNKSIZE,
                1);
        std::vector<EdgeUnit<EdgeData>> read_edge_buffer;
        while (parser.ReadBlock(read_edge_buffer)) {
            size_t pos = __sync_fetch_and_add(&num_edges, read_edge_buffer.size());
            memcpy(edge_list + pos, read_edge_buffer.data(),
                    edge_unit_size * read_edge_buffer.size());
        }
    }

    void Reset() {
        idx = 0;
    }

    bool ReadBlock(std::vector<EdgeUnit<EdgeData>>& data) {
        size_t begin = __sync_fetch_and_add(&idx, block_size);
        size_t end = begin + block_size < num_edges ? begin + block_size : num_edges;

        if (begin >= num_edges) {
            data.clear();
            return false;
        }

        if (data.size() != end - begin) {
            data.resize(end - begin);
        }

        for (size_t i=begin; i < end; ++i)
            memcpy(&data[i-begin], &edge_list[i], edge_unit_size);
        // printf("read [%lu, %lu  %d %d\n", begin, end,
        //        edge_list[begin].src, edge_list[begin].dst);
        return true;
    }

    ~TextFileReader() {
        dealloc_buffer(edge_list, edge_unit_size * MAX_NUM_EDGES);
        num_edges = 0;
    }

    EdgeUnit<EdgeData> *edge_list;
    size_t num_edges;

 private:
    size_t block_size = (1L<<20);
    size_t edge_unit_size;
    size_t idx;
};

template <typename EdgeData>
class BinaryFileReader : public BlockReader<EdgeData> {
 public:
    explicit BinaryFileReader(std::string path) {
        double cost = -get_time();
        edge_unit_size = sizeof(size_t) * 2 +
            (std::is_same<EdgeData, Empty>::value ? 0 : sizeof(EdgeData));
        assert(edge_unit_size == sizeof(EdgeUnit<EdgeData>));


        edge_list = (EdgeUnit<EdgeData>*)alloc_buffer(edge_unit_size * MAX_NUM_EDGES);
        if (fma_common::FileSystem::GetFileSystem(path).IsDir(path))
            InitFromDir(path);
        else
            InitFromFile(path);
        cost += get_time();
        printf("  binary read cost: %.2lf(s)\n", cost);
        this->Reset();
    }

    void InitFromDir(std::string path) {
        auto &fs = fma_common::FileSystem::GetFileSystem(path);
        std::vector<size_t> * sizes = new std::vector<size_t>;
        auto file_splits = fs.ListFiles(path, sizes);
        int splits = file_splits.size();

        size_t total_bytes = 0;
        std::vector<size_t> file_offset;
        file_offset.reserve(file_splits.size());
        for (auto ele : *sizes) {
            assert(ele % edge_unit_size == 0);
            file_offset.push_back(total_bytes);
            total_bytes += ele;
        }

        num_edges = total_bytes / edge_unit_size;

        int global_split_id = 0;
        size_t total_read_bytes = 0;
#pragma omp parallel
        {
            while (true) {
                int split_id = __sync_fetch_and_add(&global_split_id, 1);
                if (split_id >= splits)
                    break;
                fma_common::InputFmaStream fin(file_splits[split_id], CHUNKSIZE, false);

                size_t file_size = fin.Size();
                size_t read_bytes = 0;
                while (read_bytes < file_size) {
                    read_bytes += fin.Read(edge_list +
                        file_offset[split_id] + read_bytes, file_size - read_bytes);
                }
                fin.Close();
                assert(read_bytes == file_size);
                __sync_fetch_and_add(&total_read_bytes, read_bytes);
            }
        };
        assert(total_read_bytes == total_bytes);
    }

    void InitFromFile(std::string path) {
        fma_common::InputFmaStream fin(path, CHUNKSIZE, false);

        size_t file_size = fin.Size();
        size_t read_bytes = 0;
        while (read_bytes < file_size) {
            read_bytes += fin.Read(edge_list + read_bytes, file_size - read_bytes);
        }
        fin.Close();
        assert(read_bytes == file_size);
        num_edges = read_bytes / edge_unit_size;
    }

    void Reset() {
        idx = 0;
    }

    bool ReadBlock(std::vector<EdgeUnit<EdgeData>> & data) {
        size_t begin = __sync_fetch_and_add(&idx, block_size);
        size_t end = begin + block_size < num_edges ? begin + block_size : num_edges;

        if (begin >= num_edges) {
            data.clear();
            return false;
        }

        if (data.size() != end - begin) {
            data.resize(end - begin);
        }

        for (size_t i=begin; i < end; ++i)
            memcpy(&data[i-begin], &edge_list[i], edge_unit_size);
        // printf("read [%lu, %lu  %d %d\n", begin, end,
        //        edge_list[begin].src, edge_list[begin].dst);
        return true;
    }

    ~BinaryFileReader() {
        dealloc_buffer(edge_list, edge_unit_size * num_edges);
        num_edges = 0;
    }

    EdgeUnit<EdgeData> * edge_list;
    size_t num_edges;

 private:
    size_t block_size = (1L<<20);
    size_t edge_unit_size;
    size_t idx;
};

template <typename ArrayData>
class ArrayReader {
 public:
    virtual size_t Size() = 0;
    virtual ArrayData Get(size_t pos) = 0;
};

template <typename ArrayData>
class ArrayTextReader : public ArrayReader<ArrayData> {
 public:
    ArrayTextReader(
            std::string path,
            std::function<std::tuple<size_t, bool>(const char *, const char *,
                        ArrayData &)> parse_line) {
        auto &fs = fma_common::FileSystem::GetFileSystem(path);
        std::vector<size_t> *sizes = nullptr;
        auto file_splits = fs.ListFiles(path, sizes);
        std::sort(file_splits.begin(), file_splits.end());
        int splits = file_splits.size();
        ArrayData* array_data = (ArrayData *)alloc_buffer(sizeof(ArrayData) * MAX_NUM_EDGES);
        size_t array_size = 0;
        int global_split_id = 0;
        double cost = - get_time();
#pragma omp parallel
        {
            std::vector<ArrayData> read_buffer;
            while (true) {
                int split_id = __sync_fetch_and_add(&global_split_id, 1);
                if (split_id >= splits)
                    break;
                fma_common::InputFmaStream fin(file_splits[split_id]);
                fma_common::TextParser<ArrayData, decltype(parse_line)> parser(
                        fin,
                        parse_line,
                        CHUNKSIZE,
                        1);
                while (parser.ReadBlock(read_buffer)) {
                    size_t pos = __sync_fetch_and_add(&array_size, read_buffer.size());
                    memcpy(array_data + pos, read_buffer.data(),
                                sizeof(ArrayData) * read_buffer.size());
                }
            }
        }
        array.resize(array_size);
        memcpy(array.data(), array_data, sizeof(ArrayData) * array_size);
        dealloc_buffer(array_data, sizeof(ArrayData) * array_size);
        cost += get_time();
        printf("  single pass read cost: %.2lf(s)\n", cost);
    }

    ~ArrayTextReader() {
        array.clear();
    }

    size_t Size() {
        return array.size();
    }

    ArrayData Get(size_t pos) {
        return array[pos];
    }

    /* will clear array */
    void FetchAll(std::vector<ArrayData>& data) {
        data.swap(array);
    }

 private:
    std::vector<ArrayData> array;
};

template <typename ArrayData>
class ArrayBinaryReader : public ArrayReader<ArrayData> {
    std::vector<ArrayData> array;

 public:
    explicit ArrayBinaryReader(std::string path) {
        size_t arraydata_size = sizeof(ArrayData);
        ArrayData* array_data = (ArrayData*)alloc_buffer(arraydata_size * MAX_NUM_EDGES);

        auto &fs = fma_common::FileSystem::GetFileSystem(path);
        std::vector<size_t> * sizes = new std::vector<size_t>;
        auto file_splits = fs.ListFiles(path, sizes);
        int splits = file_splits.size();

        size_t total_bytes = 0;
        std::vector<size_t> file_offset;
        file_offset.reserve(file_splits.size());
        for (auto ele : *sizes) {
            assert(ele % arraydata_size == 0);
            file_offset.push_back(total_bytes);
            total_bytes += ele;
        }

        int global_split_id = 0;
        double cost = -get_time();
        size_t total_read_bytes = 0;
#pragma omp parallel
        {
            while (true) {
                int split_id = __sync_fetch_and_add(&global_split_id, 1);
                if (split_id >= splits)
                    break;
                fma_common::InputFmaStream fin(file_splits[split_id], CHUNKSIZE, false);

                size_t file_size = fin.Size();
                size_t read_bytes = 0;
                while (read_bytes < file_size) {
                    read_bytes += fin.Read(array_data + file_offset[split_id]
                                    + read_bytes, file_size - read_bytes);
                }
                fin.Close();
                assert(read_bytes == file_size);
                __sync_fetch_and_add(&total_read_bytes, read_bytes);
            }
        };
        assert(total_read_bytes == total_bytes);
        cost += get_time();

        array.resize(total_bytes / arraydata_size);
        memcpy(array.data(), array_data, total_bytes);
        dealloc_buffer(array_data, total_bytes);
        cost += get_time();
        printf("  binary files read cost: %.2lf(s)\n", cost);
    }

    size_t Size() {
        return array.size();
    }

    ArrayData Get(size_t pos) {
        return array[pos];
    }

    ~ArrayBinaryReader() {
        array.clear();
    }

    /* will clear array */
    void FetchAll(std::vector<ArrayData>& data) {
        data.swap(array);
    }
};

template <typename VertexData>
void FillVertexArray(std::vector<std::pair<size_t, VertexData>>& array, VertexData* vd) {
    for (size_t i=0; i < array.size(); ++i) {
        vd[array[i].first] = array[i].second;
    }
}

template <typename VertexData>
class FileWriter {
 public:
    FileWriter(std::string& path, ParallelVector<VertexData>& array, int array_size,
            std::string& name = "app", std::function<bool(VertexData &)> filter_output =
                            filter_output_default<VertexData>) {
#pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            int num_threads = omp_get_num_threads();

            size_t chunk_size = array_size / num_threads;
            size_t begin = chunk_size * thread_id;
            size_t end = (thread_id == num_threads - 1)
                                ? array_size : (chunk_size * (thread_id + 1));

            std::string file_name = fma_common::StringFormatter::Format(
                                        "{}/{}-{}.txt", path, name, thread_id);
            fma_common::OutputFmaStream fout;
            fout.Open(file_name, 64 << 20);
            for (size_t i = begin; i < end; ++i) {
                if (filter_output(array[i])) {
                    std::string line = fma_common::StringFormatter::Format(
                                                    "{} {}\n", i, array[i]);
                    fout.Write(line.c_str(), line.size());
                }
            }
        }
    }
};

}  // namespace olap
}  // namespace lgraph_api
