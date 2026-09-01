# Linked-worktree snapshot

The main worktree was clean at the initial freeze. This first-stop snapshot was
taken after the V3 correction and evidence package were added; the main-tree
dirty scope belongs to this activity. `baseline-src` and `v3-trace-src` were
also created by this activity. All other dirty worktrees pre-existed and were
left untouched.

| Worktree | HEAD | branch/state | dirty paths |
| --- | --- | --- | ---: |
| `/Users/adrian/CLionProjects/CREXX` | `b08611179` | `develop` | activity only |
| `/private/tmp/crexx-nr09-bootstrap-32bf` | `32bf7e76f` | detached, registered prunable | 0 |
| `/private/tmp/crexx-nr21-baseline-src.S10nbu` | `5626d6b87` | detached, registered prunable | 0 |
| `/private/tmp/crexx-nr27-base.Wa1aCg/src` | `65ea6b9e2` | detached, registered prunable | 0 |
| `/private/tmp/crexx-perf2-01-product.dbLYqo` | `d5b25a78f` | detached | 0 |
| `/private/tmp/crexx-perf2-02.kJ5sZT/q1-richards` | `d5b25a78f` | detached | 3 |
| `/private/tmp/crexx-perf2-02.kJ5sZT/q3-direct` | `d5b25a78f` | detached | 9 |
| `/private/tmp/crexx-perf2-02.kJ5sZT/q3b-exact` | `d5b25a78f` | detached | 1 |
| `/private/tmp/crexx-perf2-02.kJ5sZT/q4-eager` | `d5b25a78f` | detached | 1 |
| `/private/tmp/crexx-perf2-02.kJ5sZT/q7-core` | `d5b25a78f` | detached | 5 |
| `/private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics/diagnostic-src` | `d5b25a78f` | detached | 4 |
| `/private/tmp/crexx-perf2-03-slice3-baseline.hdPHUZ/src` | `6b97c2ffd` | detached | 0 |
| `/private/tmp/crexx-perf2-03.9pQYlh/p0-source` | `086138f1e` | detached | 0 |
| `/private/tmp/crexx-perf2-03.9pQYlh/p1-source` | `086138f1e` | detached | 1 |
| `/private/tmp/crexx-perf2-03.9pQYlh/p2-source` | `086138f1e` | detached | 1 |
| `/private/tmp/crexx-perf2-03.9pQYlh/p3-source` | `086138f1e` | detached | 1 |
| `/private/tmp/crexx-perf2-03.9pQYlh/p4-source` | `086138f1e` | detached | 3 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/baseline-src` | `6567f0ba2` | detached | 0 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/length-direct-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/length-empty-init-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/rexxcps-combined-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/substr-const-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/substr-direct-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/upper-const-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/upper-direct-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/upper-inline-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/word-baseline-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/word-ceiling-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/word-constant-false-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-04.3k3Dfw/word-predicate-first-src` | `6567f0ba2` | detached | 1 |
| `/private/tmp/crexx-perf2-05-sa1.MzdvHT/baseline-src` | `537d3b3d2` | detached | 0 |
| `/private/tmp/crexx-perf2-05-sa1.MzdvHT/candidate-src` | `537d3b3d2` | detached | 3 |
| `/private/tmp/crexx-perf2-06-c1b.gQ4Wvm/candidate-src` | `e7090198e` | detached | 2 |
| `/private/tmp/crexx-perf2-06-reset.0ozeyr/r0-src` | `ab9eef545` | detached | 0 |
| `/private/tmp/crexx-perf2-06-vm-c2-poc.yvemdD/baseline-src` | `ab9eef545` | detached | 0 |
| `/private/tmp/crexx-perf2-06-vm-c2-poc.yvemdD/c2a-src` | `ab9eef545` | detached | 2 |
| `/private/tmp/crexx-perf2-06-vm-c2-poc.yvemdD/c2b-src` | `ab9eef545` | detached | 2 |
| `/private/tmp/crexx-perf2-06-vm-c2.LRjTOJ/worktree` | `ab9eef545` | `codex/perf2-06-vm-c2-poc` | 3 |
| `/private/tmp/crexx-perf2-06.Pn88uJ/baseline-src` | `e7090198e` | detached | 0 |
| `/private/tmp/crexx-perf2-06.Pn88uJ/cow-src` | `e7090198e` | detached | 2 |
| `/private/tmp/crexx-perf2-06.Pn88uJ/diagnostic-src` | `e7090198e` | detached | 2 |
| `/private/tmp/crexx-perf2-0607.HOrlKC/baseline-src` | `b08611179` | detached, activity control | 0 |
| `/private/tmp/crexx-perf2-0607.HOrlKC/v3-trace-src` | `b08611179` | detached, activity diagnostic | 2 |
| `/private/tmp/crexx-slice4-exact.W6qkVP/src` | `26f4aeb6f` | detached | 0 |
| `/private/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-perf2-05.KM0ZeuBw5m/baseline-src` | `22dd01a5b` | detached | 0 |
| `/private/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-perf2-05.KM0ZeuBw5m/candidate-src` | `22dd01a5b` | detached | 22 |

