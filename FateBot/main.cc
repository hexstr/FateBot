/*
Console Color
txtblk='\033[0;30m' # Black - Regular
txtred='\033[0;31m' # Red
txtgrn='\033[0;32m' # Green
txtylw='\033[0;33m' # Yellow
txtblu='\033[0;34m' # Blue
txtpur='\033[0;35m' # Purple
txtcyn='\033[0;36m' # Cyan
txtwht='\033[0;37m' # White
*/

#include <csignal>

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include "game/base_http.hh"
#include "game/game_battle.hh"
#include "game/game_user.hh"
#include "game/shop.hh"
#include "game/task.hh"
#include "game/user_manager.hh"
#include "utils/logger.hh"
#include "utils/my_exception.hh"
#include "utils/mytime.hh"

void SignHandle(int sig) {
    if (sig == SIGINT) {
        GameBattle::is_exit_ = true;
        succ("Exiting...");
    }
}

ABSL_FLAG(uint32_t, count, 999, "Battle times.");
ABSL_FLAG(uint32_t, index, 0, "Index of the users, start from 0.");
ABSL_FLAG(uint32_t, limit_rec, 0, "Limit times for using gold apples.");
ABSL_FLAG(std::string, proxy, {}, "Set https proxy, same as curl -x.");

ABSL_FLAG(bool, enable_check_shop, false, "Check if there are some tickets in the shop.");
ABSL_FLAG(bool, enable_force_battle, false, "Force start battle.");
ABSL_FLAG(bool, enable_force_update, false, "Force check for updates.");
ABSL_FLAG(bool, enable_gacha, false, "Gacha.");
ABSL_FLAG(bool, donot_exchange, false, "DO NOT exchange my material tickets.");
ABSL_FLAG(bool, date, false, "Print version.");

int main(int argc, char* argv[]) {
#ifdef WIN32
    system("chcp 65001");
#endif
    setvbuf(stdout, nullptr, _IONBF, 0);
    signal(SIGINT, SignHandle);

    int count, index;
    bool enable_force_update, enable_gacha, print_date;

    warn("[main] Launching");
    Utility::PrintNowTime();

    {
        absl::ParseCommandLine(argc, argv);
        count = absl::GetFlag(FLAGS_count);
        index = absl::GetFlag(FLAGS_index);
        UserManager::recover_limit_ = absl::GetFlag(FLAGS_limit_rec);
        BaseHttp::proxy_ = absl::GetFlag(FLAGS_proxy);

        User::check_shop_ = absl::GetFlag(FLAGS_enable_check_shop);
        User::donot_exchange_ = absl::GetFlag(FLAGS_donot_exchange);
        GameBattle::force_battle_ = absl::GetFlag(FLAGS_enable_force_battle);
        enable_force_update = absl::GetFlag(FLAGS_enable_force_update);
        print_date = absl::GetFlag(FLAGS_date);
    }

    if (print_date) {
        warn(" Compiled on " __DATE__ " at " __TIME__);
        return 0;
    }

    // Reading cfg.json and initializing.
    try {
        BaseHttp::Initialization();
        BaseHttp::GameData(enable_force_update);
        Task::Initialization();

        UserManager::Initialization();
        GameBattle::ReadRecoveryTimestamp();

        if (User::check_shop_) {
            Shop::GetInstance();
            User::CheckShopThread();
        }

        else {
            GameBattle::StartThread(count, index);
        }
    } catch (MyException& ex) {
        error("Exception: %s", ex.what());
    }
#ifndef NDEBUG
    system("pause");
#endif

    return 0;
}