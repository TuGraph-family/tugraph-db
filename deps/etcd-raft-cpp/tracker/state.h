#pragma once
#include "../util.h"
namespace tracker {

enum class StateType {
    // StateProbe indicates a follower whose last index isn't known. Such a
    // follower is "probed" (i.e. an append sent periodically) to narrow down
    // its last index. In the ideal (and common) case, only one round of probing
    // is necessary as the follower will react with a hint. Followers that are
    // probed over extended periods of time are often offline.
    StateProbe = 0,
    // StateReplicate is the state steady in which a follower eagerly receives
    // log entries to append to its log.
    StateReplicate,
    // StateSnapshot indicates a follower that needs log entries not available
    // from the leader's Raft log. Such a follower needs a full snapshot to
    // return to StateReplicate.
    StateSnapshot
};

inline std::string ToString(StateType s) {
    switch (s) {
        case StateType::StateProbe:
            return "StateProbe";
        case StateType::StateReplicate:
            return "StateReplicate";
        case StateType::StateSnapshot:
            return "StateSnapshot";
        default:
            return "UnknownStateType";
    }
}

}