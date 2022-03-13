// //////////////////////////////////////////////////////////
// sha1.h
// Copyright (c) 2014 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#pragma once

#include "../hashing.h"

/// compute SHA1 hash
/** Usage:
    SHA1 sha1;
    std::string myHash  = sha1("Hello World");     // std::string
    std::string myHash2 = sha1("How are you", 11); // arbitrary data, 11 bytes

    // or in a streaming fashion:

    SHA1 sha1;
    while (more data available)
      sha1.add(pointer to fresh data, number of new bytes);
    std::string myHash3 = sha1.getHash();
  */
class SHA1 //: public Hash
{
public:
  /// same as reset()
  SHA1();

  /// compute SHA1 of a memory block
  const char* operator()(const void* data, size_t numBytes);
  /// compute SHA1 of a string, excluding final zero
  const char* operator()(const char* text, size_t size);

  /// add arbitrary number of bytes
  void add(const void* data, size_t numBytes);

  /// return latest hash as 16 hex characters
  const char* getHash();

  /// restart
  void reset();

private:
  /// process 64 bytes
  void processBlock(const void* data);
  /// process everything left in the internal buffer
  void processBuffer();

  /// split into 64 byte blocks (=> 512 bits)
  enum { BlockSize = 512 / 8, HashValues = 5 };

  /// size of processed data in bytes
  uint64_t m_numBytes;
  /// valid bytes in m_buffer
  size_t   m_bufferSize;
  /// bytes not processed yet
  uint8_t  m_buffer[BlockSize];
  /// hash, stored as integers
  uint32_t m_hash[HashValues];
};
