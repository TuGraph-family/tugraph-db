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

#include <memory>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace cypher {

constexpr uint64_t BITMASKS_SINGLE_ONE[64] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000, 0x800000000,
    0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000, 0x10000000000, 0x20000000000,
    0x40000000000, 0x80000000000, 0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
    0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000,
    0x20000000000000, 0x40000000000000, 0x80000000000000, 0x100000000000000, 0x200000000000000,
    0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000,
    0x4000000000000000, 0x8000000000000000};
constexpr uint64_t BITMASKS_SINGLE_ZERO[64] = {0xfffffffffffffffe, 0xfffffffffffffffd,
    0xfffffffffffffffb, 0xfffffffffffffff7, 0xffffffffffffffef, 0xffffffffffffffdf,
    0xffffffffffffffbf, 0xffffffffffffff7f, 0xfffffffffffffeff, 0xfffffffffffffdff,
    0xfffffffffffffbff, 0xfffffffffffff7ff, 0xffffffffffffefff, 0xffffffffffffdfff,
    0xffffffffffffbfff, 0xffffffffffff7fff, 0xfffffffffffeffff, 0xfffffffffffdffff,
    0xfffffffffffbffff, 0xfffffffffff7ffff, 0xffffffffffefffff, 0xffffffffffdfffff,
    0xffffffffffbfffff, 0xffffffffff7fffff, 0xfffffffffeffffff, 0xfffffffffdffffff,
    0xfffffffffbffffff, 0xfffffffff7ffffff, 0xffffffffefffffff, 0xffffffffdfffffff,
    0xffffffffbfffffff, 0xffffffff7fffffff, 0xfffffffeffffffff, 0xfffffffdffffffff,
    0xfffffffbffffffff, 0xfffffff7ffffffff, 0xffffffefffffffff, 0xffffffdfffffffff,
    0xffffffbfffffffff, 0xffffff7fffffffff, 0xfffffeffffffffff, 0xfffffdffffffffff,
    0xfffffbffffffffff, 0xfffff7ffffffffff, 0xffffefffffffffff, 0xffffdfffffffffff,
    0xffffbfffffffffff, 0xffff7fffffffffff, 0xfffeffffffffffff, 0xfffdffffffffffff,
    0xfffbffffffffffff, 0xfff7ffffffffffff, 0xffefffffffffffff, 0xffdfffffffffffff,
    0xffbfffffffffffff, 0xff7fffffffffffff, 0xfeffffffffffffff, 0xfdffffffffffffff,
    0xfbffffffffffffff, 0xf7ffffffffffffff, 0xefffffffffffffff, 0xdfffffffffffffff,
    0xbfffffffffffffff, 0x7fffffffffffffff};
constexpr uint64_t LOWER_BITMASKS[65] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff,
    0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
    0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff,
    0x3fffffff, 0x7fffffff, 0xffffffff, 0x1ffffffff, 0x3ffffffff, 0x7ffffffff, 0xfffffffff,
    0x1fffffffff, 0x3fffffffff, 0x7fffffffff, 0xffffffffff, 0x1ffffffffff, 0x3ffffffffff,
    0x7ffffffffff, 0xfffffffffff, 0x1fffffffffff, 0x3fffffffffff, 0x7fffffffffff, 0xffffffffffff,
    0x1ffffffffffff, 0x3ffffffffffff, 0x7ffffffffffff, 0xfffffffffffff, 0x1fffffffffffff,
    0x3fffffffffffff, 0x7fffffffffffff, 0xffffffffffffff, 0x1ffffffffffffff, 0x3ffffffffffffff,
    0x7ffffffffffffff, 0xfffffffffffffff, 0x1fffffffffffffff, 0x3fffffffffffffff,
    0x7fffffffffffffff, 0xffffffffffffffff};
