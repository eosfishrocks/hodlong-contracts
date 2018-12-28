cleos wallet unlock --password PW5JttK2uCvFvQ9AUn2Mc1kPK6NXFLXdG1te29cw4yVR4vJ4WW5De
cleos wallet unlock -n eosio.token --password PW5HyTs5pk7PVF544ZhaG1qFmoZSfYTndxNFCD94v2nNtANdDpumq
cleos wallet unlock -n hodlong --password PW5KdQaXP1kTyZ9adenTJUKFWHvVKfAD9ySrKAB9Vuu6Qowvr49LY
cleos wallet unlock -n trackers --password PW5Jva9P69S3YBdJvbqtaq8KxQ1ZpBHCkDNrUZ2Yfv7xd3LJCZdnL
cleos wallet unlock -n trackerusera --password PW5JuQUzGkPEvkJ87rqzYR6zS8K6Q6ZJVtrALvNKuXM3U9QAXpXPx
cleos wallet unlock -n usera --password PW5KBiAYZXqUpSSaaeMteWFjpPvc6dBCyCbu5q3yttvFN9KatAabi
cleos wallet unlock -n userb --password PW5KbKTQnqW556pBdRzUV28th3e4Lcjt3LUex2xCWvRvMWvhRcB8u
cleos wallet unlock -n userc --password PW5KVosYHrj6vCxZhEiGT5ij2mnM4kczPp9jamaDmozoGHqxWqWet

cleos create account eosio eosio.token EOS6EfdCe1FxjE2TeH9A8HT4CvpYpF2aMLdmoau5zrXAiXgBKT6Bu EOS5z1ZcLWi5cU12Y7xNjQDNDN9uAi2MxZy4Y3pgTVQocztirsQ2o
cleos create account eosio hodlong EOS8bHQcv8QXLWij77rfGrwYevjwteg7uLh3ZRHWDeiu1sHJCafjy EOS7rimLCTfR3JHtCYgxvwRLXEfzNG8YEZWpqAA5rkzjh7Pc1Y438
cleos create account eosio trackers EOS6J5Lbh1T3EqmZob1uFHdVECmh4Q2HpDPfADX9brgsihyeU2T3X EOS5M2VpUXhgxKE1u8t6XC6HEpGwktEPgg8BGRXxx3CSpS3SK8sqf
cleos create account eosio trackerusera EOS8M7vRfSuNsZ3TeGzGSZUVzUf8hcEckBegStc5TTx944sufzz8f EOS58WmMZM2vmJMigb1n9bF9vp3Vg3MhvqUhNJakCCx4JZGBns9oe
cleos create account eosio usera EOS5egjWt5ARm8SMtEkn7fCUTn9qAjv7Gpp9dbfpeTDS4rBzaAZrJ EOS5skNoL8Vo61W5XKCGPAHjnhUFpvrhRw2VhbJhSz2czpF8tU6V4
cleos create account eosio userb EOS6Uximjboj8xu5tv44ciuhhAkWDLszPkpKuwg9Erg3wPzFp1w6r EOS7bon2Hy5wCAQDQvE3VPR9RLJYRboYZ87RJe4rBEaMgsj65kJGS
cleos create account eosio userc EOS8GQzh7kUkRWGxTVJT8RpbdQrBz1tL1GG3eMr5tHxDwsMDt1YDG EOS6RMWqcn5yfjThyCKDqY3ne9dVuF7G63ULr4cSVKHKN2JP9Ec6N


cleos set contract eosio /_work/eos/build/contracts/eosio.bios -p eosio@active
cleos set contract eosio.token /_work/eos/build/contracts/eosio.token  -p eosio.token
cleos set contract hodlong ./ hodlong.wasm hodlong.abi -p hodlong@active
cleos set contract trackers ./ trackers.wasm trackers.abi -p trackers@active

cleos push action eosio.token create '["eosio", "1000000000.0000 SYS"]'  -p eosio.token@active
cleos push action eosio.token issue '["usera", "1000000.0000 SYS", "memo"]' -p eosio@active

cleos push action trackers add '["trackerusera", "http://127.0.0.1:8080"]' -p trackerusera@active

cleos push action hodlong adduser '["trackerusera", "Py4saCco9DyYg7YFN8sJjOfmR5i6P+602JMW5ppmHGOywIk1yMixdfGXCyZYcCmNn3K9XXwmw7eHT410LfS/47wsBnmb7opEXp06b5VN2F0gU8bp7n2zYGlB7u20ByJOSdC3RoPaorOSL1hXRxKyqneFdiboMDlFkslrRAfv58"]' -p trackerusera@active
cleos push action hodlong adduser '["usera", "Ty4saCco9DyYg7YFN8sJjOfmR5i6P+602JMW5ppmHGOywIk1yMixdfGXCyZYcCmNn3K9XXwmw7eHT410LfS/47wsBnmb7opEXp06b5VN2F0gU8bp7n2zYGlB7u20ByJOSdC3RoPaorOSL1hXRxKyqneFdiboMDlFkslrRAfv58"]' -p usera@active
cleos push action hodlong adduser '["userb", "Ay4saCco9DyYg7YFN8sJjOfmR5i6P+602JMW5ppmHGOywIk1yMixdfGXCyZYcCmNn3K9XXwmw7eHT410LfS/47wsBnmb7opEXp06b5VN2F0gU8bp7n2zYGlB7u20ByJOSdC3RoPaorOSL1hXRxKyqneFdiboMDlFkslrRAfv58"]' -p userb@active
cleos push action hodlong adduser '["userc", "Ny4saCco9DyYg7YFN8sJjOfmR5i6P+602JMW5ppmHGOywIk1yMixdfGXCyZYcCmNn3K9XXwmw7eHT410LfS/47wsBnmb7opEXp06b5VN2F0gU8bp7n2zYGlB7u20ByJOSdC3RoPaorOSL1hXRxKyqneFdiboMDlFkslrRAfv58"]' -p userc@active

cleos push action eosio.token transfer '["usera", "hodlong", "10000.0000 SYS", "m"]' -p usera@active

cleos push action hodlong createobj '["usera", "test", "1000", [], [], 3, 0, 1, 10000 ]' -p usera@active

cleos push action hodlong seed '["userb", 0]' -p userb@active
cleos push action hodlong seed '["trackerusera", 0]' -p trackerusera@active

cleos push action hodlong addstats '["userb", "userb", "trackerusera", 0 ,1000000]' -p userb@active
cleos push action hodlong addstats '["trackerusera", "userb", "trackerusera", 0 ,1000000]' -p trackerusera@active
