#include "Marketplace.hpp"

namespace hodlong {
    void Marketplace::buy(account_name buyer, uint64_t storage_id) {
        storageIndex storage(_self, _self);

        auto iterator = storage.find(storage_id);
        eosio_assert(iterator != storage.end(), "The bid not found");
        eosio_assert(iterator->accepted_seeders.size() >= iterator->max_seeders, "The storage object has the max amount of seeders");
    }
    void Marketplace::createobj(account_name account, hodlong::Marketplace::storage newObj) {
        require_auth(account);

        storageIndex objs(_self, _self);

        auto iterator = objs.find(newObj.storage_id);
        eosio_assert(iterator == objs.end(), "Obj for this ID already exists");

        objs.emplace(account, [&](auto &obj) {
            obj = newObj;
        });
    }
    void Marketplace::addstats(const account_name from, const account_name to, uint64_t storage_id, bool seeder, uint64_t amount) {
        require_auth(from);

        pStatsIndex pStats(_self, _self);
        time_t date = now();
        stat client_stat = {from, amount, seeder};

        auto pstats = pStats.find(storage_id);
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
            storageIndex storage(_self, _self);
            auto storageIterator = storage.find(storage_id);
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
                            storage.modify(storageIterator, from, [&](auto &newStat) {
                                newStat.bandwidth_used += amount;
                            });
                        }
                    }
                }
            }
        }
    }
    uint64_t Marketplace::max64(uint64_t value1, uint64_t value2) {
        if (value1 > value2) return value1;
        else return value2;
    }

    void deletestats(const account_name account, uint64_t storage_id) {
        require_auth(account);
        storageIndex storage(_self, _self);

        auto iterator = objs.find(newObj.storage_id);
        eosio_assert(iterator != storage.end(), "Obj does not exist");
        storage.modify(iterator, account, [&](auto& storage_object) { storage.inventory.erase(i); });
    }


    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        if (code == N(eosio.token) && action == N(transfer)) {
            // Do action
        }
    }
}