#ifndef ITEMS_HEADER
#define ITEMS_HEADER

class Items {
public:
    enum StatusFlag {
        UNLOCK = 0,
        LOCK = 1,
        EVENT_JOIN = 2,
        WITHDRAWAL = 4,
        APRIL_FOOL_CANCEL = 8,
        CHOICE = 16,
        NO_PERIOD = 32,
        COND_JOIN = 64,
        ADD_FRIENDSHIP_HEROINE = 128
    };
    static bool IsRecvPresent(int id);
    static bool IsSellServant(int id);
};

#endif // items.hh