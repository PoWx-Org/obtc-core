// Copyright (c) 2018-2019 The Bitcoin developers
// Copyright (c) 2022 PoWx Team
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_ACTIVATION_H
#define BITCOIN_CONSENSUS_ACTIVATION_H

#include <cstdint>

class CBlockIndex;

namespace Consensus {
struct Params;
}

/** Check if Feb 2022 protocol upgrade to ASERT DAA has been activated. */
bool IsAsertEnabled(const Consensus::Params &params,
                    const CBlockIndex *pindexPrev);

#endif // BITCOIN_CONSENSUS_ACTIVATION_H
