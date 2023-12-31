#ifndef USERMANAGER_HEADER
#define USERMANAGER_HEADER

#include <vector>

#include <absl/status/statusor.h>

#include "user.hh"

class GameBattle;
class GameUser;

class UserManager {
public:
    static std::vector<User> user_list_;
    static int user_amount_;
    static int daily_id_;
    static int recover_limit_;

    static void Initialization();
};

#endif // user_manager.hh