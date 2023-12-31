#ifndef GAME_USER_HEADER
#define GAME_USER_HEADER

#include <string>

#include "user.hh"

class GameUser {
public:
    GameUser(User& user) : user_(user){};

    rapidjson::Document TopFriend();
    rapidjson::Document TopProfile(const std::string&);
    rapidjson::Document EditName(const std::string&);
    rapidjson::Document OfferFriend(const std::string&);
    rapidjson::Document RemoveFriend(const std::string&);
    rapidjson::Document AssignFollow(const std::string&);
    rapidjson::Document Gacha(const char* gacha_id, const char* gacha_sub_id);
    rapidjson::Document ServantEnhance(const std::string&, const std::string& material_list);
    rapidjson::Document ServantEnhance(int64_t id, const std::string& material_list);
    rapidjson::Document EquipmentEnhance(int64_t id, const std::string& material_list);
    rapidjson::Document SetFilter();

private:
    User& user_;
};

#endif // game_user.hh