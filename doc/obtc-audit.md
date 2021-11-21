# oBTC Audit

* Code Location: https://github.com/PoWx-Org/obtc-core
* Last Commit: 29132bc679a334dc2e8b4093acbf26a0823fafba

## Audit Status

* Status: Implemented
* Date: 21/11/2021
* Commit: https://github.com/PoWx-Org/obtc-core/commit/afa521627eb1867d52f6f71120ae0fac3ecb5811

## Scope

The scope of the audit is limited to the integration of new optical PoW algorithms & dependencies within the existing bitcoin codebase. It does not include the bitcoin codebase itself, nor does it verify the security and correctness of the oPoW algorithm itself. Instead, the focus was on making sure the safety & security of the consensus and P2P logic remain unaffected. 

### Dependencies Added

* tiny_sha3 - https://github.com/mjosaarinen/tiny_sha3
* singular - https://github.com/kikuomax/singular/

### Code Changes

### Security Concerns

* No critical security issues were found in the integration of new oPoW code, though some possible improvements were identified.

### Suggested Improvements

#### 1. Re-enable Max Tip Age Check

The max tip age check was disabled in the following commit:
  - https://github.com/PoWx-Org/obtc-core/commit/01e91228e1c87caf63adeb0d89e417409a2327d1

The commit message suggests this was supposed to be temporary, but the check remains disabled.

The impact of this change is widespread, and while not specifically a security threat, this will lead to a number of initial sync optimizations in the node & wallet being skipped after receiving enough blocks to have `nMinimumChainWork` amount of work.

This code is not consensus critical, so our recommendation is to re-enable the check as soon as possible.

#### 2. Remove PoWHash Sanity Check in LoadBlockIndexGuts

LoadBlockIndexGuts, which is called for all historical block headers during startup, performs an unnecessary sanity check on the block's PoW hash.
* https://github.com/PoWx-Org/obtc-core/blob/a26d0cbaf7be2a4984f8289878c2572c369a641f/src/txdb.cpp#L278-L279

Since bitcoin uses SHA256, which can be verified very quickly, the belt-and-suspenders sanity check is harmless. But for hashing algorithms that require more clock cycles to verify, the longer time spent performing the check could lead to performance degradation during startup.

There is precedent for removing it. Litecoin, among others, has been skipping this check without issue for years, so we recommend following in their footsteps:

* https://github.com/litecoin-project/litecoin/blob/02961f00e6e185f17f47c2b1c9133e6cedb57c99/src/txdb.cpp#L277-L284
