# Bundled SQLite amalgamation

CREXX bundles the official SQLite 3.53.2 amalgamation so the supported
`rxsqlite` component builds without network access and carries its native
dependency inside both the dynamic plugin and the static provider archive.

Source archive:
`https://www.sqlite.org/2026/sqlite-amalgamation-3530200.zip`

- archive SHA3-256: `81142986038e18f96c4a54e1a72562ae17e502a916f2a7701eff43388cbf1a40`
- `sqlite3.c` SHA-256: `0a409f1633283fa31a9126b11fbfd64a1991c5d30defad07e5745d4667f5e23d`
- `sqlite3.h` SHA-256: `9e69a1353a4288450b0d5239ede11fc7f1f4c8e5eb07491fc8317eacb5b7de7e`

SQLite is in the public domain. See <https://www.sqlite.org/copyright.html>.
The CREXX provider around it is licensed under CREXX's MIT licence.

The build fixes the public contract features at `SQLITE_THREADSAFE=1` and
`SQLITE_ENABLE_FTS5=1`. It also disables loadable extensions and legacy
double-quoted string literals, and enables API armor and secure delete.
