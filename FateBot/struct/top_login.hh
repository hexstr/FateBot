#ifndef TOP_LOGIN_HEADER
#define TOP_LOGIN_HEADER

#include <string>

#include <rapidjson/document.h>

class UserGame {
public:
    std::string message_;
    uint32_t stone_;
    uint32_t friend_keep_;
    uint32_t svt_keep_;
    uint32_t svt_equip_keep_;
    uint32_t user_id_;
    std::string name_;
    uint32_t birth_day_;
    uint32_t act_max_;
    uint64_t act_recover_at_;
    uint32_t carry_over_act_point_;
    uint64_t rp_recover_at_;
    uint32_t carry_over_raid_point_;
    uint32_t gender_type_;
    uint32_t lv_;
    uint64_t exp_;
    uint64_t qp_;
    uint32_t cost_max_;
    std::string friend_code_;
    uint64_t favorite_user_svt_id_;
    uint32_t push_user_svt_id_;
    uint32_t flag_;
    uint64_t command_spell_recover_at_;
    uint32_t svt_storage_adjust_;
    uint32_t svt_equip_storage_adjust_;
    uint64_t user_equip_id_;
    uint32_t free_stone_;
    uint32_t charge_stone_;
    uint32_t mana_;
    uint32_t rare_pri_;
    uint64_t active_deck_id_;
    uint32_t main_support_deck_id_;
    uint32_t event_support_deck_id_;
    uint32_t tutorial1_;
    uint32_t tutorial2_;
    uint64_t updated_at_;
    uint64_t created_at_;

    void Unmarshal(const rapidjson::GenericValue<rapidjson::UTF8<>>&);
};

#endif // top_login.hh