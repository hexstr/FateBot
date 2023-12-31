#ifndef DROPINFO_HEADER
#define DROPINFO_HEADER

#include <string>

#include <absl/container/btree_map.h>

#include "utils/singleton.hh"

class DropItem : public Singleton<DropItem> {
public:
    DropItem();
    std::string GetItemName(int id);

private:
    absl::btree_map<int, std::string> drop_item_list_;
};

#endif // drop_item.hh