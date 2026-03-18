/* rexx */
options levelb

namespace rxfnsb expose stem

/* ------------------------------------------------------------------
 *  stem
 *
 *  A string-to-string hash map implementing classic Rexx compound
 *  variables behaviour.
 *
 * ------------------------------------------------------------------ */

stem: class
  num_buckets = .int
  buckets = .int[]
  keys = .string[]
  values = .string[]
  next = .int[]
  count = .int

  *: factory
    num_buckets = 256
    buckets = .int[256]
    keys = .string[1024]
    values = .string[1024]
    next = .int[1024]
    count = 0
    return

  hash: method = .int
    arg key = .string
    /* 
     * NOTE: Currently cREXX VM segfaults when doing string operations 
     * (like length, substr, c2d, or assembler strlen) inside a class method.
     * Therefore, all keys hash to bucket 1 for now, making this behave
     * like a linear search (Option 1 performance) but using Option 2's structure.
     * This will be fixed once the VM bug is resolved.
     */
    return 1

  get: method = .string
    arg key = .string
    
    b = hash(key)
    idx = buckets[b]
    
    do while idx > 0
      if keys[idx] = key then return values[idx]
      idx = next[idx]
    end
    
    return ""

  set: method = .void
    arg key = .string, value = .string
    
    b = hash(key)
    idx = buckets[b]
    last_idx = 0
    
    /* Update existing if found */
    do while idx > 0
      if keys[idx] = key then do
        values[idx] = value
        return
      end
      last_idx = idx
      idx = next[idx]
    end
    
    /* Need to add new entry. */
    count = count + 1
    
    /* Insert at end of chain */
    keys[count] = key
    values[count] = value
    next[count] = 0
    
    if last_idx > 0 then do
      next[last_idx] = count
    end
    else do
      buckets[b] = count
    end
    
    return
