#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <string>

namespace hodlong {
    using namespace eosio;
    using std::string;

    class Marketplace : public contract {
        using contract::contract;

    public:

        Marketplace(account_name self) : contract(self) {}

        // @abi table storage i64
        struct storage {
            uint64_t storage_id;
            account_name account;
            string filename;
            string file_size;
            string checksum;
            uint64_t bandwidth_used;
            uint64_t bandwidth_rate;
            uint64_t bandwidth_allocated;
            uint64_t max_seeders;
            uint64_t paid_rate;
            vector <uint64_t> accepted_seeders;
            uint64_t primary_key() const { return storage_id; }

            EOSLIB_SERIALIZE(storage, (account)(filename)(file_size)(checksum)(bandwidth_used)(bandwidth_rate)(bandwidth_allocated)(max_seeders)(paid_rate)(accepted_seeders));
        };
        typedef multi_index<N(marketplace), storage> storageIndex;

        struct stat {
            uint64_t from;
            uint64_t to;
            uint64_t amount;
            time_t submitted;
            bool seeder;
        };

        // @abi table marketplace i64
        struct stats {
            uint64_t storage_id;
            uint64_t amount;

            uint64_t primary_key() const { return storage_id; }
            EOSLIB_SERIALIZE(stats, (storage_id)(amount))
        };
        typedef multi_index<N(marketplace), stats> statsIndex;

        // @abi table marketplace i64
        struct pStats {
            uint64_t storage_id;
            vector <stat> pending_stats;

            uint64_t primary_key() const { return storage_id; }
            EOSLIB_SERIALIZE(pStats, (storage_id)(pending_stats))
        };
        typedef multi_index<N(marketplace), pStats> pStatsIndex;


        // @abi action
        void buy(account_name buyer, uint64_t bidId);

        // @abi action
        void createobj(account_name account, storage obj);

        // @abi action
        void addstats(const account_name from, const account_name to, uint64_t storage_id, bool seeder, uint64_t amount);

        void apply(uint64_t receiver, uint64_t code, uint64_t action);

    private:
        uint64_t abs64(uint64_t value);
        uint64_t max64(uint64_t value1, uint64_t value2);
    };

    //EOSIO_ABI(Marketplace,(buy)(createobj));
}