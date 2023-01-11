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

#include "fma-common/fma_stream.h"
#include "import/vid_table.h"

namespace lgraph {
namespace import_v2 {

class ImportDataFile {
 public:
    struct EdgeData {
        VidType vid1;
        LabelId lid;
        TemporalId tid;
        VidType vid2;
        DenseString prop;

        EdgeData() {}
        EdgeData(VidType v1, LabelId l, TemporalId t, VidType v2, std::string&& p)
            : vid1(v1), lid(l), tid(t), vid2(v2), prop(std::move(p)) {}

        bool operator<(const EdgeData& rhs) const {
            return vid1 < rhs.vid1 || (vid1 == rhs.vid1 && lid < rhs.lid) ||
                   (vid1 == rhs.vid1 && lid == rhs.lid && tid < rhs.tid) ||
                   (vid1 == rhs.vid1 && lid == rhs.lid && tid == rhs.tid && vid2 < rhs.vid2);
        }

        template <typename StreamT>
        size_t Serialize(StreamT& stream) const {
            return fma_common::BinaryWrite(stream, vid1) + fma_common::BinaryWrite(stream, lid) +
                   fma_common::BinaryWrite(stream, tid) + fma_common::BinaryWrite(stream, vid2) +
                   fma_common::BinaryWrite(stream, prop);
        }

        template <typename StreamT>
        size_t Deserialize(StreamT& stream) {
            return fma_common::BinaryRead(stream, vid1) + fma_common::BinaryRead(stream, lid) +
                   fma_common::BinaryRead(stream, tid) + fma_common::BinaryRead(stream, vid2) +
                   fma_common::BinaryRead(stream, prop);
        }

        std::string ToString() const {
            std::string rt = FMA_FMT("{}_{}_{}_{}", vid1, lid, tid, vid2);
            return rt;
        }
    };

    struct VertexDataWithVid {
        VertexDataWithVid(std::string&& data, VidType vid_) : data(std::move(data)), vid(vid_) {}
        VertexDataWithVid() {}

        DenseString data;
        VidType vid;

        size_t Serialize(fma_common::OutputFmaStream& stream) const {
            return fma_common::BinaryWrite(stream, vid) + fma_common::BinaryWrite(stream, data);
        }

        size_t Deserialize(fma_common::InputFmaStream& stream) {
            return fma_common::BinaryRead(stream, vid) + fma_common::BinaryRead(stream, data);
        }
    };

    struct BucketData {
        std::vector<VertexDataWithVid> vdata;
        std::deque<EdgeData> oes;
        std::deque<EdgeData> ins;
    };

    struct VertexSettings {
        size_t bucket_size;
        VidType start_vid;
        VidType end_vid;
    };

 protected:
    std::string dir_;
    size_t buf_size_;

    std::unordered_map<std::string, VertexSettings> vertex_settings_;
    fma_common::ThreadPool writers_;
    typedef fma_common::OutputBufferedFileStream<fma_common::UnbufferedOutputLocalFileStream,
                                                 fma_common::ThreadPoolOutputStreamBuffer>
        OEFS;

    std::string v_label_;
    size_t curr_bucket_{};
    VidType start_vid_{}, end_vid_{};
    size_t v_file_offset_{};
    fma_common::OutputFmaStream v_out_data_file_;
    std::unordered_map<std::string, std::vector<std::pair<VidType, size_t>>> v_file_offsets_;

    std::string src_label_;
    size_t src_bucket_size_{};
    VidType src_start_vid_{}, src_end_vid_{};
    std::string dst_label_;
    size_t dst_bucket_size_{};
    VidType dst_start_vid_{}, dst_end_vid_{};
    std::vector<OEFS> src_oe_files_;
    std::vector<OEFS> dst_ie_files_;

