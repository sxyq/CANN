# N05 V018 Handoff

V018 source commit is `6921ccbf87d45fe8e6ebea4faaf1b403405dc53d`, based on V017 source commit `97222450df4d33b6920f46f42128306f82907f65`. The pass-one x/residual prefetch pair shares a derived next input offset, which becomes the current offset for the following tile.

Parent and Candidate local/remote SHA256 values match. CMakeCache and the existing Runner confirm distinct input directories and bindings. Compile passed with CANN 8.5.0.alpha002; Correctness passed all four cases on d2. Paired Local completed with 21 pairs per case. The four delta medians are -0.760, -0.180, +0.220, and -0.240 us. Shared d2 load and extreme outliers leave LOCAL_SCORE=NONE; raw data is retained without filtering.

Continue the Scalar address-generation/Row-Tile study with one independent address factor per revision, keeping Vector instruction emission unchanged. Use the existing d2 Compile/Correctness/Local entry.
