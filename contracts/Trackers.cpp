#include "Trackers.hpp"

namespace bpfish {
    void trackers::add(const name account, string& tracker_url){
        require_auth(account);

        auto iterator = _webtrackers.find(account.value);
        eosio_assert(iterator == _webtrackers.end(), "A tracker is already configured for this account.");

        _webtrackers.emplace(get_self(), [&](auto &tracker){
            tracker.account = account;
            tracker.url = tracker_url;
        });

    }
    void trackers::remove(const name account){
        eosio_assert(has_auth(account) || has_auth(_self), "Fails admin or user auth");

        auto iterator = _webtrackers.find(account.value);
        eosio_assert(iterator != _webtrackers.end(), "A tracker doesn't exist for this account.");
        eosio::print("test");
        _webtrackers.erase(iterator);
    }
    void trackers::update(const name account, string& tracker_url){
        require_auth(account);

        auto iterator = _webtrackers.find(account.value);
        eosio_assert(iterator != _webtrackers.end(), "A tracker doesn't exist for this account.");

        _webtrackers.modify(iterator, account, [&](auto& trackers) {
            trackers.url = tracker_url;
        });
    }
}