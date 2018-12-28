#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/transaction.hpp>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "hodlong"
#endif

#ifndef SYMBOL_NAME
#define SYMBOL_NAME "SYS"
#endif


#ifndef TRANSFER_DELAY
#define TRANSFER_DELAY 60*60
#endif

#ifndef TOKEN_CONTRACT
#define TOKEN_CONTRACT "eosio.token"
#endif


using namespace eosio;
using std::string;
using std::vector;

namespace bpfish{
    CONTRACT hodlong : public eosio::contract{
        public:
            hodlong( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ):
               eosio::contract(receiver, code, ds), _pstats(receiver, code.value), _stats(receiver, code.value),
                _storage(receiver, code.value), _users(receiver, code.value)
            {}

            TABLE users_t {
                name account;
                string pub_key;
                asset balance;
                vector <uint64_t> owned_objects;
                vector <uint64_t> seeded_objects;
                vector <name> allowed_storage_providers;
                bool provider;

                uint64_t primary_key() const { return account.value; }
                uint64_t is_provider() const { return provider; };

                EOSLIB_SERIALIZE(users_t, (account)(pub_key)(balance)(owned_objects)(seeded_objects)
                    (allowed_storage_providers)(provider));
            };

            TABLE storage_t {
                uint64_t storage_id;
                name account;
                string filename;
                string file_size;
                vector <name> approved_seeders;
                vector <name> allowed_users;
                uint64_t max_seeders;
                uint64_t bandwidth_used;
                bool self_host;
                uint64_t bandwidth_cost;
                uint64_t bandwidth_divisor;
                // Open seeds
                uint64_t oseeds;
                uint64_t poscheck=0;
                name pos_verifier;

                uint64_t primary_key() const { return storage_id; }
                uint64_t need_seeds() const { return oseeds; }
                uint64_t pos_check() const { return poscheck; }

                EOSLIB_SERIALIZE(storage_t, (storage_id)(account)(filename)(file_size)
                    (approved_seeders)(max_seeders)(bandwidth_used)(self_host)(bandwidth_cost)
                    (bandwidth_divisor)(oseeds)(poscheck)(pos_verifier));
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


            typedef multi_index< "pstats"_n, pstats_t, indexed_by<"storageid"_n, const_mem_fun<pstats_t, uint64_t,
                &pstats_t::by_storage_id>>> pstats;
            typedef multi_index< "stats"_n, stats_t > stats;
            typedef multi_index< "storage"_n, storage_t, indexed_by<"oseeds"_n, const_mem_fun<storage_t, uint64_t,
                &storage_t::need_seeds>>> storage;
            typedef multi_index< "users"_n, users_t, indexed_by<"provider"_n, const_mem_fun<users_t, uint64_t,
                    &users_t::is_provider>>> users;


            /**
             * Action to add allowed storage provider to a user.
             * Users may to opt in to specific providers. If not set any storage provider is available to be added.
             *
             * @param authority The account with the authority to add a storage provider to the user
             * @param sp The account of the storage provider
             */
            ACTION addallowed(const name authority, const name sp);

            /**
             * Action to add an approved seeder to a storage object.
             * Users with the storage provider authority add themselves as a paid seeder to an object.
             *
             * @param authority The account with the authority to add themselves as a storage provider. to an object.
             * @param storage_id The storage id of storage object.
             * @param provider The account of the storage provider to be added to the object.
             */
            ACTION addas(name authority, uint64_t storage_id, name provider);

            /**
             * Action to add statistics from the transfer of data off the blockchain.
             * Users with Hodlong Clients add stats to the object.
             *
             * @param authority The account that is sending the statistics.
             * @param from The account that sent the storage object data.
             * @param to The account that received the storage object data.
             * @param storage_id The storage id of the storage object.
             * @param amount The amount of bytes transferred.
             */
            ACTION addstats(const name authority, const name from, const name to, uint64_t storage_id, uint64_t amount);

            /**
             * Action to add a user to the hodlong contract.
             * New users use this action to create their account.
             *
             * @param account The account to be added to the hodlong users table.
             * @param pub_key The public key of the account used for offchain verification
             */
            ACTION adduser(const name account, string &pub_key);

            /**
             * Action to create a new storage object.
             * Users create objects.
             *
             * @param account The account creating the storage object.
             * @param filename The name of the storage object(infohash)
             * @param filesize The size of the storage object in bytes.
             * @param approved_seeders The list of seeders that can get paid or transfer the storage object
             * @param allowed_users If set, the list of non storage providers that can transfer the storage object.
             * @param max_seeders The maximum amount of storage providers that will host the storage object.
             * @param self_host If true, doesn't allow storage providers to bid on object.
             * @param bandwidth_cost The value the user is willing to pay for the bandwidth on the storage object per divisor.
             * @param bandwidth_divisor The modifier for the bandwidth_cost value. IE: 0.1 SYS/1,000,000,000bytes.
             */
            ACTION createobj(name account, string &filename, string &filesize, vector<name> approved_seeders,
                    vector<name> allowed_users, uint64_t max_seeders, bool self_host,  uint64_t bandwidth_cost, uint64_t bandwidth_divisor);

            /**
             * Action to modify the storage providers authority.
             * Admins modify storage providers
             *
             * @param account The account witg creating the storage object.
             * @param provider Sets a user as a provider.
             */
            ACTION modifysp(const name user, bool provider);

            /**
             * Action to remove an approved seeder from an object.
             * Users and storage providers can remove approved seeders.
             *
             * @param authority The account with the authority to change the object
             * @param storage_id The storage id of the storage object.
             * @param seeder The seeder to be removed.
             */
            ACTION removeas(name authority, uint64_t storage_id, name seeder);

            /**
             * Action to transfer funds from balance to user.
             * Users can withdrawal funds.
             *
             * @param authority The account with the authority to change the object
             * @param storage_id The storage id of the storage object.
             * @param seeder The seeder to be removed.
             */
            ACTION removefunds(name to, asset quantity);

            /**
             * Action to remove object from network.
             * Admins can remove objects. #TODO Users
             *
             * @param authority The account with the authority to remove the object.
             * @param storage_id The storage id of the storage object.
             */
            ACTION removeo(name authority, uint64_t storage_id);

            /**
             * Action to add a seeder to an object.
             * Storage providers seed objects.
             * @param buyer
             * @param storage_id
             */
            ACTION seed(name buyer, uint64_t storage_id);

            /**
             * Action to update a user's private key.
             * Users update private keys.
             * @param buyer
             * @param storage_id
             */
            ACTION updateuser(name account, string &pub_key);


            /**
             * Public function called when token contract transfers tokens to this contract.
             */
            void transfer(name from, name to, asset quantity, string memo);

            pstats _pstats;
            stats _stats;
            storage _storage;
            users _users;
    };
}