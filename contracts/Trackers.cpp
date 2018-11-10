#include "Trackers.hpp"

namespace bpfish {
    void trackers::add(const name account, string& tracker_url){
        require_auth(account);

        wtrackers  webtrackers(_self, _self.value);
        auto iterator = webtrackers.find(account.value);
        eosio_assert(iterator == webtrackers.end(), "A tracker is already configured for this account.");

        webtrackers.emplace(account, [&](auto &tracker){
            tracker.account = account;
            tracker.url = tracker_url;
        });

    }
    void trackers::remove(const name account){
        require_auth(account);

        wtrackers webtrackers(_self, _self.value);
        auto iterator = webtrackers.find(account.value);
        eosio_assert(iterator != webtrackers.end(), "A tracker doesn't exist for this account.");

        webtrackers.erase(iterator);
    }
    void trackers::update(const name account, string& tracker_url){
        require_auth(account);

        wtrackers webtrackers(_self, _self.value);

        auto iterator = webtrackers.find(account.value);
        eosio_assert(iterator != webtrackers.end(), "A tracker doesn't exist for this account.");

        webtrackers.modify(iterator, account, [&](auto& trackers) {
            trackers.url = tracker_url;
        });
    }
}