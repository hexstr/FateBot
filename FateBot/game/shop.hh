#ifndef SHOP_HEADER
#define SHOP_HEADER

#include <absl/container/flat_hash_map.h>

#include "utils/singleton.hh"

enum class PayType {
    STONE = 1,
    QP,
    FRIEND_POINT,
    MANA,
    TICKET,
    EVENT_ITEM,
    CHARGE_STONE,
    STONE_FRAGMENTS,
    ANONYMOUS,
    RARE_PRI,
    ITEM,
    GRAIL_FRAGMENTS,
    FREE,
    COMMON_CONSUME
};

enum class ShopType {
    NONE,
    EVENT_ITEM,
    MANA,
    RARE_PRI,
    SVT_STORAGE,
    SVT_EQUIP_STORAGE,
    STONE_FRAGMENTS,
    SVT_ANONYMOUS,
    BGM,
    LIMIT_MATERIAL,
    GRAIL_FRAGMENTS,
    SVT_COSTUME,
    START_UP_SUMMON,
    SHOP13,
    TRADE_AP
};

class Shop : public Singleton<Shop> {
public:
    Shop();
    int GetLimitNumber(int id) {
        if (auto itor = opening_list_.find(id); itor != opening_list_.end()) {
            return itor->second;
        }
        return 0;
    }

    absl::flat_hash_map<int, int> opening_list_;
};

#endif // shop.hh