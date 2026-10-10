# Temporary V002 support binding diff

Scope: `本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support`; temporary binding only, restored after V002 Local. V002 `Parent.asc`, `Candidate.asc`, `runner_abi.h`, CMake timing/build contract, and runner control flow were not changed.

## Parent adapter

Before: `#include "../Parent.asc"`
During V002 build/Local: `#include "../../V002/Parent.asc"`

Before SHA256: `2013d0e9e188501fe953dcf7bbd55e0f9b800e71b9e54ce29d6ce3152eaddc28`
During SHA256: `c43679ad324c1f847c4d9a119906c8c1f4ae73bf3c785d328c2300ef6c275a86`

## Candidate adapter

Before: `#include "../Candidate.asc"`
During V002 build/Local: `#include "../../V002/Candidate.asc"`

Before SHA256: `a7dd3c814fc752cf94bc9d98b918983acc8e73fc66240d6b6599d6057e1e8932`
During SHA256: `dc64ba3d0cb280d964932fc3408dc3c442e171302562485a4c84f378e7523147`

## Runner identity strings

Exactly four output/usage strings changed Candidate identity from `W5-R01-CROSSROW-EPI-PROLOGUE-X-V001` to `W5-R01-CROSSROW-EPI-PROLOGUE-X-V002` in `runner_main.cpp`: usage text, correctness output, TSV header, and Local summary. Parent identity remained `W3-CROSSROW-FULL-PIPELINE-V012`.

Before `runner_main.cpp` SHA256: `e4c46069d21f16b863775461298b452b063430d7ca14fdad3c56ae8c0826d938`
During SHA256: `0f18b4f885b4fd3b938632be8a7bea60a4f6554d113c1762ddef4d6a1e5bcee0`

ABI SHA256 unchanged: `0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2`.

After the Local, original V001 adapter and runner sources were restored byte-for-byte and rebuilt; the restore build log records their restored hashes and RC=0.
