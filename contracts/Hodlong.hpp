#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;
using std::string;
using std::vector;

namespace bpfish{
    CONTRACT hodlong : public eosio::contract{

        public:
            using contract::contract;

            TABLE users_t {
                name account_name;
                string pub_key;
                asset balance;
                vector <uint64_t> owned_objects;
                vector <uint64_t> seeded_objects;

                uint64_t primary_key() const { return account_name.value; }

                EOSLIB_SERIALIZE(users_t, (account_name)(pub_key)(balance)(owned_objects)(seeded_objects));
            };

            TABLE storage_t {
                name storage_id;
                name account;
                string filename;
                string file_size;
                string checksum;
                vector <uint64_t> accepted_seeders;
                uint64_t max_seeders;
                uint64_t bandwidth_used;
                uint64_t primary_key() const { return storage_id.value; }

                EOSLIB_SERIALIZE(storage_t, (storage_id)(account)(filename)(file_size)(checksum)
                    (accepted_seeders)(max_seeders));
            };

            TABLE stat {
                name from;
                name to;
                uint64_t amount;
                time_t submitted;
            };

            TABLE stats_t {
                name stats_id;
                name storage_id;
                name account;
                uint64_t amount;

                uint64_t primary_key() const { return stats_id.value; }

                EOSLIB_SERIALIZE(stats_t, (stats_id)(account)(storage_id)(amount)
                )
            };

            TABLE pstats_t {
                name pstats_id;
                name storage_id;
                vector <stat> pending_stats;

                uint64_t primary_key() const { return pstats_id.value; }

                EOSLIB_SERIALIZE(pstats_t, (pstats_id)(storage_id)(pending_stats)
                )
            };

            typedef eosio::multi_index< "users"_n, users_t > users;
            typedef eosio::multi_index< "stats"_n, stats_t > stats;
            typedef eosio::multi_index< "pstats"_n, pstats_t > pstats;

            typedef eosio::multi_index< "storage"_n, storage_t > storage;

            ACTION buy(name buyer, name storage_id) {
                storage storage(_self, _self.value);

                auto iterator = storage.find(storage_id.value);
                eosio_assert(iterator != storage.end(), "The bid not found");
                eosio_assert(iterator->accepted_seeders.size() >= iterator->max_seeders, "The storage object has the max amount of seeders");
            }
            ACTION createobj(name account, storage_t newObj) {
                require_auth(account.value);

                storage storage(_self, _self.value);

                auto iterator = storage.find(newObj.storage_id.value);
                eosio_assert(iterator == storage.end(), "Obj for this ID already exists");

                storage.emplace(account, [&](auto &storage_obj) {
                    storage_obj = newObj;
                });
            }
            ACTION addstats(const name from, const name to, name storage_id, bool seeder, uint64_t amount) {
                require_auth(from);

                pstats pStats(_self, _self.value);
                time_t date = now();
                stat client_stat = {from, to, amount, seeder};

                auto pstats = pStats.find(storage_id.value);
                bool foundStat = false;
                if (pstats == pStats.end()) {
                    pStats.emplace(storage_id, [&](auto &tmp_stat) {
                        tmp_stat.storage_id = storage_id;
                        tmp_stat.pending_stats.push_back(client_stat);
                    });
                    foundStat = true;
                }
                else{
                    pStats.modify(pstats, from, [&](auto &tmp_stat) {
                        tmp_stat.pending_stats.push_back(client_stat);
                    });
                }
                //may need to break into deferred actions for delete depending on mainnet processing times
                if (foundStat) {
                    storage storage(_self, storage_id.value);
                    auto storageIterator = storage.find(storage_id.value);
                    eosio_assert(storageIterator == storage.end(), "The storage id is not found");

                    vector <uint64_t> pendingDeletion;
                    // Inefficient loop and due to RAM Cost. Cheaper to offload to cpu
                    for (int v1=0; pstats->pending_stats.size(); v1++) {
                        for (int v2=1; pstats->pending_stats.size(); v2++) {
                            int v3 = v1 + v2;
                            if ((v1 != v3) && ((pstats->pending_stats[v1].to == pstats->pending_stats[v3].from || pstats->pending_stats[v1].from == pstats->pending_stats[v3].to) && (pstats->pending_stats[v1].to != pstats->pending_stats[v3].to && pstats->pending_stats[v1].from != pstats->pending_stats[v3].to))) {
                                // Expire stats in a week.
                                if (now() - pstats->pending_stats[v1].submitted < 604800)  {
                                    pendingDeletion.push_back(v1);
                                } else if (now() - pstats->pending_stats[v3].submitted < 604800);
                                else {
                                    uint64_t verifiedAmount;
                                    uint64_t verifiedAmountModifier;
                                    if (pstats->pending_stats[v1].amount > pstats->pending_stats[v3].amount) {
                                        verifiedAmount = pstats->pending_stats[v3].amount;
                                        verifiedAmountModifier = pstats->pending_stats[v1].amount - pstats->pending_stats[v3].amount;
                                        // Search for user account to add
                                        // #TODO Update amount to multiplier times active stats
                                        action(permission_level{ from, "active"_n },
                                                      "eosio.token"_n, "transfer"_n,
                                                      std::make_tuple(_self , to, amount, std::string(""))
                                        ).send();
                                    } else {
                                        verifiedAmount = pstats->pending_stats[v1].amount;
                                        verifiedAmountModifier = pstats->pending_stats[v3].amount - pstats->pending_stats[v1].amount;
                                    }
                                    storage.modify(storageIterator, from, [&](auto &storage_stat) {
                                        storage_stat.bandwidth_used += amount;
                                    });
                                }
                            }
                        }
                    }
                }
            }
            ACTION add(const name account, string &pub_key) {
                require_auth(account);
                users users(_self, _self.value);

                auto iterator = users.find(account.value);
                eosio_assert(iterator == users.end(), "Address for account already exists");

                users.emplace(account, [&](auto &user) {
                    user.account_name = account;
                    user.pub_key = pub_key;

                });

            }
            ACTION addseed(name account, name storage_id) {
                require_auth(account);
                users users(_self, _self.value);

                auto iterator = users.find(account.value);
                eosio_assert(iterator != users.end(), "Address for account not found");

                users.modify(iterator, account, [&](auto& user) {
                    user.seeded_objects.push_back(storage_id.value);
                });
            }
            ACTION removeseed(const name account, name storageId) {
                require_auth(account);
                users users(_self, _self.value);

                auto iterator = users.begin();
                eosio_assert(iterator != users.end(), "Address for account not found");

                users.modify(iterator, account, [&](auto& user) {

                    auto position = find(user.seeded_objects.begin(), user.seeded_objects.end(), 8);
                    if (position != user.seeded_objects.end()) // == myVector.end() means the element was not found
                        user.seeded_objects.erase(position);
                });
            }
            ACTION addfunds(name from, name to, asset quantity, string memo) {
                if (from == _self)
                    return;

                users users(_self, _self.value);
                auto user = users.find(from.value);
                eosio_assert (user != users.end(), "User does not exist");

                // If using a different token, please update name
                if (_code != name("EOS"_n))
                    return;
                users.modify(user, from, [&](auto& newuser) {
                    newuser.balance += quantity;
                });

            }
            ACTION removefunds(name to, asset quantity, string memo){
                users users(_self, _self.value);
                auto user = users.find(to.value);
                eosio_assert(user != users.end(), "User does not exist");
                eosio_assert(user->balance <= quantity, "You do not have the required balance to remove the funds");
                action(permission_level{ _self, "active"_n },
                        "eosio.token"_n, "transfer"_n,
                        std::make_tuple(_self, to, quantity, std::string("Transfer of funds out of hodlong contract"))
                ).send();

            }


    };
}
extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        if (action == "addfunds"_n.value && code != receiver) {
            execute_action(eosio::name(receiver), eosio::name(code), &bpfish::hodlong::addfunds);
        }

        if (code == receiver) {
            switch (action) {
                EOSIO_DISPATCH_HELPER(bpfish::hodlong, (buy)(createobj)(addstats)(add)(addseed)(removeseed)(addfunds))
            }
        }
        eosio_exit(0);
    }
}