    // for read
    std::unique_ptr<fma_common::PipelineStage<size_t, BucketData>> bucket_reader_;
    std::unique_ptr<fma_common::PipelineStage<BucketData, BucketData>> bucket_sorter_;
    std::unique_ptr<fma_common::BoundedQueue<BucketData>> sorted_buckets_;
    size_t n_buckets_{};
    size_t next_push_bucket_{};

 public:
    // `1 << 20` in the constructor will trigger cpplint bug and throw an exception
    explicit ImportDataFile(const std::string& dir, size_t buf_size = 1024 * 1024)
        : buf_size_(buf_size), writers_(10) {
        auto& fs = fma_common::FileSystem::GetFileSystem(dir);
        dir_ = dir + fs.PathSeparater() + "_fma_tmp_";
        fs.RemoveDir(dir_);
        if (!fs.Mkdir(dir_)) {
            FMA_ERR() << "Failed to create directory [" << dir_ << "] for intermediate files";
        }
    }

    void StartWritingVertex(const std::string& label, VidType start_vid) {
        FMA_DBG_ASSERT(v_label_.empty());
        FMA_DBG_ASSERT(vertex_settings_.find(label) == vertex_settings_.end());
        v_label_ = label;
        start_vid_ = start_vid;
        v_file_offset_ = 0;
        vertex_settings_[label].start_vid = start_vid;
        v_file_offsets_[label].emplace_back(start_vid, 0);
        v_out_data_file_.Open(GetVDataFileName(v_label_), buf_size_, std::ios_base::app);
        if (!v_out_data_file_.Good()) {
            FMA_ERR() << "Error opening file [" << v_out_data_file_.Path() << "] for write";
        }
    }

    void WriteVertexes(const std::vector<VertexDataWithVid>& vs) {
        for (auto& v : vs) {
            if (v.vid % 1024 == 0) {
                v_file_offsets_[v_label_].emplace_back(v.vid, v_file_offset_);
            }
            v_file_offset_ += fma_common::BinaryWrite(v_out_data_file_, v);
        }
    }

    void EndWritingVertexAndSetBucketSize(const std::string& label, VidType end_vid,
                                          size_t bucket_size) {
        vertex_settings_[label].end_vid = end_vid;
        vertex_settings_[label].bucket_size = std::max<size_t>(1, bucket_size);
        v_out_data_file_.Close();
        v_label_.clear();
    }

    void StartWritingEdge(const std::string& src_label, const std::string& dst_label) {
        FMA_DBG_ASSERT(vertex_settings_.find(src_label) != vertex_settings_.end());
        src_label_ = src_label;
        auto& src_setting = vertex_settings_[src_label];
        src_bucket_size_ = src_setting.bucket_size;
        src_start_vid_ = src_setting.start_vid;
        src_end_vid_ = src_setting.end_vid;
        src_oe_files_.resize(GetNBuckets(src_start_vid_, src_end_vid_, src_setting.bucket_size));
        for (size_t i = 0; i < src_oe_files_.size(); i++) {
            src_oe_files_[i].Open(GetOutEdgeFileName(src_label, i), buf_size_, std::ios_base::app,
                                  std::make_tuple(), std::make_tuple(&writers_));
            if (!src_oe_files_[i].Good()) {
                FMA_ERR() << "Error opening intermediate file [" << src_oe_files_[i].Path()
                          << "] for write";
            }
        }

        FMA_DBG_ASSERT(vertex_settings_.find(dst_label) != vertex_settings_.end());
        dst_label_ = dst_label;
        auto& dst_setting = vertex_settings_[dst_label];
        dst_bucket_size_ = dst_setting.bucket_size;
        dst_start_vid_ = dst_setting.start_vid;
        dst_end_vid_ = dst_setting.end_vid;
        dst_ie_files_.resize(GetNBuckets(dst_start_vid_, dst_end_vid_, dst_setting.bucket_size));
        for (size_t i = 0; i < dst_ie_files_.size(); i++) {
            dst_ie_files_[i].Open(GetInRefFileName(dst_label, i), buf_size_, std::ios_base::app,
                                  std::make_tuple(), std::make_tuple(&writers_));
            if (!dst_ie_files_[i].Good()) {
                FMA_ERR() << "Error opening intermediate file [" << dst_ie_files_[i].Path()
                          << "] for write";
            }
        }
    }

