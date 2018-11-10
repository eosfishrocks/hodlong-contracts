#include "Hodlong.hpp"

namespace bpfish {
    void hodlong::buy(name buyer, name storage_id) {
        storage storage_table(_self, _self.value);

        auto iterator = storage_table.find(storage_id.value);
        eosio_assert(iterator != storage_table.end(), "The bid not found");
        eosio_assert(iterator->accepted_seeders.size() >= iterator->max_seeders,
                     "The storage object has the max amount of seeders");
    }

    void hodlong::createobj(name account, storage_t newObj) {
        require_auth(account.value);

        storage storage_table(_self, _self.value);

        auto iterator = storage_table.find(newObj.storage_id.value);
        eosio_assert(iterator == storage_table.end(), "Obj for this ID already exists");

        storage_table.emplace(account, [&](auto &storage_obj) {
            storage_obj = newObj;
        });
    }

    void hodlong::addstats(const name from, const name to, name storage_id, bool seeder, uint64_t amount) {
        require_auth(from);

        pstats pstats_table(_self, _self.value);
        time_t date = now();
        stat
        client_stat = {from, to, amount, seeder};

        auto pstat_itr = pstats_table.find(storage_id.value);
        bool foundStat = false;
        if (pstat_itr == pstats_table.end()) {
            pstats_table.emplace(storage_id, [&](auto &tmp_stat) {
                tmp_stat.storage_id = storage_id;
                tmp_stat.pending_stats.push_back(client_stat);
            });
            foundStat = true;
        } else {
            pstats_table.modify(pstat_itr, from, [&](auto &tmp_stat) {
                tmp_stat.pending_stats.push_back(client_stat);
            });
        }
        //may need to break into deferred actions for delete depending on mainnet processing times
        if (foundStat) {
            storage storage_table(_self, storage_id.value);
            auto storageIterator = storage_table.find(storage_id.value);
            eosio_assert(storageIterator == storage_table.end(), "The storage id is not found");

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
                                       std::make_tuple(_self, to, amount, std::string(""))
                                ).send();
                            } else {
                                verifiedAmount = pstat_itr->pending_stats[v1].amount;
                                verifiedAmountModifier =
                                        pstat_itr->pending_stats[v3].amount - pstat_itr->pending_stats[v1].amount;
                            }
                            storage_table.modify(storageIterator, from, [&](auto &storage_stat) {
                                storage_stat.bandwidth_used += amount;
                            });
                        }
                    }
                }
            }
        }
    }

    void hodlong::addaccount(const name account, string &pub_key) {
        require_auth(account);
        users users_table(_self, _self.value);

        auto iterator = users_table.find(account.value);
        eosio_assert(iterator == users_table.end(), "Address for account already exists");

        users_table.emplace(account, [&](auto &user) {
            user.account_name = account;
            user.pub_key = pub_key;

        });

    }

    void hodlong::addseed(name account, name storage_id) {
        require_auth(account);
        users users_table(_self, _self.value);

        auto iterator = users_table.find(account.value);
        eosio_assert(iterator != users_table.end(), "Address for account not found");

        users_table.modify(iterator, account, [&](auto &user) {
            user.seeded_objects.push_back(storage_id.value);
        });
    }

    void hodlong::removeseed(const name account, name storageId) {
        require_auth(account);
        users users_table(_self, _self.value);

        auto iterator = users_table.find(account.value);
        eosio_assert(iterator != users_table.end(), "Address for account not found");

        users_table.modify(iterator, account, [&](auto &user) {

            auto position = find(user.seeded_objects.begin(), user.seeded_objects.end(), 8);
            if (position != user.seeded_objects.end()) // == myVector.end() means the element was not found
                user.seeded_objects.erase(position);
        });
    }

    void hodlong::addfunds(name from, name to, asset quantity, string memo) {
        if (from == _self)
            return;

        users users_table(_self, _self.value);
        auto user = users_table.find(from.value);
        eosio_assert(user != users_table.end(), "User does not exist");

        // If using a different token, please update name
        if (_code != name("EOS"_n))
            return;
        users_table.modify(user, from, [&](auto &newuser) {
            newuser.balance += quantity;
        });

    }

    void hodlong::removefunds(name to, asset quantity, string memo) {
        users users_table(_self, _self.value);
        auto user = users_table.find(to.value);
        eosio_assert(user != users_table.end(), "User does not exist");
        eosio_assert(user->balance <= quantity, "You do not have the required balance to remove the funds");
        action(permission_level{_self, "active"_n},
               "eosio.token"_n, "transfer"_n,
               std::make_tuple(_self, to, quantity, std::string("Transfer of funds out of hodlong account"))
        ).send();

    }


};
