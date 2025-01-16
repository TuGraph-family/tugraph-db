#pragma once
#include "../raftpb/raft.pb.h"
#include <google/protobuf/util/message_differencer.h>
#include "../util.h"

namespace raftpb {
using namespace eraft;
// Equivalent returns a nil error if the inputs describe the same configuration.
// On mismatch, returns a descriptive error showing the differences.
inline Error Equivalent(ConfState cs1, ConfState cs2) {
    auto sort = [](ConfState& cs1){
        std::sort(cs1.mutable_voters()->begin(), cs1.mutable_voters()->end(), std::less<uint64_t>());
        std::sort(cs1.mutable_learners()->begin(), cs1.mutable_learners()->end(), std::less<uint64_t>());
        std::sort(cs1.mutable_voters_outgoing()->begin(), cs1.mutable_voters_outgoing()->end(), std::less<uint64_t>());
        std::sort(cs1.mutable_learners_next()->begin(), cs1.mutable_learners_next()->end(), std::less<uint64_t>());
    };
    auto orig1 = cs1;
    auto orig2 = cs2;
    sort(cs1);
    sort(cs2);
    if (!google::protobuf::util::MessageDifferencer::Equals(cs1, cs2)) {
        return  Error(format("ConfStates not equivalent after sorting:\n%s\n%s\nInputs were:\n%s\n%s",
                             cs1.ShortDebugString().c_str(), cs2.ShortDebugString().c_str(),
                             orig1.ShortDebugString().c_str(), orig2.ShortDebugString().c_str()));
    }
    return {};
}

}