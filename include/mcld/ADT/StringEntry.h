//===- StringEntry.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_STRING_ENTRY_H
#define MCLD_STRING_ENTRY_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif
#include <llvm/ADT/StringRef.h>
#include <cstdlib>
#include <cassert>

namespace mcld
{
template<typename DataType>
class StringEntryFactory;

/** \class StringEntry
 *  \brief StringEntry is a pair of strings which is designed for high locality.
 */
template<typename DataType>
class StringEntry
{
public:
  typedef llvm::StringRef   key_type;
  typedef DataType value_type;

public:
  key_type key()
  { return key_type(m_Key, m_KeyLen); }

  const key_type key() const
  { return key_type(m_Key, m_KeyLen); }

  value_type& value()
  { return m_Value; }

  const value_type& value() const
  { return m_Value; }

  size_t getKeyLength() const
  { return m_KeyLen; }

  size_t getValueLength() const
  { return m_Value.size(); }

  void setValue(const DataType& pVal)
  { m_Value = pVal; }

  bool compare(const StringEntry& pX, const StringEntry& pY) const
  { return (0 == pX.key().compare(pY.key())); }

private:
  StringEntry();
  StringEntry(const key_type& pKey);
  StringEntry(const StringEntry& pCopy);
  ~StringEntry();

private:
  DataType m_Value;
  uint16_t m_KeyLen;
  char m_Key[0];

  friend class StringEntryFactory<DataType>;
};

template<typename DataType>
class StringEntryFactory
{
public:
  typedef StringEntry<DataType>             entry_type;
  typedef typename StringEntry<DataType>::key_type   key_type;
  typedef typename StringEntry<DataType>::value_type value_type;

public:
  StringEntryFactory();
  ~StringEntryFactory();

  StringEntry<DataType>* produce(const key_type& pKey);
  void destroy(StringEntry<DataType>* pEntry);
};

#include "StringEntry.tcc"

} // namespace of mcld

#endif

