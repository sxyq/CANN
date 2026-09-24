# JUDGE HANDOFF — UB-LIVENESS-X V003

## Package verification
- submission.asc SHA256: 2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd
- submission.sha256: matches
- source-meta.json: DECISION=ONLINE_CANDIDATE, SINGLE_CHANGE_AUDIT=PASS, CORRECTNESS=PASS
- local-result.json: decision=ONLINE_CANDIDATE, correctness PASS
- diff.patch: present (14578 bytes)
- online-candidate-pool.tsv: row present with frozen SHA
- phase4/online/UB-LIVENESS-X/: not present (never submitted)

## Status
JUDGE_READY=YES
NOTICE=READY_FOR_FORMAL_SUBMISSION
route=UB-LIVENESS-X revision=V003
evidence=/Users/sunyiyang/Desktop/Project/cann-next6/UB-LIVENESS-X/phase4/local/UB-LIVENESS-X/V003/
exact_source=submission.asc (SHA above)

## Ownership
MAIN-2 does NOT self-submit.
Unified Judge Owner must run:
  npm run cannjudge:submit -- --yes --source <exact submission.asc path>
Require LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA.

## After result
Record submission_id, remote SHA, correctness, official score, direct parent, decision.
PROMOTE only if Official Score > Direct Parent; else REJECT/INCONCLUSIVE.
