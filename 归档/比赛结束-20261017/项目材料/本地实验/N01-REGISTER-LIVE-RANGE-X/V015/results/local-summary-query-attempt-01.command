REVISION=V015
PURPOSE=POST_RUN_SUMMARY_ONLY
COMMAND=rg "^(PAIR_BLOCK|SUMMARY .*metric=(device_us|paired_device_delta_us)|RUNNER_COMPLETE|RUNNER_EXIT_CODE)" results/local-v015.raw.log; rg -c "^SAMPLE " results/local-v015.raw.log; rg -c "^CASE_END " results/local-v015.raw.log
HOST=hwnput3
EXIT_CODE=127
ERROR=bash: line 1: rg: command not found
IMPACT=No effect on completed paired Runner output; summaries were computed from the retained local raw file.
