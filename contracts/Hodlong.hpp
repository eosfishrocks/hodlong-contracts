#pragma once
#include <eosiolib/eosio.hpp>

namespace bpfish{
    using namespace eosio;
    using std::string;
    using std::vector;

    CONTRACT hodlong : public eosio::contract{

        public:
            using contract::contract;

            TABLE users {
                name account_name;
                string pub_key;
                vector <uint64_t> owned_objects;
                vector <uint64_t> seeded_objects;

                uint64_t primary_key() const { return account_name.value; }

                EOSLIB_SERIALIZE(users, (account_name)(pub_key)(owned_objects)(seeded_objects));
            };

            TABLE storage {
                name storage_id;
                name account;
                string filename;
                string file_size;
                string checksum;
                vector <uint64_t> accepted_seeders;
                uint64_t max_seeders;
                uint64_t bandwidth_used;
                uint64_t primary_key() const { return storage_id.value; }

                EOSLIB_SERIALIZE(storage, (storage_id)(account)(filename)(file_size)(checksum)
                    (accepted_seeders)(max_seeders));
            };

            TABLE stat {
                name from;
                name to;
                uint64_t amount;
                time_t submitted;
                bool seeder;
            };

            TABLE stats {
                name stats_id;
                name storage_id;
                uint64_t amount;

                uint64_t primary_key() const { return stats_id.value; }

                EOSLIB_SERIALIZE(stats, (stats_id)(storage_id)(amount)
                )
            };

            TABLE pstats {
                name pstats_id;
                name storage_id;
                vector <stat> pending_stats;

                uint64_t primary_key() const { return pstats_id.value; }

                EOSLIB_SERIALIZE(pstats, (pstats_id)(storage_id)(pending_stats)
                )
            };

            typedef eosio::multi_index< "users"_n, users > userIndex;
            typedef eosio::multi_index< "stats"_n, stats > statsIndex;
            typedef eosio::multi_index< "pstats"_n, pstats > pStatsIndex;

            typedef eosio::multi_index< "storage"_n, storage > storageIndex;

            ACTION buy(name buyer, name storage_id) {
                storageIndex storage(_self, storage_id.value);

                auto iterator = storage.find(storage_id.value);
                eosio_assert(iterator != storage.end(), "The bid not found");
                eosio_assert(iterator->accepted_seeders.size() >= iterator->max_seeders, "The storage object has the max amount of seeders");
            }
            ACTION createobj(name account, storage newObj) {
                require_auth(account.value);

                storageIndex objs(_self, account.value);

                auto iterator = objs.find(newObj.storage_id.value);
                eosio_assert(iterator == objs.end(), "Obj for this ID already exists");

                objs.emplace(account, [&](auto &obj) {
                    obj = newObj;
                });
            }
            ACTION addstats(const name from, const name to, name storage_id, bool seeder, uint64_t amount) {
                require_auth(from);

                pStatsIndex pStats(_self, storage_id.value);
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
                    storageIndex storage(_self, storage_id.value);
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
                userIndex users(_self, account.value);

                auto iterator = users.find(account.value);
                eosio_assert(iterator == users.end(), "Address for account already exists");

                users.emplace(account, [&](auto &user) {
                    user.account_name = account;
                    user.pub_key = pub_key;

                });

            }
            ACTION addseed(name account, name storage_id) {
                require_auth(account);
                userIndex users(_self, storage_id.value);

                auto iterator = users.find(account.value);
                eosio_assert(iterator != users.end(), "Address for account not found");

                users.modify(iterator, account, [&](auto& user) {
                    user.seeded_objects.push_back(storage_id.value);
                });
            }
            ACTION removeseed(const name account, name storageId) {
                require_auth(account);
                userIndex users(_self, account.value);

                auto iterator = users.begin();
                eosio_assert(iterator != users.end(), "Address for account not found");

                users.modify(iterator, account, [&](auto& user) {

                    auto position = find(user.seeded_objects.begin(), user.seeded_objects.end(), 8);
                    if (position != user.seeded_objects.end()) // == myVector.end() means the element was not found
                        user.seeded_objects.erase(position);
                });
            }
    };
}
EOSIO_DISPATCH(bpfish::hodlong, (buy)(createobj)(addstats)(add)(addseed)(removeseed))