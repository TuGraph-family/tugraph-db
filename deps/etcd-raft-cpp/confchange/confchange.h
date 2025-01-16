#pragma once
#include "../util.h"
#include "../raftpb/raft.pb.h"

namespace confchange {
using namespace eraft;

namespace detail {
inline const quorum::MajorityConfig& incoming(const quorum::JointConfig& voters) { return voters[0]; }

inline quorum::MajorityConfig& incoming(quorum::JointConfig& voters) { return voters[0]; }

inline const quorum::MajorityConfig& outgoing(const quorum::JointConfig& voters) { return voters[1]; }

inline quorum::MajorityConfig& outgoing(quorum::JointConfig& voters) { return voters[1]; }
}

// Describe prints the type and NodeID of the configuration changes as a
// space-delimited string.
inline std::string Describe(const std::vector<raftpb::ConfChangeSingle>& ccs) {
	std::string buf;
    for (auto& cc : ccs) {
		if (!buf.empty()) {
            buf.push_back(' ');
		}
        buf.append(format("%s(%d)", raftpb::ConfChangeType_Name(cc.type()).c_str(), cc.node_id()));
	}
	return buf;
}

namespace detail {
inline bool joint(const tracker::Config& cfg) {
    return !outgoing(cfg.voters_).data().empty();
}

// symdiff returns the count of the symmetric difference between the sets of
// uint64s, i.e. len( (l - r) \union (r - l)).
inline size_t symdiff(const std::unordered_set<uint64_t>& l, const std::unordered_set<uint64_t>& r) {
    size_t n = 0;
    std::vector<std::vector<quorum::MajorityConfig>> pairs{
            {quorum::MajorityConfig(l), quorum::MajorityConfig(r)}, // count elems in l but not in r
            {quorum::MajorityConfig(r), quorum::MajorityConfig(l)}  // count elems in r but not in l
    };
    for (auto& p: pairs) {
        for (auto id: p[0].data()) {
            if (!p[1].data().count(id)) {
                n++;
            }
        }
    }
    return n;
}

// checkInvariants makes sure that the config and progress are compatible with
// each other. This is used to check both what the Changer is initialized with,
// as well as what it returns.
inline Error checkInvariants(const tracker::Config& cfg, const tracker::ProgressMap& trk) {
    // NB: intentionally allow the empty config. In production we'll never see a
    // non-empty config (we prevent it from being created) but we will need to
    // be able to *create* an initial config, for example during bootstrap (or
    // during tests). Instead of having to hand-code this, we allow
    // transitioning from an empty config into any other legal and non-empty
    // config.
    for (auto& ids: std::vector<std::unordered_set<uint64_t>>{cfg.voters_.IDs(), cfg.learners_, cfg.learnersNext_}) {
        for (auto id: ids) {
            if (!trk.data().count(id)) {
                return Error(format("no progress for %d", id));
            }
        }
    }

    // Any staged learner was staged because it could not be directly added due
    // to a conflicting voter in the outgoing config.
    for (auto id: cfg.learnersNext_) {
        if (!outgoing(cfg.voters_).data().count(id)) {
            return Error(format("%d is in LearnersNext, but not Voters[1]", id));
        }
        if (trk[id]->isLearner_) {
            return Error(format("%d is in LearnersNext, but is already marked as learner", id));
        }
    }
    // Conversely Learners and Voters doesn't intersect at all.
    for (auto id: cfg.learners_) {
        if (outgoing(cfg.voters_).data().count(id)) {
            return Error(format("%d is in Learners and Voters[1]", id));
        }
        if (incoming(cfg.voters_).data().count(id)) {
            return Error(format("%d is in Learners and Voters[0]", id));
        }
        if (!trk[id]->isLearner_) {
            return Error(format("%d is in Learners, but is not marked as learner", id));
        }
    }

    if (!joint(cfg)) {
        // We enforce that empty maps are nil instead of zero.
        if (!outgoing(cfg.voters_).data().empty()) {
            return Error(format("cfg.Voters[1] must be nil when not joint"));
        }
        if (!cfg.learnersNext_.empty()) {
            return Error(format("cfg.LearnersNext must be nil when not joint"));
        }
        if (cfg.autoLeave_) {
            return Error(format("AutoLeave must be false when not joint"));
        }
    }

    return {};
}

// checkAndReturn calls checkInvariants on the input and returns either the
// resulting error or the input.
inline std::tuple<tracker::Config, tracker::ProgressMap, Error>
checkAndReturn(const tracker::Config& cfg, const tracker::ProgressMap& trk) {
    auto err = checkInvariants(cfg, trk);
    if (!err.ok()) {
        return std::make_tuple(tracker::Config{}, tracker::ProgressMap{}, std::move(err));
    }
    return std::make_tuple(cfg, trk, Error());
}
}

// Changer facilitates configuration changes. It exposes methods to handle
// simple and joint consensus while performing the proper validation that allows
// refusing invalid configuration changes before they affect the active
// configuration.
struct Changer {
    tracker::ProgressTracker tracker;
    uint64_t lastIndex = 0;

