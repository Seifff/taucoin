// Copyright (c) 2018- The isncoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rewardman.h"

#include "util.h"
#include "sync.h"
#include <sstream>
//#include "tool.h"

extern bool ConvertPubkeyToAddress(const std::string& pubKey, std::string& addrStr);

static void int2str(const int64_t &int_temp, std::string &string_temp)  
{  
    stringstream stream;  
    stream << int_temp;  
    string_temp = stream.str(); 
} 

static CCriticalSection cs_rwdman;

RewardManager* RewardManager::pSingleton = NULL;

RewardManager* RewardManager::GetInstance()
{
    LOCK(cs_rwdman);

    if (pSingleton == NULL)
    {
        pSingleton = new RewardManager();
    }

    return pSingleton;
}

CAmount RewardManager::GetRewardsByAddress(std::string& address)
{
	std::vector<string> fields;
	fields.push_back(memFieldBalance);
	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableMember, fields, memFieldAddress, address);

    if (bLocal.num_rows() > 0)
    {
	    return bLocal[0]["balance"];
    }
    else
    {
        return 0;
    }
}

bool RewardManager::UpdateRewardsByAddress(std::string& address, CAmount rewards)
{
	std::vector<string> fields;
	fields.push_back(memFieldBalance);

    std::vector<string> values;
    std::string valueStr;
    int2str(rewards, valueStr);
	values.push_back(valueStr);

    mysqlpp::SimpleResult bLocal = backendDb->ISNSqlUpdate(tableMember, fields, values, memFieldAddress, address);

    if (bLocal.rows() > 0)
    {
	    return true;
    }
    else
    {
        return false;
    }
}

CAmount RewardManager::GetRewardsByPubkey(const std::string &pubkey)
{
    std::string addrStr;

    if (!ConvertPubkeyToAddress(pubkey, addrStr))
        return 0;

    return GetRewardsByAddress(addrStr);
}

bool RewardManager::UpdateRewardsByPubkey(const std::string &pubkey, CAmount rewards)
{
    std::string addrStr;

    if (!ConvertPubkeyToAddress(pubkey, addrStr))
        return 0;

    return UpdateRewardsByAddress(addrStr, rewards);
}

RewardManager::RewardManager()
{
    backendDb = ISNDB::GetInstance();
}

bool RewardManager::GetMembersByClubID(uint64_t clubID, std::vector<std::string>& addresses)
{
    addresses.clear();

	std::vector<string> fields;
	fields.push_back(memFieldAddress);
    fields.push_back(memFieldClub);

    std::string clubIDStr;
    int2str(clubID, clubIDStr);

	mysqlpp::StoreQueryResult bLocal = backendDb->ISNSqlSelectAA(tableMember, fields, memFieldClub, clubIDStr);

    uint64_t size = (uint64_t)bLocal.num_rows();
    for (uint64_t i = 0; i != size; i++)
    {
        if (clubID != (uint64_t)bLocal[i]["club_id"])
        {
            addresses.push_back(static_cast<std::string>(bLocal[i]["address"]));
        }
    }

    if (addresses.size() > 0)
    {
	    return true;
    }
    else
    {
        return false;
    }
}
