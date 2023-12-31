#include "shop.hh"

#include "utils/config.hh"
#include "utils/mytime.hh"

using namespace rapidjson;

Shop::Shop() {
    auto doc = Utility::LoadJsonFromFile(GLOBALSETTINGPATH "mstShop");
    int now_time = Utility::GetTimestamp10Int();
    for (auto& item : doc.GetArray()) {
        if (auto itor_open = item.FindMember("openedAt"); itor_open != item.MemberEnd() && itor_open->value.IsInt()) {
            if (auto itor_close = item.FindMember("closedAt"); itor_close != item.MemberEnd() && itor_close->value.IsInt()) {
                if (itor_open->value.GetInt() < now_time && now_time < itor_close->value.GetInt()) {
                    auto id = item["id"].GetInt();
                    auto payType = static_cast<PayType>(item["payType"].GetInt());
                    auto shopType = static_cast<ShopType>(item["shopType"].GetInt());
                    auto limitNum = item["limitNum"].GetInt();
                    auto price = item["prices"].GetArray();
                    auto targetIds = item["targetIds"].GetArray();
                    if (targetIds[0].GetInt() == 4001 && payType == PayType::MANA && shopType == ShopType::MANA) {
                        opening_list_[id] = limitNum;
                        std::cout << "\tShopId: " << id
                                  << "\tLimitNum: " << limitNum
                                  << std::endl;
                    }
                }
            }
        }
    }
}