    void WriteEdges(const std::vector<EdgeData>& outs, const std::vector<EdgeData>& ins) {
        for (auto& e : outs) {
            FMA_DBG_ASSERT(e.vid1 >= src_start_vid_ && e.vid1 < src_end_vid_ &&
                           e.vid2 >= dst_start_vid_ && e.vid2 < dst_end_vid_);
            size_t bid = GetBucketId(e.vid1, src_start_vid_, src_bucket_size_);
            auto& stream = src_oe_files_[bid];
            fma_common::BinaryWrite(stream, e);
        }
        for (auto& e : ins) {
            FMA_DBG_ASSERT(e.vid2 >= src_start_vid_ && e.vid2 < src_end_vid_ &&
                           e.vid1 >= dst_start_vid_ && e.vid1 < dst_end_vid_);
            size_t bid = GetBucketId(e.vid1, dst_start_vid_, dst_bucket_size_);
            auto& stream = dst_ie_files_[bid];
            fma_common::BinaryWrite(stream, e);
        }
    }

    void EndWritingEdge() {
        src_oe_files_.clear();
        dst_ie_files_.clear();
    }

    void StartReading(const std::string& vlabel, VidType start_vid, VidType end_vid,
                      size_t n_readers, size_t n_sorters) {
        FMA_DBG_ASSERT(vertex_settings_.find(vlabel) != vertex_settings_.end());
        size_t bucket_size = vertex_settings_[vlabel].bucket_size;
        n_buckets_ = GetNBuckets(start_vid, end_vid, bucket_size);
        curr_bucket_ = 0;
        v_label_ = vlabel;
        sorted_buckets_.reset(new fma_common::BoundedQueue<BucketData>(1));
        bucket_sorter_.reset(new fma_common::PipelineStage<BucketData, BucketData>(
            [this](BucketData&& data) -> BucketData {
                if (!data.vdata.empty()) FMA_DBG() << "start sorting " << data.vdata.front().vid;
                // LGRAPH_PSORT(data.ins.begin(), data.ins.end());
                // LGRAPH_PSORT(data.oes.begin(), data.oes.end());
                if (!std::is_sorted(data.ins.begin(), data.ins.end()))
                    std::stable_sort(data.ins.begin(), data.ins.end());
                if (!std::is_sorted(data.oes.begin(), data.oes.end()))
                    std::stable_sort(data.oes.begin(), data.oes.end());
                if (!data.vdata.empty()) FMA_DBG() << "end sorting " << data.vdata.front().vid;
                return std::move(data);
            },
            nullptr, 0, n_sorters, n_sorters, false, sorted_buckets_.get()));
        size_t ibuf_size = 1 << 20;
        bucket_reader_.reset(new fma_common::PipelineStage<size_t, BucketData>(
            [this, ibuf_size, bucket_size, start_vid, end_vid](size_t bucket_id) -> BucketData {
                BucketData ret;
                if (bucket_id >= n_buckets_) return ret;
                FMA_DBG() << "start reading " << bucket_id;
                VidType first_vid = start_vid + bucket_id * bucket_size;
                VidType last_vid = std::min(first_vid + bucket_size, end_vid);
                fma_common::InputFmaStream vf(GetVDataFileName(v_label_), ibuf_size);
                {
                    // seek to a close position where vid is smaller than first_vid
                    auto vfit = v_file_offsets_.find(v_label_);
                    FMA_ASSERT(vfit != v_file_offsets_.end())
                        << "vlabel: " << v_label_
                        << ", vfoffsets: " << fma_common::ToString(v_file_offsets_);
                    auto& offsets = vfit->second;
                    if (!offsets.empty()) {
                        auto it = std::upper_bound(offsets.begin(), offsets.end(),
                                                   std::make_pair(first_vid, (size_t)0));
                        if (it == offsets.end()) {
                            vf.Seek(offsets.back().second);
                        } else if (it != offsets.begin()) {
                            it--;
                            vf.Seek(it->second);
                        }
                        // if it==offsets.begin(), read from start
                    }
                }
                fma_common::InputFmaStream oef(GetOutEdgeFileName(v_label_, bucket_id), ibuf_size);
                fma_common::InputFmaStream ief(GetInRefFileName(v_label_, bucket_id), ibuf_size);
                while (vf.Good()) {
                    VertexDataWithVid v;
                    if (fma_common::BinaryRead(vf, v) == 0) break;
                    if (v.vid < first_vid) continue;
                    if (v.vid >= last_vid) break;
                    ret.vdata.emplace_back(std::move(v));
                }
                while (oef.Good()) {
                    EdgeData e;
                    if (fma_common::BinaryRead(oef, e) == 0) break;
                    ret.oes.emplace_back(std::move(e));
                }
                while (ief.Good()) {
                    EdgeData e;
                    if (fma_common::BinaryRead(ief, e) == 0) break;
                    ret.ins.emplace_back(std::move(e));
                }
                FMA_DBG() << "end reading " << bucket_id;
                return ret;
            },
            nullptr, 0, n_readers, n_buckets_, false, bucket_sorter_.get()));
        for (size_t i = 0; i < n_buckets_; i++) bucket_reader_->Push(i);
    }

