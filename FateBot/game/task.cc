#include "task.hh"

#include "utils/config.hh"
#include "utils/logger.hh"

using namespace rapidjson;

using std::string;

bool Task::is_loaded_ = false;
absl::flat_hash_map<int, Task> Task::task_list_;

void Task::Initialization() {
    if (!is_loaded_) {
        auto task = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "task.json");
        for (auto& t : task.GetArray()) {
            int task_id = t["TaskId"].GetInt();
            task_list_.emplace(task_id,
                               Task(t["FollowerId"].GetInt(),
                                    t["FollowerType"].GetInt(),
                                    t["FollowerClassId"].GetInt(),
                                    t["QuestCost"].GetInt(),
                                    t["QuestId"].GetInt(),
                                    t["QuestPhase"].GetInt(),
                                    t["FollowerEquipId"].GetInt(),
                                    t["IsEvent"].GetBool()));
        }
        info("[Task] %d tasks loaded.", task_list_.size());
        is_loaded_ = true;
    }
}