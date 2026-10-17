# N05 V019 Handoff

V019 source commit is `1521893105b92c02a47e07cea99fdafc3340a066`, based on V018 source commit `6921ccbf87d45fe8e6ebea4faaf1b403405dc53d`. The input and output batch-base cursors now share one Scalar value.

The valid paired experiment used Parent V018 SHA256 `9cfa3d8367be47e467a3afde1f8aebb9dda6c28341d452e6f63717b451246703` and Candidate V019 SHA256 `57a8b98de7bceae8785e126f62eacc69dc23db91d4b30a56c38631fbef55ab4c`; local and server copies match. CANN 8.5.0.alpha002 compile passed, Correctness passed all four cases on d2, and one V018/V019 paired Local completed with 21 pairs per case.

The paired-delta medians are +0.960, 0.000, -0.240, and +0.260 us. Every delta range spans zero, with a large FP16 D=32768 Parent outlier and shared d2 load. `LOCAL_SCORE=NONE`; raw samples are retained without filtering. Files tagged `v017-parent` are preserved diagnostic outputs from an initial wrong-parent binding and are not V019 scores.

Continue Scalar address-generation research with one independent address factor per revision. Keep Vector emission, tile traversal, arithmetic, DMA/event order, and Core ownership unchanged. Use the existing server3 paired CMake target and d2 runner.
