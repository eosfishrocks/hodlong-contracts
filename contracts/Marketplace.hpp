

namespace Hodlong {
    using namespace eosio;
    using std::string;

    class Hodlong : public contract {
        using contract::contract;

    public:

        Marketplace(account_name self) : contract(self) {}

        struct [[eosio::table]] storage {
            uint64_t storage_id;
            account_name account;
            string filename;
            string file_size;
            string checksum;
            vector <uint64_t> acceptedSeeders;
            uint64_t primary_key() const { return account; }

            EOSLIB_SERIALIZE(storage, (account)(filename)(file_size)(checksum)
            );
        };

        typedef multi_index<N(marketplace), storage> storageIndex;
        struct [[eosio::table]] stat {
            uint64_t from;
            uint64_t to;
            uint64_t amount;
            time_t submitted;
            bool seeder;
        };

        struct [[eosio::table]] stats {
            uint64_t storage_id;
            uint64_t amount;

            uint64_t primary_key() const { return storage_id; }

            EOSLIB_SERIALIZE(stats, (storage_id)(amount)
            )
        };

        typedef multi_index<N(marketplace), stats> statsIndex;

        struct [[eosio::table]] pStats {
            uint64_t storage_id;
            vector <stat> pending_stats;

            uint64_t primary_key() const { return storage_id; }

            EOSLIB_SERIALIZE(pStats, (storage_id)(pending_stats)
            )
        };

        typedef multi_index<N(marketplace), pStats> pStatsIndex;


        [[eosio::action]]
        void buy(account_name buyer, uint64_t bidId);

        [[eosio::action]]
        void createobj(account_name account, storage obj);

        [[eosio::action]]
        void
        addstats(const account_name from, const account_name to, uint64_t storage_id, bool seeder, uint64_t amount);

        [[eosio::action]]
        void deletestats(const account_name account, uint64_t storage_id);

        [[eosio::action]]
        void add(const account_name account, string &pub_key);

        [[eosio::action]]
        void createobj(const account_name account, uint64_t storageId);

        [[eosio::action]]
        void addseed(const account_name account, uint64_t storageId);

        [[eosio::action]]
        void removeseed(const account_name account, uint64_t storageId);

    private:
        uint64_t abs64(uint64_t value);

        uint64_t max64(uint64_t value1, uint64_t value2);
    }

    EOSIO_ABI(Hodlong,(add)(createobj)(addseed)(removeseed)(buy)(createobj));
}