# Older CodeQL alerts: beta 3 readiness triage

Review-only receipt, 28 September 2026. Product code/test/build input is
published `develop` `9f2f44cfd888d324858769809b0381e524850b4c`.
The current local checkout adds status documentation only. This review used
GitHub alert and instance APIs, the develop CodeQL SARIF analysis `1853375568`
(SHA-256 `1c519c43b37fc9a8d7c1f5d27b687e5b923754320004fede26c6992fdd59a15c`),
and the source and documented `rxfs` contract at that product revision. No
tests, workflows, production changes, alert dismissals or suppressions were
made. Every disposition below is proposed for coordinator review, not accepted
product policy or closure.

All nine requested alert IDs have an **open `refs/heads/develop` instance** at
`9f2f44cfd`. For #216-224 the alert endpoint's `most_recent_instance` happens
to report `master` at `2d24ae989`; the develop instance has shifted line
numbers. The REST `?state=open` list includes only seven of these IDs: direct
alert reads for #225 and #226 return top-level `state:null`, while their
develop instances explicitly say `open`. Do not infer those two are fixed or
dismissed from the list discrepancy.

| Alert | Current develop path | Review-only classification and smallest next step |
| --- | --- | --- |
| [#216](https://github.com/adesutherland/CREXX/security/code-scanning/216) | `compiler/rxcp_ast_core.c:864` | Harmless true redundancy: `first_token` returned on null at :820 and was dereferenced at :855/861 before this `line` ternary. No reachable null case was found for this check. Remove only the redundant ternary if cleaning the alert; preserve source position semantics. |
| [#217](https://github.com/adesutherland/CREXX/security/code-scanning/217) | `compiler/rxcp_ast_core.c:865` | Same cause for `column`; no distinct defect. |
| [#218](https://github.com/adesutherland/CREXX/security/code-scanning/218) | `compiler/rxcp_ast_core.c:866` | Same cause for `source_start`; no distinct defect. |
| [#220](https://github.com/adesutherland/CREXX/security/code-scanning/220) | `binutils/rxbin007.c:1061` | **Confirmed error-path correctness defect.** `operand_index` is `size_t` (:994), but the second argument is printed with `%d` in `rxbin007_set_error` when semantic-graph operand resolution fails (:1053-1064). The adjacent branches correctly use `%zu`. Replace this one specifier with `%zu`; preserve the actual graph failure and nonzero status. |
| [#222](https://github.com/adesutherland/CREXX/security/code-scanning/222) | `lib/plugins/fs/rxfs.c:210` | Real pathname race: `rename_file` checks `stat(source)` at :202 and later renames that pathname, which can identify a different object. Its precheck only establishes existence, not type. Candidate: operate once with `rename`/`MoveFileExA` and map missing-source errors to the existing `-4` status; preserve other status cases. |
| [#223](https://github.com/adesutherland/CREXX/security/code-scanning/223) | `lib/plugins/fs/rxfs.c:184` | Same check/use race: `delete_file` checks `stat(path)` before `remove(path)`. Candidate: call the removal primitive once and classify its error, preserving stable-path return conventions. This still leaves a path-based operation, not a pinned object identity. |
| [#224](https://github.com/adesutherland/CREXX/security/code-scanning/224) | `lib/plugins/fs/rxfs.c:162` | Same check/use race: `remove_directory` checks `stat` and `S_ISDIR` before `rmdir`; `rmdir` itself enforces directory type. Candidate: call `rmdir` once and classify missing/non-directory versus other failures. |
| [#225](https://github.com/adesutherland/CREXX/security/code-scanning/225) | `compiler/rxcpfunc.c:3973` | Harmless true redundancy: `imported_variable_type_node` returns when `var->type` is null/empty at :3969, then uses `var->type ? var->type : ".unknown"`. The fallback is unreachable; use `var->type` directly if cleaning the alert. No missing-type dereference was established. |
| [#226](https://github.com/adesutherland/CREXX/security/code-scanning/226) | `lib/plugins/fs/rxfs_ops.h:97` | **Unresolved documented-contract/security question.** On Unix `rxfs_transfer` checks `lstat(source)` and final-symlink/type at :84, then uses the pathname in `link` (:85), no-overwrite `rename` (:88-93), or `open(O_NOFOLLOW)` (:97). Windows similarly checks `GetFileAttributesW` at :74-75 before pathname operations at :76-78. A concurrent replacement can change the source after the check. Unix `O_NOFOLLOW` rejects a substituted final symlink for copy, but not a substituted regular file; hardlink/move can operate on a substituted symlink. The library reference says transfer operations reject a final symlink. Do not weaken that promise or treat `fileguard` as an attacker-proof lock without a decision. Copy can classify the opened descriptor with `fstat`; hardlink/move need a precise cross-platform source-identity/no-symlink strategy or an explicitly approved contract change. |

The three legacy `rxfs` alerts (#222-224) share an avoidable precheck/use
pattern. The transfer alert (#226) is related but stronger because the
[published `rxfs` reference](../../books/crexx_library_reference/rxfs.md)
explicitly promises final-symlink rejection and destination non-overwrite.
Destination exclusion uses `O_EXCL`, `RENAME_EXCL`/`RENAME_NOREPLACE`, or
non-replacing Windows calls; these alerts concern the **source** identity.
The reference assigns parent traversal, permissions and transactional
publication to callers, but does not authorize silently relaxing the final
symlink promise. Unix `fileguard` is documented as cooperative and cannot stop
an attacker who ignores it.

## Proposed verification before any closure

1. For #220, exercise the semantic-graph resolution failure that takes
   `rxbin007.c:1059`, check the complete formatted error and nonzero result,
   then run the focused normal test on the changed binutils input. The `%zu`
   repair is independent of the native beta 3 boundary and should be reviewed
   as a small correctness fix.
2. If repairing #222-224, use a deterministic path-swap hook or controlled
   filesystem fixture between check and operation as a negative control;
   verify stable-path success and the historical `-4`/`-8` status cases on
   supported platforms. A single-operation implementation should eliminate
   the intra-function check/use window without promising cross-process
   transactionality.
3. For #226, decide the documented source-identity guarantee first. Verify
   final symlink and regular-file substitution for copy, hardlink and move;
   verify destination non-overwrite on each supported OS and explicit failure
   where a strong guarantee cannot be supplied. Do not assert portable
   atomicity from a second `lstat` or a cooperative fileguard.
4. #216-218/#225 need only source-level cleanup and affected focused compiler
   checks if changed. Their current alerts describe redundant checks, not
   evidence of a reachable null dereference.

This inventory does not close the nine alerts. It identifies one small
confirmed formatting defect (#220) and a material source-identity decision
for #226 before claiming an alert-clean beta 3 candidate. The older pathname
races are real patterns; their security impact depends on whether an untrusted
actor can mutate a path that a privileged cREXX process is operating on.
