
namespace Hodlong {
    void Hodlong::add(const account_name account, string &pub_key) {
        require_auth(account);
        userIndex users(_self, _self);

        auto iterator = users.find(account);
        eosio_assert(iterator == users.end(), "Address for account already exists");

        users.emplace(account, [&](auto &user) {
            user.account_name = account;
            user.pub_key = pub_key;

        });

    }

    void Hodlong::createobj(account_name account, uint64_t storageId) {
        require_auth(account);
        userIndex users(_self, _self);

        auto iterator = users.find(account);
        eosio_assert(iterator != users.end(), "Address for account not found");

        users.modify(iterator, account, [&](auto& user) {
            user.ownedObjects.push_back(storageId);
        });
    }

    void Hodlong::addseed(account_name account, uint64_t storageId) {
        require_auth(account);
        userIndex users(_self, _self);

        auto iterator = users.find(account);
        eosio_assert(iterator != users.end(), "Address for account not found");

        users.modify(iterator, account, [&](auto& user) {
            user.seededObjects.push_back(storageId);
        });
    }
    void Hodlong::removeseed(const account_name account, uint64_t storageId) {
        require_auth(account);
        userIndex users(_self, _self);

        auto iterator = users.begin();
        eosio_assert(iterator != users.end(), "Address for account not found");

        users.modify(iterator, account, [&](auto& user) {

            auto position = find(user.seededObjects.begin(), user.seededObjects.end(), 8);
            if (position != user.seededObjects.end()) // == myVector.end() means the element was not found
                user.seededObjects.erase(position);
        });
    } 
};