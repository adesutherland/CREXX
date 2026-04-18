options levelb comments_dash
namespace id_id expose id
import id

/**
* class id provides identifier generation methods.
* It wraps the native id._* functions so the public
* methods remain recursion-safe and signature-aligned.
*
* All methods take a string argument `s` to match the
* native interface (arg0=.string).
* 
* @author René Vincent Jansen
* @author Peter Jacob
* 
*/

id: class

/** 
 * the class factory returns an instance of the id class
 */
  *: factory
    return

/** 
 * method uuid returns a UUIDv4 string.
 */
  uuid: method = .string
    return _uuid()

/** 
 * method uuidCompat returns the compatibility/demo UUIDv4 string.
 */
  uuidCompat: method = .string
    return _uuidt()

/** 
 * method uuidV7 returns a UUIDv7 string.
 */
  uuidV7: method = .string
    return _uuidv7()

/** 
 * method ulid returns a ULID string.
 */
  ulid: method = .string
    return _ulid()

/** 
 * method nanoId returns a NanoID string.
 */
  nanoId: method = .string
    return _nanoid()

/** 
 * method snowflake returns a Snowflake ID string.
 */
  snowflake: method = .string
    return _snowflake()

/** 
 * method base58 returns a Base58 identifier string.
 */
  base58: method = .string
    return _base58()
