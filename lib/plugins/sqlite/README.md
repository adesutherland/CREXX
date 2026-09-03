# rxsqlite

`rxsqlite` is CREXX's supported, generic typed SQLite provider. It publishes
the `rxsqlite` cREXX namespace, the `rxsqlite.rxplugin` dynamic provider and a
canonical `rxsqlite` static archive used automatically by `crexx --native`.
The installed `rxsqlite_address.rxbin` module is a separate Level-G façade for
`ADDRESS SQLITE`; it contains no native SQLite implementation.

The component was donated from the tracked `crexx-rag` implementation at
revision `d0a3b26f7ea22a745f5b5828bbce2f5ac1d0070a` under Adrian Sutherland's
licensing authority and is distributed under CREXX's MIT licence. The bundled
SQLite amalgamation is public-domain third-party code; its provenance and
checksums are recorded in `vendor/README.md`.

Full API, ownership, concurrency, packaging and ADDRESS documentation is in
`docs/books/crexx_library_reference/rxsqlite.md`.
