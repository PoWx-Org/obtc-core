// Copyright (c) 2022 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POWCACHE_H
#define BITCOIN_POWCACHE_H

#include <dbwrapper.h>
#include <uint256.h>

/** Implements a persistent hash lookup cache, using SHA1 hash as the key
 * allowing instant retrieval of the more cpu-expensive pow hash if known.
 * The hash is not tied to a height, preventing invalid hashes from potentially
 * being returned in the instance of a block reorganisation etc.
 */
class PowCacheDB
{
protected:
    CDBWrapper db;

private:
    int hit_log{0};
    int miss_log{0};

public:
    explicit PowCacheDB(fs::path ldb_path, size_t nCacheSize, bool fMemory, bool fWipe);

    int hit(bool inc = false) {
        if (inc) ++hit_log;
        return hit_log;
    }
    int miss(bool inc = false) {
        if (inc) ++miss_log;
        return miss_log;
    }

    bool HaveCacheEntry(const uint256& lookupHash) const;
    bool GetCacheEntry(const uint256& lookupHash, uint256& powHash) const;
    bool WriteCacheEntry(const uint256& lookupHash, uint256& powHash);
};

extern PowCacheDB pow_cache;

#endif // BITCOIN_POWCACHE_H