    Changer(tracker::ProgressTracker t, uint64_t i) : tracker(std::move(t)), lastIndex(i) {}

    // checkAndCopy copies the tracker's config and progress map (deeply enough for
    // the purposes of the Changer) and returns those copies. It returns an error
    // if checkInvariants does.
    std::tuple<tracker::Config, tracker::ProgressMap, Error>
    checkAndCopy() {
        auto cfg = tracker.config_.Clone();
        tracker::ProgressMap trk;

        for (auto& item : tracker.progress_.data()) {
            auto id = item.first;
            auto& pr = item.second;
            // A shallow copy is enough because we only mutate the Learner field.
            trk.data()[id] = std::make_shared<tracker::Progress>(*pr);
        }
        return detail::checkAndReturn(cfg, trk);
    }

    // err returns zero values and an error.
    std::tuple<tracker::Config, tracker::ProgressMap, Error>
    err(const Error& err) {
        return std::make_tuple(tracker::Config{}, tracker::ProgressMap{}, err);
    }

    // initProgress initializes a new progress for the given node or learner.
    void initProgress(tracker::Config& cfg, tracker::ProgressMap& trk, uint64_t id, bool isLearner) const {
        if (!isLearner) {
            detail::incoming(cfg.voters_).data().emplace(id);
        } else {
            cfg.learners_.emplace(id);
        }
        auto pro = std::make_shared<tracker::Progress>();
        // Initializing the Progress with the last index means that the follower
        // can be probed (with the last index).
        //
        // TODO(tbg): seems awfully optimistic. Using the first index would be
        // better. The general expectation here is that the follower has no log
        // at all (and will thus likely need a snapshot), though the app may
        // have applied a snapshot out of band before adding the replica (thus
        // making the first index the better choice).
        pro->next_ = lastIndex;
        pro->match_ = 0;
        pro->inflights_ = tracker::NewInflights(tracker.maxInflight_, tracker.maxInflightBytes_);
        pro->isLearner_ = isLearner;
        // When a node is first added, we should mark it as recently active.
        // Otherwise, CheckQuorum may cause us to step down if it is invoked
        // before the added node has had a chance to communicate with us.
        pro->recentActive_ = true;

        trk.data()[id] = pro;
    };

    // makeVoter adds or promotes the given ID to be a voter in the incoming
    // majority config.
    void makeVoter(tracker::Config& cfg, tracker::ProgressMap& trk, uint64_t id) const {
        if (!trk.data().count(id)) {
            initProgress(cfg, trk, id, false /* isLearner */);
            return;
        }
        auto pr = trk.data()[id];
        pr->isLearner_ = false;
        cfg.learners_.erase(id);
        cfg.learnersNext_.erase(id);
        detail::incoming(cfg.voters_).data().emplace(id);
    }

