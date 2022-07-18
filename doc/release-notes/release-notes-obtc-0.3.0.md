Version 0.3.0
===============================

Summary
===============================

This version contains an enhancement related to long block indexation and wallet rescaning, which requires chain marching.
A new leveldb database is added, which is stored in `powcache` file.
Due to the fact that computing block's heavy hash is very expensive, it is stored in the `powcache` database.

Key -> Value: SHA1 -> Heavy Hash

Compatibility
===============================

It is fully compatile with the previous versions, and has no forking potential.

Updated and new sources
===============================

- powcache.*
- block.*
- logging.*
- init.cpp
- txdb.cpp
- validation.cpp