# Stack plugin

The stack plugin provides small list, stack, and queue helpers over a
`.string[]` array exposed from the caller. The current implementation lives in
`stack.c` and uses the RXPA array attribute helpers, so the caller owns the
array and the plugin mutates it in place.

Import the plugin package before calling the functions. See
`stack_test.crexx` for a runnable example.

## Registered procedures

| Procedure | Registered result | Arguments | Notes |
|-----------|-------------------|-----------|-------|
| `stack.push` | `.int` | `expose list=.string[], ll_arg=.string` | Alias of `stack.additem`; appends at the end. |
| `stack.additem` | `.int` | `expose list=.string[], ll_arg=.string` | Appends at the end and returns the new high-water mark. |
| `stack.queue` | `.string` | `expose list=.string[], ll_arg=.string` | Inserts at the front. |
| `stack.pull` | `.string` | `expose list=.string[]` | Removes and returns the last item. |
| `stack.pullq` | `.string` | `expose list=.string[]` | Removes and returns the first item. |
| `stack.insertitem` | `.int` | `expose list=.string[], ll_index=.int, ll_arg=.string` | Inserts at the requested 1-based position, clamped to the list bounds. |
| `stack.delitem` | `.int` | `expose list=.string[], ll_index=.int` | Deletes a 1-based item; returns `4` for an invalid index. |
| `stack.moveitem` | `.int` | `expose list=.string[], ll_from=.int, ll_to=.int` | Moves an item and returns the final position; returns `4` for an invalid source. |
| `stack.swapitem` | `.int` | `expose list=.string[], ll_indx1=.int, ll_indx2=.int` | Swaps two 1-based items; returns `4` for an invalid index. |
| `stack.finditem` | `.int` | `expose list=.string[], ll_arg=.string, ll_index=.int` | Finds the first item containing `ll_arg`, starting at `ll_index`; returns `0` when not found. |
| `stack.listitems` | `.int` | `expose list=.string[]` | Prints the current items with 1-based indexes. |
| `stack.createll` | `.int` | `expose list=.string[], ll_arg=.string` | Clears the exposed list. |

The plugin does not currently add bounds checks for empty `pull` or `pullq`.
Callers should check the array high-water mark before removing an item from an
empty list.