    // remove this peer as a voter or learner from the incoming config.
    void remove(tracker::Config& cfg, tracker::ProgressMap& trk, uint64_t id) {
        if (!trk.data().count(id)) {
            return;
        }

        detail::incoming(cfg.voters_).data().erase(id);
        cfg.learners_.erase(id);
        cfg.learnersNext_.erase(id);

        // If the peer is still a voter in the outgoing config, keep the Progress.
        if (!detail::outgoing(cfg.voters_).data().count(id)) {
            trk.data().erase(id);
        }
    }

    // makeLearner makes the given ID a learner or stages it to be a learner once
    // an active joint configuration is exited.
    //
    // The former happens when the peer is not a part of the outgoing config, in
    // which case we either add a new learner or demote a voter in the incoming
    // config.
    //
    // The latter case occurs when the configuration is joint and the peer is a
    // voter in the outgoing config. In that case, we do not want to add the peer
    // as a learner because then we'd have to track a peer as a voter and learner
    // simultaneously. Instead, we add the learner to LearnersNext, so that it will
    // be added to Learners the moment the outgoing config is removed by
    // LeaveJoint().
    void makeLearner(tracker::Config& cfg, tracker::ProgressMap& trk, uint64_t id) {
        if (!trk.data().count(id)) {
            initProgress(cfg, trk, id, true /* isLearner */);
            return;
        }
        auto pr = trk.data()[id];
        if (pr->isLearner_) {
            return;
        }
        // Remove any existing voter in the incoming config...
        remove(cfg, trk, id);
        // ... but save the Progress.
        trk.data()[id] = pr;
        // Use LearnersNext if we can't add the learner to Learners directly, i.e.
        // if the peer is still tracked as a voter in the outgoing config. It will
        // be turned into a learner in LeaveJoint().
        //
        // Otherwise, add a regular learner right away.
        if (detail::outgoing(cfg.voters_).data().count(id)) {
            cfg.learnersNext_.emplace(id);
        } else {
            pr->isLearner_ = true;
            cfg.learners_.emplace(id);
        }
    }

    // apply a change to the configuration. By convention, changes to voters are
    // always made to the incoming majority config Voters[0]. Voters[1] is either
    // empty or preserves the outgoing majority configuration while in a joint state.
    Error apply(tracker::Config& cfg, tracker::ProgressMap& trk, const std::vector<raftpb::ConfChangeSingle>& ccs) {
        for (auto& cc : ccs) {
            if (cc.node_id() == 0) {
                // etcd replaces the NodeID with zero if it decides (downstream of
                // raft) to not apply a change, so we have to have explicit code
                // here to ignore these.
                continue;
            }
            switch (cc.type()) {
                case raftpb::ConfChangeAddNode:
                    makeVoter(cfg, trk, cc.node_id());
                    break;
                case raftpb::ConfChangeAddLearnerNode:
                    makeLearner(cfg, trk, cc.node_id());
                    break;
                case raftpb::ConfChangeRemoveNode:
                    remove(cfg, trk, cc.node_id());
                    break;
                case raftpb::ConfChangeUpdateNode:
                    // TODO wangzhiyong
                    break;
                default:
                    return Error(format("unexpected conf type %d", cc.type()));
            }
        }
        if (detail::incoming(cfg.voters_).data().empty()) {
            return Error("removed all voters");
        }
        return {};
    }

