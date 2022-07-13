// Copyright (c) 2022 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POWCACHE_H
#define BITCOIN_POWCACHE_H

#include <dbwrapper.h>
#include <optional.h>
#include <primitives/block.h>
#include <uint256.h>

/** Implements a persistent hash lookup cache, using SHA1 hash as the key.
 * If hash was previosuly encountered and stored it allows for instant retrieval of the more cpu-expensive pow HeavyHash.
 * The hash is not tied to a height, preventing invalid hashes from potentially
 * being returned in the instance of a block reorganisation etc.
 */

//! Max memory allocated for HeavyHash cache (4 MiB ~ 4.2 MB)
static const int64_t nMaxHeavyHashCache = 4;

class CPowCacheDB
{
protected:
    CDBWrapper db;

public:
    explicit CPowCacheDB(fs::path ldb_path, size_t nCacheSize, bool fMemory, bool fWipe);
    
    // Read method already checks for record existence
    Optional<uint256> GetCacheEntry(const uint256& lookupHash) const;
    bool WriteCacheEntry(const uint256& lookupHash, const uint256& powHash);
};

class CPowHashProxy
{
protected:
    std::unique_ptr<CPowCacheDB> pow_cachedb;

public:
    void Init(size_t nCacheSize);
    void Stop();
    uint256 GetHash(CBlockHeader& block);
};

// Global wrapper around CPoWCacheDB object. It is responsible for its existence, and based on this it decides how to retrieve block's hash
extern CPowHashProxy powHashProxy;

#endif // BITCOIN_POWCACHE_H
