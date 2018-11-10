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

            ACTION buy(name buyer, name storage_id);
            ACTION createobj(name account, storage_t newObj);
            ACTION addstats(const name from, const name to, name storage_id, bool seeder, uint64_t amount);
            ACTION addaccount(const name account, string &pub_key);
            ACTION addseed(name account, name storage_id);
            ACTION removeseed(const name account, name storageId);
            ACTION addfunds(name from, name to, asset quantity, string memo);
            ACTION removefunds(name to, asset quantity, string memo);

    };
}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    if (action == "addfunds"_n.value && code != receiver) {
        execute_action(eosio::name(receiver), eosio::name(code), &bpfish::hodlong::addfunds);
    }

    if (code == receiver) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(bpfish::hodlong,
                                  (buy)(createobj)(addstats)(addaccount)(addseed)(removeseed)(addfunds))
        }
    }
    eosio_exit(0);
}
}

