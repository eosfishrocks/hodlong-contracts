#include "Trackers.hpp"

namespace bpfish {
    void trackers::add(const name account, string& tracker_url){
        require_auth(account);

        wtrackers  wtrackers(_self, _self.value);
        auto iterator = wtrackers.find(account.value);
        eosio_assert(iterator == wtrackers.end(), "A tracker is already configured for this account.");

        wtrackers.emplace(account, [&](auto &tracker){
            tracker.account = account;
            tracker.url = tracker_url;
        });

    }
    void trackers::remove(const name account){
        require_auth(account);

        wtrackers wtrackers(_self, _self.value);
        auto iterator = wtrackers.find(account.value);
        eosio_assert(iterator != wtrackers.end(), "A tracker doesn't exist for this account.");

        wtrackers.erase(iterator);
    }
    void trackers::update(const name account, string& tracker_url){
        require_auth(account);

        wtrackers wtrackers(_self, _self.value);

        auto iterator = wtrackers.find(account.value);
        eosio_assert(iterator != wtrackers.end(), "A tracker doesn't exist for this account.");

        wtrackers.modify(iterator, account, [&](auto& trackers) {
            trackers.url = tracker_url;
        });
    }
}