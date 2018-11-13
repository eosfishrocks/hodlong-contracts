#include "Hodlong.hpp"

    ACTION hodlong::buy(name buyer, name storage_id) {
        auto iterator = _storage.find(storage_id.value);
        eosio_assert(iterator != _storage.end(), "The bid not found");
        eosio_assert(iterator->accepted_seeders.size() >= iterator->max_seeders,
                     "The storage object has the max amount of seeders");
    }

    ACTION hodlong::createobj(name account, storage_t newObj) {
        require_auth(account.value);
        auto iterator = _storage.find(newObj.storage_id.value);
        eosio_assert(iterator == _storage.end(), "Obj for this ID already exists");
        _storage.emplace(get_self(), [&](auto &storage_obj) {
            storage_obj = newObj;
        });

    }

    ACTION hodlong::addstats(const name from, const name to, name storage_id, bool seeder, uint64_t amount) {
        require_auth(from);


        time_t date = now();
        stat
        client_stat = {from, to, amount, seeder};

        auto pstat_itr = _pstats.find(storage_id.value);
        bool foundStat = false;
        if (pstat_itr == _pstats.end()) {
            _pstats.emplace(get_self(), [&](auto &tmp_stat) {
                tmp_stat.storage_id = storage_id;
                tmp_stat.pending_stats.push_back(client_stat);
            });
            foundStat = true;
        } else {
            _pstats.modify(pstat_itr, from, [&](auto &tmp_stat) {
                tmp_stat.pending_stats.push_back(client_stat);
            });
        }
        //may need to break into deferred actions for delete depending on mainnet processing times
        if (foundStat) {

            auto storageIterator = _storage.find(storage_id.value);
            eosio_assert(storageIterator == _storage.end(), "The storage id is not found");

            vector <uint64_t> pendingDeletion;
            // Inefficient loop and due to RAM Cost. Cheaper to offload to cpu
            for (int v1 = 0; pstat_itr->pending_stats.size(); v1++) {
                for (int v2 = 1; pstat_itr->pending_stats.size(); v2++) {
                    int v3 = v1 + v2;
                    if ((v1 != v3) && ((pstat_itr->pending_stats[v1].to == pstat_itr->pending_stats[v3].from ||
                            pstat_itr->pending_stats[v1].from == pstat_itr->pending_stats[v3].to) &&
                                       (pstat_itr->pending_stats[v1].to != pstat_itr->pending_stats[v3].to &&
                                               pstat_itr->pending_stats[v1].from != pstat_itr->pending_stats[v3].to))) {
                        // Expire stats in a week.
                        if (now() - pstat_itr->pending_stats[v1].submitted < 604800) {
                            pendingDeletion.push_back(v1);
                        } else if (now() - pstat_itr->pending_stats[v3].submitted < 604800);
                        else {
                            uint64_t verifiedAmount;
                            uint64_t verifiedAmountModifier;
                            if (pstat_itr->pending_stats[v1].amount > pstat_itr->pending_stats[v3].amount) {
                                verifiedAmount = pstat_itr->pending_stats[v3].amount;
                                verifiedAmountModifier =
                                        pstat_itr->pending_stats[v1].amount - pstat_itr->pending_stats[v3].amount;
                                // Search for user account to add
                                // #TODO Update amount to multiplier times active stats
                                action(permission_level{from, "active"_n},
                                       "eosio.token"_n, "transfer"_n,
                                       std::make_tuple(get_self(), to, amount, std::string(""))
                                ).send();
                            } else {
                                verifiedAmount = pstat_itr->pending_stats[v1].amount;
                                verifiedAmountModifier =
                                        pstat_itr->pending_stats[v3].amount - pstat_itr->pending_stats[v1].amount;
                            }
                            _storage.modify(storageIterator, from, [&](auto &storage_stat) {
                                storage_stat.bandwidth_used += amount;
                            });
                        }
                    }
                }
            }
        }
    }

    ACTION hodlong::adduser(const name account, string &pub_key) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator == _users.end(), "A user exist for this account.");

        _users.emplace(account, [&](auto &u) {
            u.account_name = account;
            u.pub_key = pub_key;

        });

    }
    ACTION hodlong::updateuser(const name account, string &pub_key){
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator != _users.end(), "User account does not exist");

        _users.modify(iterator, get_self(), [&](auto &u) {
            u.pub_key = pub_key;

        });
    }

    ACTION hodlong::addseed(name account, name storage_id) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator != _users.end(), "Address for account not found");

        _users.modify(iterator, account, [&](auto &user) {
            user.seeded_objects.push_back(storage_id.value);
        });
    }

    ACTION hodlong::removeseed(const name account, name storageId) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator != _users.end(), "Address for account not found");

        _users.modify(iterator, account, [&](auto &user) {

            auto position = find(user.seeded_objects.begin(), user.seeded_objects.end(), 8);
            if (position != user.seeded_objects.end()) // == myVector.end() means the element was not found
                user.seeded_objects.erase(position);
        });
    }

    ACTION hodlong::transfer(const name from,const  name to, asset quantity, string memo) {
        if (from == get_self() || to != get_self())
            return;
        auto user = _users.find(from.value);

        eosio_assert(user != _users.end(), "User does not exist");

        _users.modify(user, get_self(), [&](auto u) {
            u.balance += quantity;
        });

    }

    ACTION hodlong::removefunds(name to, asset quantity, string memo) {
        auto user = _users.find(to.value);
        eosio_assert(user != _users.end(), "User does not exist");
        eosio_assert(user->balance <= quantity, "You do not have the required balance to remove the funds");
        action(permission_level{get_self(), "active"_n},
               "eosio.token"_n, "transfer"_n,
               std::make_tuple(get_self(), to, quantity, std::string("Transfer of funds out of hodlong account"))
        ).send();

    }

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    if (action == "transfer"_n.value && code == "eosio.token"_n.value) {

        eosio::execute_action(name(receiver), name(code), &hodlong::transfer);
    }

    if (code == receiver) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(hodlong,
                                  (buy)(createobj)(addstats)(adduser)(addseed)(removeseed)(transfer))
        }
    }
    eosio_exit(0);
}
}