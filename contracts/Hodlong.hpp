#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/symbol.hpp>


using namespace eosio;
using std::string;
using std::vector;

namespace bpfish{
    CONTRACT hodlong : public eosio::contract{
        public:
            hodlong( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ):
               eosio::contract(receiver, code, ds), _storage(receiver, code.value),
                _pstats(receiver, code.value), _users(receiver, code.value)
            {}

            TABLE users_t {
                name account;
                string pub_key;
                asset balance;
                vector <uint64_t> owned_objects;
                vector <uint64_t> seeded_objects;

                uint64_t primary_key() const { return account.value; }

                EOSLIB_SERIALIZE(users_t, (account)(pub_key)(balance)(owned_objects)(seeded_objects));
            };

            TABLE storage_t {
                uint64_t storage_id;
                name account;
                string filename;
                string file_size;
                string checksum;
                vector <name> accepted_seeders;
                uint64_t max_seeders;
                uint64_t bandwidth_used;
                bool self_host;
                uint64_t primary_key() const { return storage_id; }

                EOSLIB_SERIALIZE(storage_t, (storage_id)(account)(filename)(file_size)(checksum)
                    (accepted_seeders)(max_seeders)(bandwidth_used)(self_host));
            };

            TABLE stat {
                name from;
                name to;
                uint64_t amount;
                time_t submitted;
            };

            TABLE stats_t {
                name storage;
                name account;
                uint64_t amount;

                uint64_t primary_key() const { return storage.value; }

                EOSLIB_SERIALIZE(stats_t, (account)(storage)(amount)
                )
            };

            TABLE pstats_t {
                uint64_t pstats_id;
                uint64_t storage_id;
                vector <stat> pending_stats;

                uint64_t primary_key() const { return pstats_id; }

                EOSLIB_SERIALIZE(pstats_t, (pstats_id)(storage_id)(pending_stats)
                )
            };

            typedef multi_index< "users"_n, users_t > users;
            typedef multi_index< "stats"_n, stats_t > stats;
            typedef multi_index< "pstats"_n, pstats_t > pstats;
            typedef multi_index< "storage"_n, storage_t > storage;

            ACTION buy(name buyer, name storage_id);
            ACTION createobj(name account, string &filename, string &filesize, string &checksum,
                    vector<name> accepted_seeders, uint64_t max_seeders, bool self_host);
            ACTION addstats(const name from, const name to, uint64_t storage_id, bool seeder, uint64_t amount);
            ACTION adduser(const name account, string &pub_key);
            ACTION addseed(name account, name storage_id);
            ACTION removeseed(const name account, name storageId);
            ACTION transfer(name from, name to, asset quantity, string memo);
            ACTION removefunds(name to, asset quantity, string memo);
            ACTION updateuser(name account, string &pub_key);
            storage _storage;
            pstats _pstats;
            users _users;
    };
}