    // EnterJoint verifies that the outgoing (=right) majority config of the joint
    // config is empty and initializes it with a copy of the incoming (=left)
    // majority config. That is, it transitions from
    //
    // (1 2 3)&&()
    // to
    // (1 2 3)&&(1 2 3).
    //
    // The supplied changes are then applied to the incoming majority config,
    // resulting in a joint configuration that in terms of the Raft thesis[1]
    // (Section 4.3) corresponds to `C_{new,old}`.
    //
    // [1]: https://github.com/ongardie/dissertation/blob/master/online-trim.pdf
    std::tuple<tracker::Config, tracker::ProgressMap, Error>
    EnterJoint(bool autoLeave, const std::vector<raftpb::ConfChangeSingle>& ccs) {
        tracker::Config cfg;
        tracker::ProgressMap trk;
        Error err;
        std::tie(cfg, trk, err) = checkAndCopy();
        if (!err.ok()) {
            return this->err(err);
        }
        if (detail::joint(cfg)) {
            return this->err(Error("config is already joint"));
        }
        if (detail::incoming(cfg.voters_).data().empty()) {
            // We allow adding nodes to an empty config for convenience (testing and
            // bootstrap), but you can't enter a joint state.
            return this->err(Error("can't make a zero-voter config joint"));
        }
        // Clear the outgoing config.
        detail::outgoing(cfg.voters_) = quorum::MajorityConfig{};
        // Copy incoming to outgoing.
        for (auto id : detail::incoming(cfg.voters_).data()) {
            detail::outgoing(cfg.voters_).data().emplace(id);
        }
        err = apply(cfg, trk, ccs);
        if (!err.ok()) {
            return this->err(err);
        }
        cfg.autoLeave_ = autoLeave;
        return detail::checkAndReturn(cfg, trk);
    }

    // LeaveJoint transitions out of a joint configuration. It is an error to call
    // this method if the configuration is not joint, i.e. if the outgoing majority
    // config Voters[1] is empty.
    //
    // The outgoing majority config of the joint configuration will be removed,
    // that is, the incoming config is promoted as the sole decision maker. In the
    // notation of the Raft thesis[1] (Section 4.3), this method transitions from
    // `C_{new,old}` into `C_new`.
    //
    // At the same time, any staged learners (LearnersNext) the addition of which
    // was held back by an overlapping voter in the former outgoing config will be
    // inserted into Learners.
    //
    // [1]: https://github.com/ongardie/dissertation/blob/master/online-trim.pdf
    std::tuple<tracker::Config, tracker::ProgressMap, Error>
    LeaveJoint() {
        tracker::Config cfg;
        tracker::ProgressMap trk;
        Error err;
        std::tie(cfg, trk, err) = checkAndCopy();
        if (!err.ok()) {
            return this->err(err);
        }
        if (!detail::joint(cfg)) {
            return this->err(Error("can't leave a non-joint config"));
        }
        for (auto id : cfg.learnersNext_) {
            cfg.learners_.emplace(id);
            // TODO(wangzhiyong)
            trk[id]->isLearner_ = true;
        }
        cfg.learnersNext_.clear();

        for (auto id : detail::outgoing(cfg.voters_).data()) {
            auto isVoter = detail::incoming(cfg.voters_).data().count(id) > 0;
            auto isLearner = cfg.learners_.count(id) > 0;

            if (!isVoter && !isLearner) {
                trk.data().erase(id);
            }
        }
        detail::outgoing(cfg.voters_).data().clear();
        cfg.autoLeave_ = false;

        return detail::checkAndReturn(cfg, trk);
    }

    // Simple carries out a series of configuration changes that (in aggregate)
    // mutates the incoming majority config Voters[0] by at most one. This method
    // will return an error if that is not the case, if the resulting quorum is
    // zero, or if the configuration is in a joint state (i.e. if there is an
    // outgoing configuration).
    std::tuple<tracker::Config, tracker::ProgressMap, Error>
    Simple(const std::vector<raftpb::ConfChangeSingle>& ccs) {
        tracker::Config cfg;
        tracker::ProgressMap trk;
        Error err;
        std::tie(cfg, trk, err) = checkAndCopy();
        if (!err.ok()) {
            return this->err(err);
        }
        if (detail::joint(cfg)) {
            return this->err(Error("can't apply simple config change in joint config"));
        }
        err = apply(cfg, trk, ccs);
        if (!err.ok()){
            return this->err(err);
        }
        auto n = detail::symdiff(detail::incoming(tracker.config_.voters_).data(), detail::incoming(cfg.voters_).data());
        if (n > 1) {
            return std::make_tuple(tracker::Config{}, tracker::ProgressMap{}, Error("more than one voter changed without entering joint config"));
        }

        return detail::checkAndReturn(cfg, trk);
    }

};

}