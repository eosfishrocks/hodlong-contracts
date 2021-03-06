#include "Hodlong.hpp"
namespace bpfish{
    ACTION hodlong::addallowed(const name authority, const name sp){
        require_auth(authority);
        auto iterator = _users.find(authority.value);
        eosio_assert(iterator != _users.end(), "User id does not exist");

        for (int i=0; i < iterator->allowed_storage_providers.size(); i++)
        {
            eosio_assert(iterator->allowed_storage_providers[i].value != sp.value, "User already has storage provider");
        }
        _users.modify(iterator, get_self(), [&](auto &u) {
            u.allowed_storage_providers.push_back(sp);
        });

    }
    ACTION hodlong::addas(const name authority, uint64_t storage_id, name provider) {
        auto iterator = _storage.find(storage_id);
        eosio_assert(iterator != _storage.end(), "storage_id does not exist");
        eosio_assert((authority == _self || authority == iterator->account),
                     "User does not have the authority to add to the the approved seeder");
        for (int i=0; i < iterator->approved_seeders.size(); i++)
        {
            eosio_assert(iterator->approved_seeders[i].value != provider.value, "User is already an approved seeder");
        }
        _storage.modify(iterator, get_self(), [&](auto &u) {
            u.approved_seeders.push_back(provider);
        });

    }
    ACTION hodlong::addstats(const name authority, const name from, const name to, uint64_t storage_id, uint64_t amount) {
        require_auth(authority);
        time_t date = now();

        bool found_stat = false;
        name paid_account;
        auto storage_iter = _storage.find(storage_id);
        eosio_assert(storage_iter != _storage.end(), "The storage id is not found");


        for (int s0 = 0; s0 < storage_iter->approved_seeders.size(); s0++){
            if (storage_iter->approved_seeders[s0] == from) paid_account=from;
            if (storage_iter->approved_seeders[s0] == to) paid_account=to;{
                stat client_stat = {authority, from, to, amount, date};
            }
        }
        auto pstat_itr = _pstats.lower_bound(storage_id);

        stat client_stat = {authority, from, to, amount, date};
        if (pstat_itr == _pstats.end() && paid_account) {
            _pstats.emplace(get_self(), [&](auto &tmp_stat) {
                tmp_stat.storageid = storage_id;
                tmp_stat.pending_stats.push_back(client_stat);
            });
        } else if (paid_account){
            _pstats.modify(pstat_itr, get_self(), [&](auto &tmp_stat) {
                tmp_stat.pending_stats.push_back(client_stat);
            });
            found_stat = true;
        }

        //may need to break into deferred actions for delete depending on mainnet processing times
        if (found_stat && paid_account) {
            eosio_assert(from == authority || to == authority, "Neither to or from is the authority");
            vector <uint64_t> pending_deletion;
            // #TODO Add multi index secondary key of authority.value + from.value & to.value to cut processing times
            // #TODO for envs with many users.
            for (int v1 = 0; v1 < pstat_itr->pending_stats.size(); v1++) {
                int v3 = v1 + 1;
                if (now() - pstat_itr->pending_stats[v1].date > 604800) {
                    pending_deletion.push_back(v1);
                    break;
                }

                for (int v2 = 0; v2 < pstat_itr->pending_stats.size(); v2++) {
                    if (v2 == 0) {
                        v2 = v3;
                    }

                    if (pstat_itr->pending_stats[v1].to == pstat_itr->pending_stats[v3].to &&
                        pstat_itr->pending_stats[v1].from == pstat_itr->pending_stats[v3].from &&
                        pstat_itr->pending_stats[v1].to != pstat_itr->pending_stats[v3].from &&
                        pstat_itr->pending_stats[v1].from != pstat_itr->pending_stats[v3].to &&
                        pstat_itr->pending_stats[v1].authority != pstat_itr->pending_stats[v3].authority &&
                        pstat_itr->pending_stats[v1].amount !=0 && pstat_itr->pending_stats[v2].amount != 0) {
                        uint64_t verified_amount;
                        if (pstat_itr->pending_stats[v1].amount == pstat_itr->pending_stats[v3].amount) {
                            verified_amount = pstat_itr->pending_stats[v1].amount;
                            _pstats.modify(pstat_itr, get_self(), [&](auto &s) {
                                s.pending_stats[v1].amount = 0;
                                s.pending_stats[v3].amount = 0;
                            });
                            pending_deletion.push_back(v1);
                            pending_deletion.push_back(v3);
                        } else if (pstat_itr->pending_stats[v1].amount > pstat_itr->pending_stats[v3].amount) {
                            verified_amount = pstat_itr->pending_stats[v3].amount;
                            _pstats.modify(pstat_itr, get_self(), [&](auto &s) {
                                s.pending_stats[v1].amount -= pstat_itr->pending_stats[v3].amount;
                                s.pending_stats[v3].amount = 0;
                            });
                            pending_deletion.push_back(v3);
                        } else if (pstat_itr->pending_stats[v1].amount < pstat_itr->pending_stats[v3].amount) {
                            verified_amount = pstat_itr->pending_stats[v1].amount;
                            _pstats.modify(pstat_itr, get_self(), [&](auto &s) {
                                s.pending_stats[v3].amount -= pstat_itr->pending_stats[v1].amount;
                                s.pending_stats[v1].amount = 0;
                            });
                            pending_deletion.push_back(v1);
                        }
                        if (verified_amount > 0) {
                            _storage.modify(storage_iter, get_self(), [&](auto &storage_stat) {
                                storage_stat.bandwidth_used += verified_amount;
                            });
                            auto stat_iter = _stats.find(storage_id);
                            if (stat_iter == _stats.end()) {
                                _stats.emplace(get_self(), [&](auto &s) {
                                    s.storage_id = _stats.available_primary_key();
                                    s.account = storage_iter->account;
                                    s.amount = verified_amount;
                                    s.negative = true;
                                });
                                _stats.emplace(get_self(), [&](auto &s) {
                                    s.storage_id = _stats.available_primary_key();
                                    s.account = paid_account;
                                    s.amount = verified_amount;
                                    s.negative = false;
                                });
                            } else {
                                for(;stat_iter != _stats.end();stat_iter++) {
                                    if (stat_iter->account == paid_account || stat_iter->account == storage_iter->account){
                                        _stats.modify(stat_iter, get_self(), [&](auto &s) {
                                            s.amount += verified_amount;
                                        });
                                        // Move balance if over divisor
                                        if (stat_iter->amount > storage_iter->bandwidth_divisor){
                                            uint64_t new_balance = stat_iter->amount % storage_iter->bandwidth_divisor;
                                            uint64_t paid_amount = storage_iter->bandwidth_cost * (stat_iter->amount/storage_iter->bandwidth_divisor);
                                            asset paid_amount_s = asset(paid_amount, symbol(symbol_code(SYMBOL_NAME),4));


                                            auto user_iter = _users.find(stat_iter->account.value);
                                            eosio_assert(user_iter != _users.end(), "User account does not exist.");
                                            _users.modify(user_iter, get_self(), [&](auto &u) {
                                                if (stat_iter->negative) {
                                                    if (user_iter->balance < paid_amount_s){
                                                        u.balance = asset(0, symbol(symbol_code(SYMBOL_NAME),4));
                                                    }
                                                    eosio_assert(u.balance > paid_amount_s, "User does not have the balance to pay");
                                                    u.balance -= paid_amount_s;
                                                }
                                                else {
                                                    u.balance += paid_amount_s;
                                                }
                                            });
                                            _stats.modify(stat_iter, get_self(), [&](auto &s) {
                                                s.amount = new_balance;
                                            });

                                        }
                                    }
                                }
                            }

                        }
                    }
                }
                // delete empty stats in pstats
                for (int d = 0;  d < pending_deletion.size(); d++){
                    _pstats.modify(pstat_itr, get_self(), [&](auto &s) {
                        s.pending_stats.erase(s.pending_stats.begin() + pending_deletion[d] - d );
                    });
                    if (pstat_itr->pending_stats.size() == 0){
                        _pstats.erase(pstat_itr);
                    }
                }
            }
        }
    }
    ACTION hodlong::addprovider(name authority, name provider, uint64_t bandwidth_cost, uint64_t bandwidth_divisor, uint64_t storage_cost) {
        require_auth(authority);

        auto iterator = _users.find(provider.value);
        eosio_assert(iterator != _users.end(), "User does not exist.");
        eosio_assert(iterator->provider != true, "Provider is already set for user");

        _providers.emplace(get_self(), [&](auto &p) {
            p.provider_id= provider;
            p.bandwidth_cost = bandwidth_cost;
            p.bandwidth_divisor = bandwidth_divisor;
            p.storage_cost = storage_cost;

        });
        _users.modify(iterator, get_self(), [&](auto &u) {
            u.provider = true;
        });
    }
    ACTION hodlong::adduser(const name account, string &pub_key) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator == _users.end(), "A user exist for this account.");

        _users.emplace(get_self(), [&](auto &u) {
            u.account = account;
            u.pub_key = pub_key;
            u.balance = asset(0,symbol(symbol_code(SYMBOL_NAME),4));
        });

    }
    ACTION hodlong::createobj(name account, string &filename, string &file_size, vector<name> &approved_seeders,
            vector<name> &allowed_users,uint64_t max_seeders, bool self_host, uint64_t bandwidth_cost, uint64_t bandwidth_divisor) {
        require_auth(account);
        auto iterator = _users.find(account.value);
        eosio_assert(iterator != _users.end(), "User does not exist.");

        uint64_t storage_id = _storage.available_primary_key();
        _storage.emplace(get_self(), [&](auto &s) {
            s.storage_id = storage_id;
            s.account = account;
            s.filename = filename;
            s.file_size = file_size;
            s.max_seeders = max_seeders;
            s.self_host = self_host;
            s.bandwidth_used = 0;
            s.approved_seeders = approved_seeders;
            s.bandwidth_cost = bandwidth_cost;
            s.bandwidth_divisor = bandwidth_divisor;
            s.oseeds = max_seeders;
            s.active = true;
        });

        _users.modify(iterator, get_self(), [&](auto &u) {
            u.owned_objects.push_back(storage_id);
        });
    }
    ACTION hodlong::modifysp(const name user, bool provider){
        require_auth(_self);
        auto iterator = _users.find(user.value);
        eosio_assert(iterator != _users.end(), "User account does not exist");

        _users.modify(iterator, get_self(), [&](auto &u) {
            u.provider = provider;
        });

    }
    ACTION hodlong::removeas(const name authority, uint64_t storage_id, name seeder) {
        auto iterator = _storage.find(storage_id);
        eosio_assert(iterator != _storage.end(), "storage_id does not exist");
        eosio_assert((authority == _self || authority == iterator->account || authority == seeder),
                     "User does not have the authority to remove the approved seeder");
        for (int i=0; i < iterator->approved_seeders.size(); i++)
        {
            if (iterator->approved_seeders[i].value == seeder.value){
                _storage.erase(iterator);
            }
        }

    }
    ACTION hodlong::removefunds(name to, asset quantity) {
        auto user = _users.find(to.value);
        eosio_assert(user != _users.end(), "User does not exist");
        eosio_assert(user->balance <= quantity, "You do not have the required balance to remove the funds");
        asset transfer_amount;

        _users.modify(user, name(CONTRACT_NAME), [&](auto &u) {
            u.balance -= quantity;
        });


        transaction deferredTrans{};

    deferredTrans.actions.emplace_back(
            action(permission_level{_self, "removefunds"_n},
                   name(TOKEN_CONTRACT),
                   "transfer"_n,
                   make_tuple(name(CONTRACT_NAME), to, quantity,
                              string("Transfer of funds out of hodlong account")))
    );

    deferredTrans.delay_sec = TRANSFER_DELAY;
        uint128_t sender_id = (uint128_t(to.value) << 64) | now();
        deferredTrans.send(sender_id, _self);
    }
    ACTION hodlong::removeo(const name authority, uint64_t storage_id) {
        require_auth(_self);
        auto iterator = _storage.find(storage_id);
        eosio_assert(iterator != _storage.end(), "storage_id does not exist");

        _storage.erase(iterator);
    }
    ACTION hodlong::seed(name buyer, uint64_t storage_id) {
        auto aiter= _users.find(buyer.value);
        auto iterator = _storage.find(storage_id);
        eosio_assert(aiter!= _users.end(), "User does not exist");
        eosio_assert(iterator != _storage.end(), "The bid not found");
        eosio_assert(iterator->approved_seeders.size() <= iterator->max_seeders,
                     "The storage object has the max amount of seeders");
        eosio_assert(!iterator->self_host, "Object owner controls seed management");
        
        _storage.modify(iterator, get_self(), [&](auto &s) {
            s.approved_seeders.push_back(buyer);
            s.oseeds -=  1;
        });
        _users.modify(aiter, get_self(), [&](auto &u) {
            u.seeded_objects.push_back(storage_id);;
        });
    }
    ACTION hodlong::transfer(const name from,const  name to, asset quantity, string memo) {
        require_auth(from);
        eosio_assert(from != name(CONTRACT_NAME), "From field must not originate from this contract.");
        eosio_assert(to == name(CONTRACT_NAME), "To field must be this contract.");
        eosio_assert(from != name("eosio.stake"), "Tokens cannot be staked");

        // use new multi index variable due to code & receiver originating from eosio.token::transfer
        users transfer_users(name(CONTRACT_NAME), name(CONTRACT_NAME).value);
        auto iterator = transfer_users.find(from.value);
        eosio_assert(iterator != transfer_users.end(), "User account does not exist");

        transfer_users.modify(iterator, name(CONTRACT_NAME), [&](auto &u) {
            u.balance += quantity;
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
}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    if (action == "transfer"_n.value && code == name(TOKEN_CONTRACT).value) {
        eosio::execute_action(name(receiver), name(code), &bpfish::hodlong::transfer);
    }
    else if (code == receiver) {
        switch (action) {
            EOSIO_DISPATCH_HELPER(bpfish::hodlong, (seed)(createobj)(addstats)(adduser)(addprovider)(updateuser));
        }
    }
    eosio_exit(0);
}
}