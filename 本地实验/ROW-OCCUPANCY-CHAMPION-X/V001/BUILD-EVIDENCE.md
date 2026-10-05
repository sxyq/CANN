# ROW-OCC-H1 V001 Build Evidence

BUILD_STATUS=PASS
PARENT_PROBE=PASS
CANDIDATE_PROBE=PASS
FULL_LINK=PASS
SOURCE_SHA=7757274fe51507a2f8bf599c5771a9f1e9b21dce68c66a0b2b97934d7f4d1315
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
SOURCE_COMMIT=4c135963
HARNESS_SHA=2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7
NPU_ARCH=dav-2201
SOC=Ascend910B3

The frozen canonical support harness was copied without modification. The
build compiled and linked parent, candidate, and full-link targets with one
host build job. The compiler was the toolkit `bisheng` compiler. Existing
compiler warnings are retained in `build.log`; there were no compilation or
link errors.

Executable identities are recorded in `build.json`. The source and parent
SHA-256 values were checked before build. NPU device 4 was observed at 90%
HBM usage with a conservative free-HBM lower bound above the 100 MB hard
block; no process was killed, paused, or migrated.