constexpr uint64_t HIGH_BITMASKS[65] = {0x0, 0x8000000000000000, 0xc000000000000000,
    0xe000000000000000, 0xf000000000000000, 0xf800000000000000, 0xfc00000000000000,
    0xfe00000000000000, 0xff00000000000000, 0xff80000000000000, 0xffc0000000000000,
    0xffe0000000000000, 0xfff0000000000000, 0xfff8000000000000, 0xfffc000000000000,
    0xfffe000000000000, 0xffff000000000000, 0xffff800000000000, 0xffffc00000000000,
    0xffffe00000000000, 0xfffff00000000000, 0xfffff80000000000, 0xfffffc0000000000,
    0xfffffe0000000000, 0xffffff0000000000, 0xffffff8000000000, 0xffffffc000000000,
    0xffffffe000000000, 0xfffffff000000000, 0xfffffff800000000, 0xfffffffc00000000,
    0xfffffffe00000000, 0xffffffff00000000, 0xffffffff80000000, 0xffffffffc0000000,
    0xffffffffe0000000, 0xfffffffff0000000, 0xfffffffff8000000, 0xfffffffffc000000,
    0xfffffffffe000000, 0xffffffffff000000, 0xffffffffff800000, 0xffffffffffc00000,
    0xffffffffffe00000, 0xfffffffffff00000, 0xfffffffffff80000, 0xfffffffffffc0000,
    0xfffffffffffe0000, 0xffffffffffff0000, 0xffffffffffff8000, 0xffffffffffffc000,
    0xffffffffffffe000, 0xfffffffffffff000, 0xfffffffffffff800, 0xfffffffffffffc00,
    0xfffffffffffffe00, 0xffffffffffffff00, 0xffffffffffffff80, 0xffffffffffffffc0,
    0xffffffffffffffe0, 0xfffffffffffffff0, 0xfffffffffffffff8, 0xfffffffffffffffc,
    0xfffffffffffffffe, 0xffffffffffffffff};

class BitMask {
 public:
    static constexpr uint64_t NO_NULL_ENTRY = 0;
    static constexpr uint64_t ALL_NULL_ENTRY = ~uint64_t(NO_NULL_ENTRY);
    static constexpr uint64_t BITS_PER_ENTRY_LOG2 = 6;  // 64 bits per entry
    static constexpr uint64_t BITS_PER_ENTRY = (uint64_t)1 << BITS_PER_ENTRY_LOG2;
    static constexpr uint64_t BYTES_PER_ENTRY = BITS_PER_ENTRY >> 3;  // 8 bytes per entry

    explicit BitMask(uint64_t capacity) : may_contain_nulls_{false} {
        auto num_null_entries = (capacity + BITS_PER_ENTRY - 1) / BITS_PER_ENTRY;
        buffer_ = std::make_unique<uint64_t[]>(num_null_entries);
        data_ = buffer_.get();
        size_ = num_null_entries;
        std::fill(data_, data_ + num_null_entries, NO_NULL_ENTRY);
    }

    explicit BitMask(uint64_t* mask_data, size_t size, bool may_contain_nulls)
        : data_{mask_data}, size_{size}, buffer_{nullptr}, may_contain_nulls_{may_contain_nulls} {}
        BitMask(const BitMask& other) {
        size_ = other.size_;
        may_contain_nulls_ = other.may_contain_nulls_;
        buffer_ = std::make_unique<uint64_t[]>(size_);
        std::copy(other.data_, other.data_ + size_, buffer_.get());
        data_ = buffer_.get();
    }

    BitMask& operator=(const BitMask& other) {
        if (this == &other) return *this;
        size_ = other.size_;
        may_contain_nulls_ = other.may_contain_nulls_;
        buffer_ = std::make_unique<uint64_t[]>(size_);
        std::copy(other.data_, other.data_ + size_, buffer_.get());
        data_ = buffer_.get();
        return *this;
    }

    void SetAllNonNull() {
        if (!may_contain_nulls_) return;
        std::fill(data_, data_ + size_, NO_NULL_ENTRY);
        may_contain_nulls_ = false;
    }

    void SetAllNull() {
        std::fill(data_, data_ + size_, ALL_NULL_ENTRY);
        may_contain_nulls_ = true;
    }

    bool HasNoNullsGuarantee() const { return !may_contain_nulls_; }

