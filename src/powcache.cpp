// Copyright (c) 2022 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <powcache.h>

CPowCacheDB::CPowCacheDB(fs::path ldb_path, size_t nCacheSize, bool fMemory, bool fWipe) :
    db(ldb_path, nCacheSize, fMemory, fWipe, true)
{}

Optional<uint256> CPowCacheDB::GetCacheEntry(const uint256& lookupHash) const {
    uint256 blockHash;
    if (db.Read(lookupHash, blockHash)) return blockHash;

    return nullopt;
}

bool CPowCacheDB::WriteCacheEntry(const uint256& lookupHash, const uint256& powHash) {
    return db.Write(lookupHash, powHash);
}

void CPowHashProxy::Init(size_t nCacheSize)
{
    pow_cachedb.reset();
    pow_cachedb.reset(new CPowCacheDB(GetDataDir(false) / "powcache", nCacheSize, false, false));
};

void CPowHashProxy::Stop()
{
    pow_cachedb.reset();
};

uint256 CPowHashProxy::GetHash(CBlockHeader& block)
{
    if(pow_cachedb)
    {
        uint256 cacheHash = block.GetCacheHash();
        
        if (Optional<uint256> optionalBlockHash = pow_cachedb->GetCacheEntry(cacheHash))
            return optionalBlockHash.get();
        
        uint256 blockHash = block.GetHash();
        pow_cachedb->WriteCacheEntry(cacheHash, blockHash);
        return blockHash;
    }

    return block.GetHash();
};

CPowHashProxy powHashProxy;
