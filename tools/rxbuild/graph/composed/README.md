# Composed graph example

Run from the repository root:

```powershell
.\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\graph\composed\main.graph.build `
  --verbose
```

`main.graph.build` includes `nested/demo.graph.build`. The nested graph uses
its own `BUILD_DIR` and `SOURCE` variables. Its `nested_program` target is
resolved by the main graph because the name is unique across the composed
graph. The nested graph's default is not selected; the main graph's
`main_program` default is used.
