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
               eosio::contract(receiver, code, ds), _pstats(receiver, code.value), _pstats_storage(receiver, code.value),
                _stats(receiver, code.value),_storage(receiver, code.value), _users(receiver, code.value)
            {}
            // Global contract name for transfers from eosio.token
            name contract_name = name("hodlong");
            string symbol_name = "SYS";

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
                vector <name> approved_seeders;
                uint64_t max_seeders;
                uint64_t bandwidth_used;
                bool self_host;
                bool secure;
                uint64_t bandwidth_cost;
                uint64_t bandwidth_divisor;

                uint64_t primary_key() const { return storage_id; }

                EOSLIB_SERIALIZE(storage_t, (storage_id)(account)(filename)(file_size)(checksum)
                    (approved_seeders)(max_seeders)(bandwidth_used)(self_host)(secure)(bandwidth_cost)(bandwidth_divisor));
            };
            // Generic Stat Object
            TABLE stat {
                name authority;
                name from;
                name to;

                uint64_t amount;
                time_t date;
                uint64_t primary_key() const { return authority.value + from.value + to.value; }
            };
            // Pending Stats to be Paid
            TABLE stats_t {
                uint64_t storage_id;
                name account;
                uint64_t amount;
                bool negative;

                uint64_t primary_key() const { return storage_id; }

                    EOSLIB_SERIALIZE(stats_t, (storage_id)(account)(amount)(negative));
            };
            // Pending Stats that haven't been processed
            TABLE pstats_t {
                uint64_t pstats_id;
                uint64_t storageid;
                vector <stat> pending_stats;

                uint64_t primary_key() const { return pstats_id; }
                uint64_t by_storage_id() const { return storageid; }
                EOSLIB_SERIALIZE(pstats_t, (pstats_id)(storageid)(pending_stats));
            };

            typedef multi_index< "users"_n, users_t > users;
            typedef multi_index< "stats"_n, stats_t > stats;
            typedef multi_index< "pstats"_n, pstats_t > pstats;
            typedef multi_index< "storage"_n, storage_t > storage;
            typedef multi_index< "pstats"_n, pstats_t, indexed_by<"storageid"_n, const_mem_fun<pstats_t, uint64_t,
                    &pstats_t::by_storage_id>>> pstats_storage;

            //add an approved seeder
            ACTION addas(name authority, uint64_t storage_id, name seeder);
            // main stat collating and balance transfer method
            ACTION addstats(const name authority, const name from, const name to, uint64_t storage_id, uint64_t amount);
            // add new user to the account/pubkey ledger
            ACTION adduser(const name account, string &pub_key);
            // create new storage object
            ACTION createobj(name account, string &filename, string &filesize, string &checksum,
                    vector<name> approved_seeders, uint64_t max_seeders, bool self_host, bool secure, uint64_t bandwidth_cost,
                    uint64_t bandwidth_divisor);
            //remove approved seeder from object
            ACTION removeas(name authority, uint64_t storage_id, name seeder);
            // Remove funds from account to users account
            ACTION removefunds(name to, asset quantity);
            // Remove object
            ACTION removeo(name authority, uint64_t storage_id);
            // Accept an object to be seeded by a paid user
            ACTION seed(name buyer, uint64_t storage_id);
            // action from eosio.token to contract
            ACTION transfer(name from, name to, asset quantity, string memo);
            // Update users pubkey
            ACTION updateuser(name account, string &pub_key);

            pstats _pstats;
            pstats_storage _pstats_storage;
            stats _stats;
            storage _storage;
            users _users;
    };
}