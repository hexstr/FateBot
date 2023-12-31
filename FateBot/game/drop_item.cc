#include "drop_item.hh"

#include "utils/config.hh"
using namespace rapidjson;

DropItem::DropItem() {
    Document doc = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "drop_item.json");
    if (doc.IsArray() && doc.Size() > 0) {
        for (const auto& item : doc.GetArray()) {
            if (item.IsObject()) {
                if (auto itor = item.FindMember("id"); itor != item.MemberEnd() && itor->value.IsNumber()) {
                    if (auto itor2 = item.FindMember("name"); itor2 != item.MemberEnd() && itor2->value.IsString()) {
                        auto id = itor->value.GetInt();
                        auto name = itor2->value.GetString();
                        drop_item_list_.emplace(id, name);
                    }
                }
            }
        }
    }
}

std::string DropItem::GetItemName(int id) {
    if (auto itor = drop_item_list_.find(id); itor != drop_item_list_.end()) {
        return itor->second;
    }
    return {};
}