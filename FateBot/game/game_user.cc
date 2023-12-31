#include "game_user.hh"

#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>

#include "items.hh"
#include "utils/config.hh"
#include "utils/logger.hh"
#include "utils/my_exception.hh"

using std::string;

using absl::StrAppendFormat;

using namespace cpr;
using namespace rapidjson;

absl::flat_hash_set<int64_t> gacha_material_id;
int64_t target_to_enhance = 0;

Document GameUser::TopFriend() {
    return user_.PostRequest(Url(GameServer "friend/top?_userId=" + user_.id_));
}

Document GameUser::TopProfile(const std::string& friend_code) {
    user_.builder_.AddParameter("friendCode", friend_code);
    return user_.PostRequest(Url(GameServer "profile/top?_userId=" + user_.id_));
}

Document GameUser::EditName(const string& name) {
    succ("[EditName] [%s] [%s]", user_.name_, name);
    user_.builder_.AddParameter("genderType", "2");
    user_.builder_.AddParameter("message", "");
    user_.builder_.AddParameter("name", name);
    return user_.PostRequest(Url(GameServer "profile/editName?_userId=" + user_.id_));
}

Document GameUser::OfferFriend(const std::string& id) {
    succ("[OfferFriend] [%s]", user_.name_);
    user_.builder_.AddParameter("targetUserId", id);
    return user_.PostRequest(Url(GameServer "friend/offer?_userId=" + user_.id_));
}

Document GameUser::RemoveFriend(const string& targetUserId) {
    succ("[RemoveFriend] [%s]", user_.name_);
    user_.builder_.AddParameter("targetUserId", targetUserId);
    return user_.PostRequest(Url(GameServer "friend/offer?_userId=" + user_.id_));
}

Document GameUser::AssignFollow(const std::string& id) {
    succ("[AssignFollow] [%s]", user_.name_);
    user_.builder_.AddParameter("followId", id);
    return user_.PostRequest(Url(GameServer "follow/assign?_userId=" + user_.id_));
}

Document GameUser::Gacha(const char* gachaId, const char* gachaSubId) {
    succ("[Gacha] [%s]", user_.name_);
    user_.builder_.AddParameter("gachaId", gachaId);
    user_.builder_.AddParameter("gachaSubId", gachaSubId);
    user_.builder_.AddParameter("num", "10");
    user_.builder_.AddParameter("shopIdIndex", "1");
    user_.builder_.AddParameter("storyAdjustIds", "[]");
    user_.builder_.AddParameter("ticketItemId", "0");

    return user_.PostRequest(Url(GameServer "gacha/draw?_userId=" + user_.id_));
}

Document GameUser::ServantEnhance(const string& id, const std::string& material_list) {
    succ("[ServantEnhance] [%s]", user_.name_);
    user_.builder_.AddParameter("baseUserSvtId", id);
    user_.builder_.AddParameter("materialUserSvtIds", material_list);
    return user_.PostRequest(Url(GameServer "card/combine?_userId=" + user_.id_));
}

Document GameUser::ServantEnhance(int64_t id, const std::string& material_list) {
    succ("[ServantEnhance] [%s]", user_.name_);
    user_.builder_.AddParameter("baseUserSvtId", id);
    user_.builder_.AddParameter("materialUserSvtIds", material_list);
    return user_.PostRequest(Url(GameServer "card/combine?_userId=" + user_.id_));
}

Document GameUser::EquipmentEnhance(int64_t id, const string& material_list) {
    succ("[EquipmentEnhance] [%s]", user_.name_);
    user_.builder_.AddParameter("baseUserSvtId", id);
    user_.builder_.AddParameter("materialUserSvtIds", material_list);
    return user_.PostRequest(Url(GameServer "svtEquip/combine?_userId=" + user_.id_));
}

Document GameUser::SetFilter() {
    succ("[SetFilter] [%s]", user_.name_);
    user_.builder_.AddParameter("onFlagNumbers", "[13,14,15,16]");
    return user_.PostRequest(Url(GameServer "userStatus/flagSet?_userId=" + user_.id_));
}
