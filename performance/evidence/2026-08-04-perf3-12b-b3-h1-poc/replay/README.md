# Generated-RXAS replay

Start from the accepted optimized generated RXAS with SHA-256
`49efe16928bc9720904d127325a93d1e7c39642eb2f592f2972e52fbdcdcb585`.
Copy it to a scratch directory as `benchmark_rexxcps_levelb_opt.rxas`, then
apply the patch in place:

```sh
patch -p0 < h1-cache-seed.patch
# resulting SHA-256: d30cc26b95da0f1b449fe5c1077288fc20c1a08c5d9c1303aac8a474989166ce
```

The patch deliberately modifies generated RXAS rather than canonical benchmark
source. It redirects the first existing joined key to new private local r103;
it does not add a runtime setup instruction. The implementation is the single
commit `80c78fcee628dd60fe4d572892718f4dda09a4fc`, based on
`83de3db3c2d42f3dfcd61a1242e37d3ef196437b`.
