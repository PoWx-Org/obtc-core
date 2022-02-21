// Copyright (c) 2022 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <powcache.h>

PowCacheDB pow_cache(GetDataDir(false) / "powcache", 4194304, false, false);

PowCacheDB::PowCacheDB(fs::path ldb_path, size_t nCacheSize, bool fMemory, bool fWipe) :
    db(ldb_path, nCacheSize, fMemory, fWipe, true)
{}

bool PowCacheDB::HaveCacheEntry(const uint256& lookupHash) const {
    return db.Exists(lookupHash);
}

bool PowCacheDB::GetCacheEntry(const uint256& lookupHash, uint256& powHash) const {
    return db.Read(lookupHash, powHash);
}

bool PowCacheDB::WriteCacheEntry(const uint256& lookupHash, uint256& powHash) {
    return db.Write(lookupHash, powHash);
}

