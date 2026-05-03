/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
 * method uuidt returns a time base uuid string.
  */
  uuidt: method = .string
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
