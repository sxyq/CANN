# V004 Candidate diff: provenance and repair receipt

ROUTE: `CROSSROW-EPI-PROLOGUE-X`  
REVISION CONTEXT: V008 selector/build setup  
STATUS: OWN-AGENT-ERROR_CONFIRMED; V004 RESTORED TO HEAD

## Cause

The existing canonical staging path `/tmp/w5-r01-canonical.Mk9W1S/V003/submission.asc` is a symlink to this Route's `本地实验/CROSSROW-EPI-PROLOGUE-X/V005/../V004/Candidate.asc`. The V008 build setup copied `V008/Candidate.asc` to that selector using ordinary `cp`, which followed the symlink and overwrote the V004 worktree file. Its temporary content became byte-identical to V008 Candidate SHA `7c6f620ce231e9ebad71b7c8a37bcc8e2ae45ce56086e6f978f02cd1ca9cce06`; the V004 diff therefore both removed V004's rolling index state and removed the `invRmsValues` zero initializer. This was caused by this Agent's V008 selector binding, not an unrelated process.

The later selector restoration copied the saved original submission bytes through the same symlink, restoring V004/Candidate.asc to the committed V004 file. Current worktree SHA is `642b2ae9482ea14e6cde8ed04b457cffc0f1ae2c2d52b242128e85fe36cfd994`, matching `git show HEAD:本地实验/CROSSROW-EPI-PROLOGUE-X/V004/Candidate.asc`; V004 has no current diff. No reset, checkout, history rewrite, or deletion was used. The original bytes are still represented by the saved Parent/Candidate sources and the reconstructed patch below.

## Preserved diff

At the time of the original diff, V004's working source had SHA256 `7c6f620ce231e9ebad71b7c8a37bcc8e2ae45ce56086e6f978f02cd1ca9cce06`; the committed V004 source SHA256 was `642b2ae9482ea14e6cde8ed04b457cffc0f1ae2c2d52b242128e85fe36cfd994` (Git blob `7c9da0bc7599d48e156554dc0b9d53a2ee8f54c4`). Its replacement content is exactly V008 Candidate (Git blob `a7c7ad7cf3e7e6c688367fc665c81c3c79047e6e`). The complete four-hunk unified diff was reconstructed from those exact byte identities and is preserved at:

`canonical-paired-20261011/OUT_OF_SCOPE_V004_CANDIDATE_DIFF_RECONSTRUCTED.patch`

SHA256: `cf3dfc7822e7360370713cc2e1c35f841bc41fb270a778d02d44ae130c7e22bd` (54 lines, 4 hunks). The initial post-restore capture `OUT_OF_SCOPE_V004_CANDIDATE_DIFF.patch` is retained as an empty file because the worktree had already been restored when it was captured; it is not substituted for the reconstructed patch.

The patch is forensic evidence only, not a V008 Candidate change. It will not be staged as `V004/Candidate.asc`, and V004's source history is unchanged.
