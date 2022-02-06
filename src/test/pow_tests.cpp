// Copyright (c) 2015-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1261130161; // Block #30240
    CBlockIndex pindexLast;
    pindexLast.nHeight = 32255;
    pindexLast.nTime = 1262152739;  // Block #32255
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00d86aU);
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1231006505; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1233061996;  // Block #2015
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00ffffU);
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1279008237; // Block #66528
    CBlockIndex pindexLast;
    pindexLast.nHeight = 68543;
    pindexLast.nTime = 1279297671;  // Block #68543
    pindexLast.nBits = 0x1c05a3f4;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1c0168fdU);
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1263163443; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 46367;
    pindexLast.nTime = 1269211443;  // Block #46367
    pindexLast.nBits = 0x1c387f6f;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00e1fdU);
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_negative_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    nBits = UintToArith256(consensus.powLimit).GetCompact(true);
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_overflow_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits = ~0x00800000;
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_too_easy_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 nBits_arith = UintToArith256(consensus.powLimit);
    nBits_arith *= 2;
    nBits = nBits_arith.GetCompact();
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_biger_hash_than_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith = UintToArith256(consensus.powLimit);
    nBits = hash_arith.GetCompact();
    hash_arith *= 2; // hash > nBits
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_zero_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith{0};
    nBits = hash_arith.GetCompact();
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1269211443 + i * chainParams->GetConsensus().nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

// ASERT DAA related tests
// Tests of the CalculateASERT function.

std::string StrPrintCalcArgs(const arith_uint256 refTarget,
                             const int64_t targetSpacing,
                             const int64_t timeDiff,
                             const int64_t heightDiff,
                             const arith_uint256 expectedTarget,
                             const uint32_t expectednBits) {
    return strprintf("\n"
                     "ref=         %s\n"
                     "spacing=     %d\n"
                     "timeDiff=    %d\n"
                     "heightDiff=  %d\n"
                     "expTarget=   %s\n"
                     "exp nBits=   0x%08x\n",
                     refTarget.ToString(),
                     targetSpacing,
                     timeDiff,
                     heightDiff,
                     expectedTarget.ToString(),
                     expectednBits);
}

