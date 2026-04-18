options levelb comments_dash
namespace data_KeyDB expose KeyDB
import keyaccess

/**
 * Class KeyDB
 *
 * This module implements a transactional key-value database with the following features:
 * - Key-value storage with CRUD operations
 * - Transaction support (begin, commit, rollback)
 * - LRU cache for improved read performance
 * - File locking for concurrent access
 * - Database maintenance (backup, compact, validate)
 * - Error logging and statistics
 *
 * File Structure:
 * - Data file: Stores actual key-value data
 * - Index file: Maintains key locations and metadata
 * - Log file: Records operations and errors
 *
 * Error Codes:
 * - KA_SUCCESS (0): Operation successful
 * - KA_ERROR_PARAM (-1): Invalid parameters
 * - KA_ERROR_MEMORY (-2): Memory allocation failed
 * - KA_ERROR_IO (-3): I/O operation failed
 * - KA_ERROR_LOCK (-4): File locking failed
 * - KA_ERROR_NOTFOUND (-5): Key not found
 * - KA_ERROR_EXISTS (-6): Key already exists
 * - KA_ERROR_CORRUPT (-7): Database corruption detected
 * - KA_ERROR_TXACTIVE (-8): Transaction already active
 * - KA_ERROR_TXINACTIVE (-9): No active transaction
 * - KA_ERROR_TOOLONG (-10): Key or value too long
 *
 * Example Usage:
 * ```rexx
 * handle = keyaccess.openkey("mydb.dat", "w+")
 * call keyaccess.txbegin handle
 * call keyaccess.writekey handle, "name", "John Doe"
 * call keyaccess.txcommit handle
 * value = keyaccess.readkey(handle, "name")
 * call keyaccess.closekey handle
 * ```
 *
 * @author Peter Jacobs
 * @author René Vincent Jansen
 * @date 2024-03-20
 * @version 1.0
 */

KeyDB: class
  handle = .int

  /**
   * Factory creates a closed keydb object.
   */
  *: factory
    handle = 0
    return

  /**
   * Open or create a database.
   * @parm .string filename
   * @parm .string mode     "r", "w", or "w+"
   * @return .int          handle or negative error code from plugin
   */
  open: method = .int
    arg filename = .string, mode = .string
    handle = _openkey(filename, mode)
    return handle

  /**
   * Close the database if open.
   * @return .int status code
   */
  close: method = .int
    if handle = 0 then return 0
    rc = _closekey(handle)
    if rc = 0 then handle = 0
    return rc

  /**
   * Return 1 if this object currently has an open native handle, else 0.
   */
  isOpen: method = .int
    if handle <> 0 then return 1
    return 0

  /**
   * Begin transaction.
   * @return .int status code
   */
  begin: method = .int
    return _txbegin(handle)

  /**
   * Commit transaction.
   * @return .int status code
   */
  commit: method = .int
    return _txcommit(handle)

  /**
   * Roll back transaction.
   * @return .int status code
   */
  rollback: method = .int
    return _txrollback(handle)

  /**
   * Store a key/value pair.
   * Note: plugin requires an active transaction.
   * @parm .string key
   * @parm .string value
   * @return .int status code
   */
  put: method = .int
    arg key = .string, value = .string
    return _writekey(handle, key, value)

  /**
   * Read a value by key.
   * Plugin returns a string value, "NOT_FOUND", or "ERROR".
   * @parm .string key
   * @return .string
   */
  get: method = .string
    arg key = .string
    return _readkey(handle, key)

  /**
   * Delete a key.
   * Note: plugin requires an active transaction.
   * @parm .string key
   * @return .int status code
   */
  remove: method = .int
    arg key = .string
    return _deletekey(handle, key)

  /**
   * Count non-deleted keys.
   * @return .int
   */
  size: method = .int
    return _listkey(handle)

  /**
   * Return formatted statistics text.
   * @return .string
   */
  stats: method = .string
    return _stats(handle)

  /**
   * Back up the database.
   * @parm .string path
   * @return .int
   */
  backup: method = .int
    arg path = .string
    return _backup(handle, path)

  /**
   * Validate the database.
   * Returns number of errors found or negative error code.
   * @return .int
   */
  validate: method = .int
    return _validate(handle)

  /**
   * Compact the database.
   * @return .int
   */
  compact: method = .int
    return _compact(handle)

  /**
   * Convenience method: 1 if key exists, else 0.
   * This interprets the plugin's string return.
   * @parm .string key
   * @return .int
   */
  containsKey: method = .int
    arg key = .string
    value = get(key)
    if value = 'NOT_FOUND' then return 0
    if value = 'ERROR' then return 0
    return 1

  /**
   * Convenience method: write a single key/value in its own transaction.
   * @parm .string key
   * @parm .string value
   * @return .int
   */
  putCommit: method = .int
    arg key = .string, value = .string
    rc = begin()
    if rc <> 0 then return rc
    rc = put(key, value)
    if rc <> 0 then do
      call rollback()
      return rc
    end
    return commit()

  /**
   * Convenience method: delete a single key in its own transaction.
   * @parm .string key
   * @return .int
   */
  removeCommit: method = .int
    arg key = .string
    rc = begin()
    if rc <> 0 then return rc
    rc = remove(key)
    if rc <> 0 then do
      call rollback()
      return rc
    end
    return commit()

  /**
   * Return the raw native handle.
   * @return .int
   */
  getHandle: method = .int
    return handle

  /**
   * Finalizer-style cleanup helper.
   */
  dispose: method = .int
    return close()
