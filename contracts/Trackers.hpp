#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <string>
#include <vector>

using namespace eosio;
using std::string;

namespace bpfish {
    CONTRACT trackers : public eosio::contract {

    public:
        using contract::contract;

        ACTION add(const name account, string& url);
        
        ACTION remove(const name account);
        
        ACTION update(const name account, string& url);

    private:

        TABLE wtrackers_t {
            uint64_t tracker_id;
            string url;
            name account;

            uint64_t primary_key() const { return tracker_id; }

            EOSLIB_SERIALIZE(wtrackers_t, (tracker_id)(url)(account));
        };
        typedef eosio::multi_index< "wtracker"_n, wtrackers_t > wtrackers;
    };

    EOSIO_DISPATCH(bpfish::trackers, (add)(remove)(update));
}