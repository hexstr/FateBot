#ifndef TASK_HEADER
#define TASK_HEADER

#include <string>

#include <absl/container/flat_hash_map.h>
#include <absl/status/statusor.h>

class Task {
public:
    Task() = delete;
    Task(int follower_id,
         int follower_type,
         int follower_classId,
         int quest_cost,
         int quest_id,
         int quest_phase,
         int follower_equip_id,
         bool is_event)
        : follower_id_(follower_id),
          follower_type_(follower_type),
          follower_class_id_(follower_classId),
          quest_cost_(quest_cost),
          quest_id_(quest_id),
          quest_phase_(quest_phase),
          follower_equip_id_(follower_equip_id),
          is_event_(is_event) {}

    int follower_id_;
    int follower_type_; // Nono = 0; Friend = 1; Not friend = 2; NPC = 3
    int follower_class_id_;
    int quest_cost_;
    int quest_id_;
    int quest_phase_;
    int follower_equip_id_;
    bool is_event_;

    static bool is_loaded_;
    static absl::flat_hash_map<int, Task> task_list_;
    static void Initialization();
};

#endif // task.hh