BOOST_AUTO_TEST_CASE(calculate_asert_test) {
  const Consensus::Params params = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
  const int64_t nHalfLife = params.nASERTHalfLife;

  const arith_uint256 powLimit = UintToArith256(params.powLimit);
  arith_uint256 initialTarget = powLimit >> 4; // anchor block target
  int64_t height = 0; // height will always be preincrementing

  // The CalculateASERT function uses the absolute ASERT formulation
  // and adds +1 to the height difference that it receives.
  // The time difference passed to it must factor in the difference
  // to the *parent* of the reference block.
  // We assume the parent is ideally spaced in time before the reference block.
  static const int64_t parent_time_diff = params.nPowTargetSpacing; // diff of nTime of anchor and parent of anchor

  // Steady
  arith_uint256 nextTarget = CalculateASERT(initialTarget, params.nPowTargetSpacing, parent_time_diff + 600 /* nTimeDiff - diff between current tip and anchor's parent */, ++height, powLimit, nHalfLife);
  BOOST_CHECK(nextTarget == initialTarget);

  // A block that arrives in half the expected time
  nextTarget = CalculateASERT(initialTarget, params.nPowTargetSpacing, parent_time_diff + 600 + 300, ++height, powLimit, nHalfLife);
  BOOST_CHECK(nextTarget < initialTarget); // aka higher difficulty

  // A block that makes up for the shortfall of the previous one, restores the target to initial
  arith_uint256 prevTarget = nextTarget;
  nextTarget = CalculateASERT(initialTarget, params.nPowTargetSpacing, parent_time_diff + 600 + 300 + 900, ++height, powLimit, nHalfLife);
  BOOST_CHECK(nextTarget > prevTarget);
  BOOST_CHECK(nextTarget == initialTarget);

  // Two days ahead of schedule should double the target (halve the difficulty) (for 288s block the ideal would be 288*600)
  prevTarget = nextTarget;
  nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing, parent_time_diff + 288*1200, 288, powLimit, nHalfLife);
  BOOST_CHECK(nextTarget == prevTarget * 2);

  // Two days behind of schedule should halve the target (double the difficulty) (for 288s block the ideal would be 288*600)
  prevTarget = nextTarget;
  nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing, parent_time_diff + 288*0, 288, powLimit, nHalfLife);
  BOOST_CHECK(nextTarget == prevTarget / 2);
  BOOST_CHECK(nextTarget == initialTarget);

  // Ramp up from initialTarget to PowLimit - should only take 4 doublings...
  uint32_t powLimit_nBits = powLimit.GetCompact();
  uint32_t next_nBits;
  for (size_t k = 0; k < 3; k++) {
      prevTarget = nextTarget;
      nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing, parent_time_diff + 288*1200, 288, powLimit, nHalfLife);
      BOOST_CHECK(nextTarget == prevTarget * 2);
      BOOST_CHECK(nextTarget < powLimit);
      next_nBits = nextTarget.GetCompact();
      BOOST_CHECK(next_nBits != powLimit_nBits);
  }

  prevTarget = nextTarget;
  nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing, parent_time_diff + 288*1200, 288, powLimit, nHalfLife);
  next_nBits = nextTarget.GetCompact();
  BOOST_CHECK(nextTarget == prevTarget * 2);
  BOOST_CHECK(next_nBits == powLimit_nBits);

  // Fast periods now cannot increase target beyond POW limit, even if we try to overflow nextTarget.
  // prevTarget is a uint256, so 256*2 = 512 days would overflow nextTarget unless CalculateASERT
  // correctly detects this error
  nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing, parent_time_diff + 512*144*600, 0, powLimit, nHalfLife);
  next_nBits = nextTarget.GetCompact();
  BOOST_CHECK(next_nBits == powLimit_nBits);

  // We also need to watch for underflows on nextTarget. We need to withstand an extra ~446 days worth of blocks.
  // This should bring down a powLimit target to the a minimum target of 1.
  nextTarget = CalculateASERT(powLimit, params.nPowTargetSpacing, 0, 2*(256-33)*144, powLimit, nHalfLife);
  next_nBits = nextTarget.GetCompact();
  BOOST_CHECK_EQUAL(next_nBits, arith_uint256(1).GetCompact());

  // Define a structure holding parameters to pass to CalculateASERT.
  // We are going to check some expected results  against a vector of
  // possible arguments.
  struct calc_params {
      arith_uint256 refTarget;
      int64_t targetSpacing;
      int64_t timeDiff;
      int64_t heightDiff;
      arith_uint256 expectedTarget;
      uint32_t expectednBits;
  };

  // Define some named input argument values
  const arith_uint256 SINGLE_300_TARGET { "0000000000ffb1004e0000000000000000000000000000000000000000000000" };
  const arith_uint256 FUNNY_REF_TARGET { "000000000080000000000000000fffffffffffffffffffffffffffffffffffff" };

  // Define our expected input and output values.
  // The timeDiff entries exclude the `parent_time_diff` - this is
  // added in the call to CalculateASERT in the test loop.
  const std::vector<calc_params> calculate_args = {

      /* refTarget, targetSpacing, timeDiff, heightDiff, expectedTarget, expectednBits */

      { powLimit, 600, 0, 2*144, powLimit >> 1, 0x1b7fff80 }, // block comes too early (exactly nHalfLife) the target should be dropped twice
      { powLimit, 600, 0, 4*144, powLimit >> 2, 0x1b3fffc0 },
      { powLimit >> 1, 600, 0, 2*144, powLimit >> 2, 0x1b3fffc0 },
      { powLimit >> 2, 600, 0, 2*144, powLimit >> 3, 0x1b1fffe0 },
      { powLimit >> 3, 600, 0, 2*144, powLimit >> 4, 0x1b0ffff0 },
      { powLimit, 600, 0, 2*(256-42)*144, 3, 0x01030000 },
      { powLimit, 600, 0, 2*(256-32)*144 + 119, 1, 0x01010000 },
      { powLimit, 600, 0, 2*(256-40)*144 + 120, 1, 0x01010000 },
      { powLimit, 600, 0, 2*(256-39)*144-1, 1, 0x01010000 },
      { powLimit, 600, 0, 2*(256-33)*144, 1, 0x01010000 },  // 1 bit less since we do not need to shift to 0
      { powLimit, 600, 0, 2*(256-32)*144, 1, 0x01010000 },  // more will not decrease below 1
      { 1, 600, 0, 2*(256-32)*144, 1, 0x01010000 },
      { powLimit, 600, 2*(512-32)*144, 0, powLimit, powLimit_nBits },
      { 1, 600, (512-64)*144*600, 0, powLimit, powLimit_nBits },
      { powLimit, 600, 300, 1, SINGLE_300_TARGET, 0x1c00ffb1 },  // clamps to powLimit
      { FUNNY_REF_TARGET, 600, 600*2*33*144, 0, powLimit, powLimit_nBits }, // confuses any attempt to detect overflow by inspecting result
      { 1, 600, 600*2*256*144, 0, powLimit, powLimit_nBits }, // overflow to exactly 2^256
      { 1, 600, 600*2*224*144 - 1, 0, arith_uint256(0xffff) << 200, powLimit_nBits }, // just under powlimit (not clamped) yet over powlimit_nbits
  };

  for (auto& v : calculate_args) {
      nextTarget = CalculateASERT(v.refTarget, v.targetSpacing, parent_time_diff + v.timeDiff, v.heightDiff, powLimit, nHalfLife);
      next_nBits = nextTarget.GetCompact();
      const auto failMsg =
          StrPrintCalcArgs(v.refTarget, v.targetSpacing, parent_time_diff + v.timeDiff, v.heightDiff, v.expectedTarget, v.expectednBits)
          + strprintf("nextTarget=  %s\nnext nBits=  0x%08x\n", nextTarget.ToString(), next_nBits);
      BOOST_CHECK_MESSAGE(nextTarget == v.expectedTarget && next_nBits == v.expectednBits, failMsg);
  }

}

BOOST_AUTO_TEST_SUITE_END()
