#pragma once
#include <eosiolib/eosio.hpp>
using namespace eosio;
using std::string;
using std::vector;

class Hodlong : public eosio::contract{

    public: Hodlong(uint64_t self) : eosio::contract(self){}

        TABLE users {
            uint64_t account_name;
            string pub_key;
            vector <uint64_t> ownedObjects;
            vector <uint64_t> seededObjects;

            uint64_t primary_key() const { return account_name; }

            EOSLIB_SERIALIZE(users, (account_name)(pub_key)(ownedObjects)(seededObjects));
        };

        TABLE storage {
            uint64_t storage_id;
            uint64_t account;
            string filename;
            string file_size;
            string checksum;
            vector <uint64_t> acceptedSeeders;
            uint64_t primary_key() const { return account; }

            EOSLIB_SERIALIZE(storage, (account)(filename)(file_size)(checksum)
            );
        };

        TABLE stat {
            uint64_t from;
            uint64_t to;
            uint64_t amount;
            time_t submitted;
            bool seeder;
        };

        TABLE stats {
            uint64_t storage_id;
            uint64_t amount;

            uint64_t primary_key() const { return storage_id; }

            EOSLIB_SERIALIZE(stats, (storage_id)(amount)
            )
        };

        TABLE pstats {
            uint64_t storage_id;
            vector <stat> pending_stats;

            uint64_t primary_key() const { return storage_id; }

            EOSLIB_SERIALIZE(pstats, (storage_id)(pending_stats)
            )
        };

        typedef eosio::multi_index<N(users), users > userIndex;
        typedef eosio::multi_index< "storage"_n, Hodlong::storage > storageIndex;
        typedef eosio::multi_index< "stats"_n, Hodlong::stats > statsIndex;
        typedef eosio::multi_index< "pstats"_n, Hodlong::pstats > pStatsIndex;


        ACTION buy(uint64_t buyer, uint64_t bidId);


        ACTION createobj(uint64_t account, storage obj);

        
        ACTION addstats(const uint64_t from, const uint64_t to, uint64_t storage_id, bool seeder, uint64_t amount);

        
        ACTION deletestats(const uint64_t account, uint64_t storage_id);

        
        ACTION add(const uint64_t account, string &pub_key);

        
        ACTION createobj(const uint64_t account, uint64_t storageId);

        
        ACTION addseed(const uint64_t account, uint64_t storageId);

        
        ACTION removeseed(const uint64_t account, uint64_t storageId);

    }

    EOSIO_DISPATCH(hodlong,(buy)(createobj)(addstats)(deletestats)(add));
}