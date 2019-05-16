cleos wallet unlock --password PW5JttK2uCvFvQ9AUn2Mc1kPK6NXFLXdG1te29cw4yVR4vJ4WW5De
cleos wallet unlock -n eosio.token --password PW5HyTs5pk7PVF544ZhaG1qFmoZSfYTndxNFCD94v2nNtANdDpumq
cleos wallet unlock -n hodlong --password PW5KdQaXP1kTyZ9adenTJUKFWHvVKfAD9ySrKAB9Vuu6Qowvr49LY
cleos wallet unlock -n trackers --password PW5Jva9P69S3YBdJvbqtaq8KxQ1ZpBHCkDNrUZ2Yfv7xd3LJCZdnL
cleos wallet unlock -n trackerusera --password PW5JuQUzGkPEvkJ87rqzYR6zS8K6Q6ZJVtrALvNKuXM3U9QAXpXPx
cleos wallet unlock -n usera --password PW5KBiAYZXqUpSSaaeMteWFjpPvc6dBCyCbu5q3yttvFN9KatAabi
cleos wallet unlock -n userb --password PW5KbKTQnqW556pBdRzUV28th3e4Lcjt3LUex2xCWvRvMWvhRcB8u
cleos wallet unlock -n userc --password PW5KVosYHrj6vCxZhEiGT5ij2mnM4kczPp9jamaDmozoGHqxWqWet

cleos $1 $2 create account eosio eosio.token EOS6EfdCe1FxjE2TeH9A8HT4CvpYpF2aMLdmoau5zrXAiXgBKT6Bu EOS5z1ZcLWi5cU12Y7xNjQDNDN9uAi2MxZy4Y3pgTVQocztirsQ2o
cleos $1 $2 create account eosio hodlong EOS8bHQcv8QXLWij77rfGrwYevjwteg7uLh3ZRHWDeiu1sHJCafjy EOS7rimLCTfR3JHtCYgxvwRLXEfzNG8YEZWpqAA5rkzjh7Pc1Y438
cleos $1 $2 create account eosio trackers EOS6J5Lbh1T3EqmZob1uFHdVECmh4Q2HpDPfADX9brgsihyeU2T3X EOS5M2VpUXhgxKE1u8t6XC6HEpGwktEPgg8BGRXxx3CSpS3SK8sqf
cleos $1 $2 create account eosio trackerusera EOS8M7vRfSuNsZ3TeGzGSZUVzUf8hcEckBegStc5TTx944sufzz8f EOS58WmMZM2vmJMigb1n9bF9vp3Vg3MhvqUhNJakCCx4JZGBns9oe
cleos $1 $2 create account eosio usera EOS5egjWt5ARm8SMtEkn7fCUTn9qAjv7Gpp9dbfpeTDS4rBzaAZrJ EOS5skNoL8Vo61W5XKCGPAHjnhUFpvrhRw2VhbJhSz2czpF8tU6V4
cleos $1 $2 create account eosio userb EOS6Uximjboj8xu5tv44ciuhhAkWDLszPkpKuwg9Erg3wPzFp1w6r EOS7bon2Hy5wCAQDQvE3VPR9RLJYRboYZ87RJe4rBEaMgsj65kJGS
cleos $1 $2 create account eosio providera EOS8GQzh7kUkRWGxTVJT8RpbdQrBz1tL1GG3eMr5tHxDwsMDt1YDG EOS6RMWqcn5yfjThyCKDqY3ne9dVuF7G63ULr4cSVKHKN2JP9Ec6N

cleos $1 $2 set contract eosio /_work/eos/build/contracts/eosio.bios -p eosio@active
cleos $1 $2 set contract eosio.token /_work/eos/build/contracts/eosio.token  -p eosio.token
cleos $1 $2 set contract hodlong ./ hodlong.wasm hodlong.abi -p hodlong@active
cleos $1 $2 set contract trackers ./ trackers.wasm trackers.abi -p trackers@active

cleos $1 $2 push action eosio.token create '["eosio", "1000000000.0000 SYS"]'  -p eosio.token@active
cleos $1 $2 push action eosio.token issue '["usera", "1000000.0000 SYS", "memo"]' -p eosio@active

cleos $1 $2 push action trackers add '["trackerusera", "dev.hodlong.com:8080"]' -p trackers@active

cleos $1 $2 push action hodlong adduser '["trackerusera", "hmFGT1Zc88R6Zovzy5hh9Gpzh5e8DkfR72fQIxWQDoPpMGzb59SYHHx/NiUWqMJO+eimSvmRyZZ3qln+h4ZGmc1TVR1TdSIh0bYPmEANwYlOZPH3BA7aw53wFGGm9VCaz7JQwOK0HRaQ7hTYM8iJvB/IkUXCXv/dZU0nTWr0QZk="]' -p trackerusera@active
cleos $1 $2 push action hodlong adduser '["usera", "RU1TqtDUZ0PqCrYH7BLTQy/xOXlOIozo4/yc1gw70u18BK6ZA7Ql8g+F59/wXFGAojLeOwKns8Wr69bi1vpFcJ/3Mq4r3T4A6QXbm7v5TteDbSud7Y2kS0tG+LF7zLcOO99PsJxB2wtXaa9pfGJHszn8Mvzc1L7sQ6YJqltvXN8="]' -p usera@active
cleos $1 $2 push action hodlong adduser '["userb", "r0cw9AZpsCVPCONH/gRejieB075w3/wjwWGLXi+B+khGU5A1M3C5ivJBw/iFm0eySM5aiAhmYbqNMz0Ufn4OjMiV5bP0oLPRIyWgSP5HORZCT3WG+v9sTh38vKZQv/YEmW67i0Qgxd+Za2ZGG5SeagCWWoUq/spisDnn+rexnAs="]' -p userb@active
cleos $1 $2 push action hodlong adduser '["providera", "fENYCwK6/epX7eImpwMtnTsZH3A5iaJ9jHVVqf53jvef8KBeL6M2GWtItu6CzG9BiEFAZzbggWvTPOan4L7WNf5GIg+3N8yPcYNKVQiFxtcDOA70BRHWpNF6QjKCban8+DaCTlWtm9uFrV/psyh1L8HJhdgHY2BMDQ0/Am8ZVTU="]' -p providera@active

cleos $1 $2 push action eosio.token transfer '["usera", "hodlong", "10000.0000 SYS", "m"]' -p usera@active

cleos $1 $2 push action hodlong createobj '{"account": "usera", "filename": "b0e317d984a81f4ffe2195ea03c534a973b67758", "filesize": "16133022", "approved_seeders":["providera"], "allowed_users": [], "max_seeders": 3, "self_host": 0, "bandwidth_cost": 1, "bandwidth_divisor": 10133020}' -p usera@active
cleos $1 $2 push action hodlong createobj '{"account": "usera", "filename": "d2474e86c95b19b8bcfdb92bc12c9d44667cfa36", "filesize": "1999", "approved_seeders":["providera"], "allowed_users": [], "max_seeders": 3, "self_host": 0, "bandwidth_cost": 1, "bandwidth_divisor": 1}' -p usera@active

//cleos $1 $2 push action hodlong seed '["providera", 0]' -p providera@active