    bool ReadBucket(BucketData& bucket) {
        if (curr_bucket_++ >= n_buckets_) return false;
        bucket.ins.clear();
        bucket.oes.clear();
        bucket.vdata.clear();
        return sorted_buckets_->Pop(bucket);
    }

    void EndReadingVertex() {
        bucket_reader_.reset();
        bucket_sorter_.reset();
        sorted_buckets_.reset();
        v_label_.clear();
    }

    void CleanTempFiles() {
        auto& fs = fma_common::FileSystem::GetFileSystem(dir_);
        fs.RemoveDir(dir_);
    }

    /**
     * Gets number of buckets for current vertex label. Should be used only after StartReading
     *
     * @return  The n buckets.
     */
    size_t GetNBuckets() const { return n_buckets_; }

 protected:
    size_t GetNBuckets(VidType start_vid, VidType end_vid, size_t bucket_size) {
        return (end_vid - start_vid + bucket_size - 1) / bucket_size;
    }

    size_t GetBucketId(VidType vid, VidType start_vid, size_t bucket_size) {
        return (vid - start_vid) / bucket_size;
    }

    std::string GetDataFilePrefix(const std::string& label, size_t bucket_id) {
        return fma_common::StringFormatter::Format(
            "{}{}{}.e.{}", dir_, fma_common::FileSystem::GetFileSystem(dir_).PathSeparater(), label,
            bucket_id);
    }

    std::string GetVDataFileName(const std::string& label) {
        return fma_common::StringFormatter::Format(
            "{}{}{}.vdata", dir_, fma_common::FileSystem::GetFileSystem(dir_).PathSeparater(),
            label);
    }

    std::string GetVOffsetFileName(const std::string& label) {
        return fma_common::StringFormatter::Format(
            "{}{}{}.voff", dir_, fma_common::FileSystem::GetFileSystem(dir_).PathSeparater(),
            label);
    }

    std::string GetOutEdgeFileName(const std::string& label, size_t bucket_id) {
        return GetDataFilePrefix(label, bucket_id) + ".out";
    }

    std::string GetInRefFileName(const std::string& label, size_t bucket_id) {
        return GetDataFilePrefix(label, bucket_id) + ".in";
    }
};

}  // namespace import_v2
}  // namespace lgraph
