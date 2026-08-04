# Generated-RXAS replay

Start from the accepted optimized generated RXAS with SHA-256
`49efe16928bc9720904d127325a93d1e7c39642eb2f592f2972e52fbdcdcb585`.
Copy it to a scratch directory as `benchmark_rexxcps_levelb_opt.rxas`, then
apply one patch in place:

```sh
patch -p0 < s1-left-load.patch
# resulting SHA-256: ba25f2b8041fce10697afa47088d0b21ac4712372b66d3057bbe78f2935d785a
```

or:

```sh
patch -p0 < same-shaped-control.patch
# resulting SHA-256: 4cb64e13cc6a679b66d36995eaad0d4cf5e654439485d613660a536af835d148
```

Use a fresh copy for each patch. The patches deliberately modify generated
RXAS rather than canonical benchmark source. The implementation itself is the
single commit `888fa94eb0d5b14aa4d3a9a028117a424c6358be` based on
`83de3db3c2d42f3dfcd61a1242e37d3ef196437b`.
