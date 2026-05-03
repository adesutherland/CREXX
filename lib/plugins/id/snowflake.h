#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <stdint.h>

/* ----- Tunables (override via -D on your build if you like) ----- */

/* Custom epoch (Twitter epoch by default: 2010-11-04T01:42:54.657Z) */
#ifndef SNOWFLAKE_EPOCH_MS
#define SNOWFLAKE_EPOCH_MS 1288834974657ULL
#endif

/* Bit layout: 41 bits timestamp, 10 bits node id, 12 bits sequence */
#ifndef SNOWFLAKE_NODE_BITS
#define SNOWFLAKE_NODE_BITS 10
#endif

#ifndef SNOWFLAKE_SEQ_BITS
#define SNOWFLAKE_SEQ_BITS 12
#endif

/* Max values derived from bit widths */
#define SNOWFLAKE_MAX_NODE  ((1U << SNOWFLAKE_NODE_BITS) - 1)
#define SNOWFLAKE_MAX_SEQ   ((1U << SNOWFLAKE_SEQ_BITS) - 1)

/* ----- API ----- */

/* Optionally set a fixed node id (0..SNOWFLAKE_MAX_NODE). If not set, an
   auto-derived id from hostname+PID is used. Returns 1 on success, 0 on bad id. */
int snowflake_set_node(uint16_t node_id);

/* Generate next Snowflake ID as a 64-bit integer. Returns 1 on success. */
int snowflake_next_u64(uint64_t *out);

/* Generate next Snowflake ID as a decimal string. out must be char[21].
   (Max 20 digits for uint64 + NUL). Returns 1 on success. */
int snowflake_next_str(char out[21]);

#endif /* SNOWFLAKE_H */
