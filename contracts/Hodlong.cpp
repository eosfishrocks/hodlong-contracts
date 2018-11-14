#include "Hodlong.hpp"
namespace bpfish{
    ACTION hodlong::buy(name buyer, uint64_t storage_id) {
        auto aiter= _users.find(buyer.value);
        auto iterator = _storage.find(storage_id);
        eosio_assert(aiter!= _users.end(), "User does not exist");
        eosio_assert(iterator != _storage.end(), "The bid not found");
        eosio_assert(iterator->accepted_seeders.size() <= iterator->max_seeders,
                     "The storage object has the max amount of seeders");
        _storage.modify(iterator, get_self(), [&](auto &u) {
            u.accepted_seeders.push_back(buyer);
        });
        _users.modify(aiter, get_self(), [&](auto &u) {
            u.seeded_objects.push_back(storage_id);
        });
    }


    ACTION hodlong::createobj(name account, string &filename, string &file_size, string &checksum,
            vector<name> accepted_seeders, uint64_t max_seeders, bool self_host, uint64_t bandwidth_cost) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator != _users.end(), "User does not exist.");

        uint64_t storage_id = _storage.available_primary_key();
        _storage.emplace(get_self(), [&](auto &s) {
            s.storage_id = storage_id;
            s.account = account;
            s.filename = filename;
            s.file_size = file_size;
            s.checksum = checksum;
            s.max_seeders = max_seeders;
            s.self_host = self_host;
            s.bandwidth_used = 0;
            s.accepted_seeders = vector<name>();
            s.bandwidth_cost = bandwidth_cost;
        });
        _users.modify(iterator, account, [&](auto &u) {
            u.owned_objects.push_back(storage_id);
        });
    }
    ACTION hodlong::addstats(const name authority, const name from, const name to, uint64_t storage_id, uint64_t amount) {
        require_auth(authority);
        time_t date = now();
        stat client_stat = {authority, from, to, amount, date};
        bool found_stat = false;
        bool users_are_paid = false;
        auto siterator = _storage.find(storage_id);
        eosio_assert(siterator != _storage.end(), "The storage id is not found");

        for (int s0 = 0; s0 < siterator->accepted_seeders.size(); s0++){
            if (siterator->accepted_seeders[s0] == from || siterator->accepted_seeders[s0] == to){
                users_are_paid= true;
            }
        }
        auto pstat_itr = _pstats_storage.lower_bound(storage_id);

        if (pstat_itr == _pstats_storage.end() && users_are_paid) {
            _pstats.emplace(get_self(), [&](auto &tmp_stat) {
                tmp_stat.storageid = storage_id;
                tmp_stat.pending_stats.push_back(client_stat);
            });
        } else if (users_are_paid){
            _pstats_storage.modify(pstat_itr, get_self(), [&](auto &tmp_stat) {
                tmp_stat.pending_stats.push_back(client_stat);
            });
            found_stat = true;
        }

        //may need to break into deferred actions for delete depending on mainnet processing times
        if (found_stat && users_are_paid) {
            eosio_assert(from == authority || to == authority, "Neither to or from is the authority");
            vector <uint64_t> pending_deletion;
            //  #TODO Add multi index secondary key to cut processing times for envs with many users.
            for (int v1 = 0; v1 < pstat_itr->pending_stats.size(); v1++) {
                int v3 = v1 + 1;
                print(std::to_string(now() - pstat_itr->pending_stats[v1].submitted));
                if (now() - pstat_itr->pending_stats[v1].submitted > 604800) {
                    pending_deletion.push_back(v1);
                    break;
                }

                for (int v2 = 0; v2 < pstat_itr->pending_stats.size(); v2++) {
                    if (v2 == 0) {
                        v2 = v3;
                    }
                    eosio::print(std::to_string(v1+v2));
                    print(std::to_string(v2+v1));
                    if (pstat_itr->pending_stats[v1].to == pstat_itr->pending_stats[v3].to &&
                        pstat_itr->pending_stats[v1].from == pstat_itr->pending_stats[v3].from &&
                        pstat_itr->pending_stats[v1].authority != pstat_itr->pending_stats[v3].authority &&
                        pstat_itr->pending_stats[v1].amount !=0 && pstat_itr->pending_stats[v2].amount != 0) {
                            uint64_t verifiedAmount;
                            uint64_t verifiedAmountModifier;
                        if (pstat_itr->pending_stats[v1].amount == pstat_itr->pending_stats[v3].amount) {
                            verifiedAmount = pstat_itr->pending_stats[v1].amount;
                            _pstats_storage.modify(pstat_itr, get_self(), [&](auto &s) {
                                s.pending_stats[v1].amount = 0;
                                s.pending_stats[v3].amount = 0;
                            });
                            pending_deletion.push_back(v1);
                            pending_deletion.push_back(v3);
                        } else if (pstat_itr->pending_stats[v1].amount > pstat_itr->pending_stats[v3].amount) {
                            verifiedAmount = pstat_itr->pending_stats[v3].amount;
                            _pstats_storage.modify(pstat_itr, get_self(), [&](auto &s) {
                                s.pending_stats[v1].amount -= pstat_itr->pending_stats[v3].amount;
                                s.pending_stats[v3].amount = 0;
                            });
                            pending_deletion.push_back(v3);
                        } else if (pstat_itr->pending_stats[v1].amount < pstat_itr->pending_stats[v3].amount) {
                            verifiedAmount = pstat_itr->pending_stats[v1].amount;
                            _pstats_storage.modify(pstat_itr, get_self(), [&](auto &s) {
                                s.pending_stats[v3].amount -= pstat_itr->pending_stats[v1].amount;
                                s.pending_stats[v1].amount = 0;
                            });
                            pending_deletion.push_back(v1);
                        }
                        if (verifiedAmount > 0) {
                            _storage.modify(siterator, get_self(), [&](auto &storage_stat) {
                                storage_stat.bandwidth_used += verifiedAmount;
                            });
                        }
                    }
                }
            }
        }
    }

    ACTION hodlong::adduser(const name account, string &pub_key) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator == _users.end(), "A user exist for this account.");

        _users.emplace(get_self(), [&](auto &u) {
            u.account = account;
            u.pub_key = pub_key;
            u.balance = asset(0,symbol(symbol_code("SYS"),4));

        });

    }
    ACTION hodlong::updateuser(const name account, string &pub_key){
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator != _users.end(), "User account does not exist");

        _users.modify(iterator, get_self(), [&](auto &u) {
            u.pub_key = pub_key;
        });
    }


    ACTION hodlong::transfer(const name from,const  name to, asset quantity, string memo) {
        // use explicit naming due to code & receiver originating from eosio.token::transfer
        name hname = name("hodlong");
        if (from == hname || to != hname)
            return;
        require_auth(from);
        users transfer_users(hname, hname.value);
        auto iterator = transfer_users.find(from.value);
        eosio_assert(iterator != transfer_users.end(), "User account does not exist");

        transfer_users.modify(iterator, name("hodlong"), [&](auto &u) {
            u.balance += quantity;
        });

    }

    ACTION hodlong::removefunds(name to, asset quantity, string memo) {
        auto user = _users.find(to.value);
        eosio_assert(user != _users.end(), "User does not exist");
        eosio_assert(user->balance <= quantity, "You do not have the required balance to remove the funds");
        action(permission_level{get_self(), "active"_n},
               "eosio.token"_n, "transfer"_n,
               std::make_tuple(get_self(), to, quantity, std::string("Transfer of funds out of hodlong account"))
        ).send();

    }

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    if (action == "transfer"_n.value && code == "eosio.token"_n.value) {
        eosio::execute_action(name(receiver), name(code), &bpfish::hodlong::transfer);
    }
    else if (code == receiver) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(bpfish::hodlong, (buy)(createobj)(addstats)(adduser)(updateuser)(transfer));
        }
    }
    eosio_exit(0);
}
}