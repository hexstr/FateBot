#include "top_login.hh"

void UserGame::Unmarshal(const rapidjson::GenericValue<rapidjson::UTF8<>>& input) {
    if (auto itor = input.FindMember("message"); itor != input.MemberEnd() && itor->value.IsString()) {
        message_ = itor->value.GetString();
    }

    if (auto itor = input.FindMember("stone"); itor != input.MemberEnd() && itor->value.IsUint()) {
        stone_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("friendKeep"); itor != input.MemberEnd() && itor->value.IsUint()) {
        friend_keep_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("svtKeep"); itor != input.MemberEnd() && itor->value.IsUint()) {
        svt_keep_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("svtEquipKeep"); itor != input.MemberEnd() && itor->value.IsUint()) {
        svt_equip_keep_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("userId"); itor != input.MemberEnd() && itor->value.IsUint()) {
        user_id_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("name"); itor != input.MemberEnd() && itor->value.IsString()) {
        name_ = itor->value.GetString();
    }

    if (auto itor = input.FindMember("birthDay"); itor != input.MemberEnd() && itor->value.IsInt()) {
        birth_day_ = itor->value.GetInt();
    }

    if (auto itor = input.FindMember("actMax"); itor != input.MemberEnd() && itor->value.IsUint()) {
        act_max_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("actRecoverAt"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        act_recover_at_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("carryOverActPoint"); itor != input.MemberEnd() && itor->value.IsUint()) {
        carry_over_act_point_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("rpRecoverAt"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        rp_recover_at_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("carryOverRaidPoint"); itor != input.MemberEnd() && itor->value.IsUint()) {
        carry_over_raid_point_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("genderType"); itor != input.MemberEnd() && itor->value.IsUint()) {
        gender_type_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("lv"); itor != input.MemberEnd() && itor->value.IsUint()) {
        lv_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("exp"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        exp_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("qp"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        qp_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("costMax"); itor != input.MemberEnd() && itor->value.IsUint()) {
        cost_max_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("friendCode"); itor != input.MemberEnd() && itor->value.IsString()) {
        friend_code_ = itor->value.GetString();
    }

    if (auto itor = input.FindMember("favoriteUserSvtId"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        favorite_user_svt_id_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("pushUserSvtId"); itor != input.MemberEnd() && itor->value.IsUint()) {
        push_user_svt_id_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("flag"); itor != input.MemberEnd() && itor->value.IsUint()) {
        flag_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("commandSpellRecoverAt"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        command_spell_recover_at_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("svtStorageAdjust"); itor != input.MemberEnd() && itor->value.IsUint()) {
        svt_storage_adjust_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("svtEquipStorageAdjust"); itor != input.MemberEnd() && itor->value.IsUint()) {
        svt_equip_storage_adjust_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("userEquipId"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        user_equip_id_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("freeStone"); itor != input.MemberEnd() && itor->value.IsUint()) {
        free_stone_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("chargeStone"); itor != input.MemberEnd() && itor->value.IsUint()) {
        charge_stone_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("mana"); itor != input.MemberEnd() && itor->value.IsUint()) {
        mana_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("rarePri"); itor != input.MemberEnd() && itor->value.IsUint()) {
        rare_pri_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("activeDeckId"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        active_deck_id_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("mainSupportDeckId"); itor != input.MemberEnd() && itor->value.IsUint()) {
        main_support_deck_id_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("eventSupportDeckId"); itor != input.MemberEnd() && itor->value.IsUint()) {
        event_support_deck_id_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("tutorial1"); itor != input.MemberEnd() && itor->value.IsUint()) {
        tutorial1_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("tutorial2"); itor != input.MemberEnd() && itor->value.IsUint()) {
        tutorial2_ = itor->value.GetUint();
    }

    if (auto itor = input.FindMember("updatedAt"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        updated_at_ = itor->value.GetUint64();
    }

    if (auto itor = input.FindMember("createdAt"); itor != input.MemberEnd() && itor->value.IsUint64()) {
        created_at_ = itor->value.GetUint64();
    }
}