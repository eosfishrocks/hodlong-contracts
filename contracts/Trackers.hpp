#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <string>
#include <vector>

using namespace eosio;
using std::string;

namespace bpfish {
    CONTRACT trackers : public eosio::contract {
    public:
        trackers( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ):
            eosio::contract(receiver, code, ds),  _webtrackers(receiver, code.value)
        {}

        ACTION add(const name account, string& url);
        
        ACTION remove(const name account);

        ACTION update(const name account, string& url);



        TABLE wtracker_t {
            string url;
            name account;

            uint64_t primary_key() const { return account.value; }

            EOSLIB_SERIALIZE(wtracker_t, (url)(account));
       };
      typedef eosio::multi_index< "wtrackers"_n, wtracker_t > wtrackers;
      wtrackers _webtrackers;
    };
};
EOSIO_DISPATCH(bpfish::trackers, (add)(remove)(update));