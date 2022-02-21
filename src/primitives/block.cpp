// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Copyright (c) 2022 barrystyle
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/sha1.h>
#include <logging.h>
#include <primitives/block.h>
#include <powcache.h>
#include <hash.h>
#include <tinyformat.h>

uint256 CBlockHeader::GetHash() const
{
    return GetPoWHash();
}

uint256 CBlockHeader::GetLightHash() const
{
    uint256 hash;
    CSHA1().Write((const unsigned char*)this, 80).Finalize((unsigned char*)&hash);
    return hash;
}

uint256 CBlockHeader::GetPoWHash() const
{
    //! light sha1 hash for lookup
    const uint256 lookupHash = GetLightHash();
    bool cacheEntry = pow_cache.HaveCacheEntry(lookupHash);
    if (cacheEntry) {
        uint256 powHash;
        pow_cache.GetCacheEntry(lookupHash, powHash);
        LogPrint(BCLog::POWCACHE, "%s - cachehit %6d cachemiss %6d (%s)\n", __func__, pow_cache.hit(true), pow_cache.miss(), powHash.ToString());
        return powHash;
    }

    //! store for later usage
    uint256 powHash = GetHeavyHash();
    pow_cache.WriteCacheEntry(lookupHash, powHash);
    LogPrint(BCLog::POWCACHE, "%s - cachehit %6d cachemiss %6d (%s)\n", __func__, pow_cache.hit(), pow_cache.miss(true), powHash.ToString());
    return powHash;
}

uint256 CBlockHeader::GetHeavyHash() const
{
    uint256 seed;
    CSHA3_256().Write(hashPrevBlock.begin(), 32).Finalize(seed.begin());
    uint64_t matrix[64*64];
    GenerateHeavyHashMatrix(seed, matrix);
    return SerializeHeavyHash(*this, matrix);
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
