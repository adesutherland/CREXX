/* rexx */
options levelb comments_dash

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
  buckets = .int[256]
  keys = .string[]
  values = .string[]
  next = .int[]
  count = .int

  *: factory
    num_buckets = 256
    count = 0
    return

  hash: method = .int
    arg key = .string
    
    h = 0
    len = length(key)
    if len = 0 then return 1
    
    do i = 1 to len
      char = substr(key, i, 1)
      val = c2d(char)
      h = h * 31 + val
    end
    
    rem = h % num_buckets
    if rem < 0 then rem = -rem
    return rem + 1

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

  size: method = .int
    return count

  key: method = .string
    arg ix = .int
    return keys[ix]

  value: method = .string
    arg kv = .string
    return values[kv]
    