    static void SetBit(uint64_t* entries, uint32_t pos, bool is_null) {
        auto [entry_pos, bit_pos_in_entry] = GetEntryAndBitPos(pos);
        if (is_null) {
            entries[entry_pos] |= BITMASKS_SINGLE_ONE[bit_pos_in_entry];
        } else {
            entries[entry_pos] &= BITMASKS_SINGLE_ZERO[bit_pos_in_entry];
        }
    }

    void SetBit(uint32_t pos, bool is_null) {
        SetBit(data_, pos, is_null);
        if (is_null) {
            may_contain_nulls_ = true;
        }
    }

    bool IsBitSet(uint32_t pos) const {
        auto [entry_pos, bit_pos_in_entry] = GetEntryAndBitPos(pos);
        return data_[entry_pos] & BITMASKS_SINGLE_ONE[bit_pos_in_entry];
    }

    const uint64_t* GetData() const { return data_; }

    static uint64_t GetNumEntries(uint64_t num_bits) {
        return (num_bits >> BITS_PER_ENTRY_LOG2) +
               ((num_bits - (num_bits << BITS_PER_ENTRY_LOG2)) == 0 ? 0 : 1);
    }

    void resize(uint64_t capacity) {
        auto num_entries = (capacity + BITS_PER_ENTRY - 1) / BITS_PER_ENTRY;
        auto resized_buffer = std::make_unique<uint64_t[]>(num_entries);
        if (data_) {
            std::memcpy(resized_buffer.get(), data_,
                std::min(size_, num_entries) * sizeof(uint64_t));
        }
        buffer_ = std::move(resized_buffer);
        data_ = buffer_.get();
        size_ = num_entries;
    }

    void SetNullFromRange(uint64_t offset, uint64_t num_bits_to_set, bool is_null) {
        if (is_null) {
            may_contain_nulls_ = true;
        }
        SetNullRange(data_, offset, num_bits_to_set, is_null);
    }

    static void SetNullRange(uint64_t* null_entries, uint64_t offset,
                             uint64_t num_bits_to_set, bool is_null) {
        auto [first_entry_pos, first_bit_pos] = GetEntryAndBitPos(offset);
        auto [last_entry_pos, last_bit_pos] = GetEntryAndBitPos(offset + num_bits_to_set);

        if (last_entry_pos > first_entry_pos + 1) {
            std::fill(null_entries + first_entry_pos + 1, null_entries + last_entry_pos,
                      is_null ? ALL_NULL_ENTRY : NO_NULL_ENTRY);
        }

        if (first_entry_pos == last_entry_pos) {
            if (is_null) {
                null_entries[first_entry_pos] |= (~LOWER_BITMASKS[first_bit_pos]
                                & ~HIGH_BITMASKS[BITS_PER_ENTRY - last_bit_pos]);
            } else {
                null_entries[first_entry_pos] &= (LOWER_BITMASKS[first_bit_pos]
                                | HIGH_BITMASKS[BITS_PER_ENTRY - last_bit_pos]);
            }
        } else {
            if (is_null) {
                null_entries[first_entry_pos] |= ~LOWER_BITMASKS[first_bit_pos];
                if (last_bit_pos > 0) {
                    null_entries[last_entry_pos] |= LOWER_BITMASKS[last_bit_pos];
                }
            } else {
                null_entries[first_entry_pos] &= LOWER_BITMASKS[first_bit_pos];
                if (last_bit_pos > 0) {
                    null_entries[last_entry_pos] &= ~LOWER_BITMASKS[last_bit_pos];
                }
            }
        }
    }

 private:
    static std::pair<uint64_t, uint64_t> GetEntryAndBitPos(uint64_t pos) {
        auto entry_pos = pos >> BITS_PER_ENTRY_LOG2;
        return {entry_pos, pos - (entry_pos << BITS_PER_ENTRY_LOG2)};
    }

 private:
    uint64_t* data_;
    size_t size_;
    std::unique_ptr<uint64_t[]> buffer_;
    bool may_contain_nulls_;
};

}  // namespace cypher
