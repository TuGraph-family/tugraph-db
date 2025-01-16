#pragma once
#include "../raftpb/raft.pb.h"
#include "confchange.h"

namespace confchange {
using namespace eraft;
using namespace raftpb;
namespace detail {
// toConfChangeSingle translates a conf state into 1) a slice of operations creating
// first the config that will become the outgoing one, and then the incoming one, and
// b) another slice that, when applied to the config resulted from 1), represents the
// ConfState.
inline std::pair<std::vector<raftpb::ConfChangeSingle>, std::vector<raftpb::ConfChangeSingle>>
    toConfChangeSingle(const raftpb::ConfState& cs) {
	// Example to follow along this code:
	// voters=(1 2 3) learners=(5) outgoing=(1 2 4 6) learners_next=(4)
	//
	// This means that before entering the joint config, the configuration
	// had voters (1 2 4 6) and perhaps some learners that are already gone.
	// The new set of voters is (1 2 3), i.e. (1 2) were kept around, and (4 6)
	// are no longer voters; however 4 is poised to become a learner upon leaving
	// the joint state.
	// We can't tell whether 5 was a learner before entering the joint config,
	// but it doesn't matter (we'll pretend that it wasn't).
	//
	// The code below will construct
	// outgoing = add 1; add 2; add 4; add 6
	// incoming = remove 1; remove 2; remove 4; remove 6
	//            add 1;    add 2;    add 3;
	//            add-learner 5;
	//            add-learner 4;
	//
	// So, when starting with an empty config, after applying 'outgoing' we have
	//
	//   quorum=(1 2 4 6)
	//
	// From which we enter a joint state via 'incoming'
	//
	//   quorum=(1 2 3)&&(1 2 4 6) learners=(5) learners_next=(4)
	//
	// as desired.

    std::vector<raftpb::ConfChangeSingle> in, out;
    for (auto id : cs.voters_outgoing()) {
		// If there are outgoing voters, first add them one by one so that the
		// (non-joint) config has them all.
        raftpb::ConfChangeSingle c;
        c.set_node_id(id);
        c.set_type(raftpb::ConfChangeAddNode);
        out.emplace_back(std::move(c));
	}

	// We're done constructing the outgoing slice, now on to the incoming one
	// (which will apply on top of the config created by the outgoing slice).

	// First, we'll remove all of the outgoing voters.
    for (auto id : cs.voters_outgoing()) {
        raftpb::ConfChangeSingle c;
        c.set_node_id(id);
        c.set_type(raftpb::ConfChangeRemoveNode);
        in.emplace_back(std::move(c));
	}
	// Then we'll add the incoming voters and learners.
    for (auto id : cs.voters()) {
        raftpb::ConfChangeSingle c;
        c.set_node_id(id);
        c.set_type(raftpb::ConfChangeAddNode);
        in.emplace_back(std::move(c));
	}
    for (auto id : cs.learners()) {
        raftpb::ConfChangeSingle c;
        c.set_node_id(id);
        c.set_type(raftpb::ConfChangeAddLearnerNode);
        in.emplace_back(std::move(c));
	}
	// Same for LearnersNext; these are nodes we want to be learners but which
	// are currently voters in the outgoing config.
    for (auto id : cs.learners_next()) {
        raftpb::ConfChangeSingle c;
        c.set_node_id(id);
        c.set_type(raftpb::ConfChangeAddLearnerNode);
        in.emplace_back(std::move(c));
	}
	return {std::move(out), std::move(in)};
}

inline std::tuple<tracker::Config, tracker::ProgressMap, Error>
chain(Changer chg, const std::vector<std::function<std::tuple<tracker::Config, tracker::ProgressMap, Error>(Changer)>>& ops) {
    for (auto& op : ops) {
        tracker::ProgressMap trk;
        tracker::Config cfg;
        Error err;
		std::tie(cfg, trk, err) = op(chg);
		if (!err.ok()) {
			return std::make_tuple(tracker::Config{}, tracker::ProgressMap{}, err);
		}
		chg.tracker.config_ = cfg;
		chg.tracker.progress_ = trk;
	}
	return std::make_tuple(chg.tracker.config_, chg.tracker.progress_, Error());
}
}

// Restore takes a Changer (which must represent an empty configuration), and
// runs a sequence of changes enacting the configuration described in the
// ConfState.
//
// TODO(tbg) it's silly that this takes a Changer. Unravel this by making sure
// the Changer only needs a ProgressMap (not a whole Tracker) at which point
// this can just take LastIndex and MaxInflight directly instead and cook up
// the results from that alone.
inline std::tuple<tracker::Config, tracker::ProgressMap, Error>
Restore(Changer chg, const raftpb::ConfState& cs) {
    std::vector<raftpb::ConfChangeSingle> outgoing, incoming;
	std::tie(outgoing, incoming) = detail::toConfChangeSingle(cs);

    std::vector<std::function<std::tuple<tracker::Config, tracker::ProgressMap, Error>(Changer)>> ops;

	if (outgoing.empty()) {
		// No outgoing config, so just apply the incoming changes one by one.
        for (auto& cc : incoming) {
            ops.emplace_back([cc](Changer chg) {
                return chg.Simple({cc});
            });
		}
	} else {
		// The ConfState describes a joint configuration.
		//
		// First, apply all of the changes of the outgoing config one by one, so
		// that it temporarily becomes the incoming active config. For example,
		// if the config is (1 2 3)&(2 3 4), this will establish (2 3 4)&().
        for (auto& cc : outgoing) {
            ops.emplace_back([cc](Changer chg) {
                return chg.Simple({cc});
            });
		}
		// Now enter the joint state, which rotates the above additions into the
		// outgoing config, and adds the incoming config in. Continuing the
		// example above, we'd get (1 2 3)&(2 3 4), i.e. the incoming operations
		// would be removing 2,3,4 and then adding in 1,2,3 while transitioning
		// into a joint state.
        ops.emplace_back([cs, incoming](Changer chg) {
            return chg.EnterJoint(cs.auto_leave(), incoming);
        });
	}

	return detail::chain(std::move(chg), ops);
}

}