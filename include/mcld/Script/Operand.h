//===- Operand.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_OPERAND_INTERFACE_H
#define MCLD_SCRIPT_OPERAND_INTERFACE_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/Script/ExprToken.h>
#include <mcld/Support/Allocators.h>
#include <mcld/Config/Config.h>
#include <llvm/Support/DataTypes.h>
#include <string>
#include <cassert>

namespace mcld
{

/** \class Operand
 *  \brief This class defines the interfaces to an operand token.
 */

class Operand : public ExprToken
{
public:
  enum Type {
    SYMBOL,
    INTEGER,
    SECTION
  };

protected:
  Operand(Type pType);
  virtual ~Operand();

public:
  Type type() const { return m_Type; }

  virtual bool isDOT() const { return false; }

  virtual uint64_t value() const = 0;

  static bool classof(const ExprToken* pToken)
  {
    return pToken->kind() == ExprToken::OPERAND;
  }

private:
  Type m_Type;
};

/** \class SymOperand
 *  \brief This class defines the interfaces to a symbol operand.
 */

class SymOperand : public Operand
{
private:
  friend class Chunk<SymOperand, MCLD_SYMBOLS_PER_INPUT>;
  SymOperand();
  SymOperand(const std::string& pName);

public:
  void dump() const;

  const std::string& name() const { return m_Name; }

  bool isDot() const;

  uint64_t value() const { return m_Value; }

  void setValue(uint64_t pValue) { m_Value = pValue; }

  static bool classof(const Operand* pOperand)
  {
    return pOperand->type() == Operand::SYMBOL;
  }

  /* factory method */
  static SymOperand* create(const std::string& pName);
  static void destroy(SymOperand*& pOperand);
  static void clear();

private:
  std::string m_Name;
  uint64_t m_Value;
};

/** \class IntOperand
 *  \brief This class defines the interfaces to an integer operand.
 */

class IntOperand : public Operand
{
private:
  friend class Chunk<IntOperand, MCLD_SYMBOLS_PER_INPUT>;
  IntOperand();
  IntOperand(uint64_t pValue);

public:
  void dump() const;

  uint64_t value() const { return m_Value; }

  void setValue(uint64_t pValue) { m_Value = pValue; }

  static bool classof(const Operand* pOperand)
  {
    return pOperand->type() == Operand::INTEGER;
  }

  /* factory method */
  static IntOperand* create(uint64_t pValue);
  static void destroy(IntOperand*& pOperand);
  static void clear();

private:
  uint64_t m_Value;
};

/** \class SectOperand
 *  \brief This class defines the interfaces to an section name operand.
 */

class SectOperand : public Operand
{
private:
  friend class Chunk<SectOperand, MCLD_SECTIONS_PER_INPUT>;
  SectOperand();
  SectOperand(const std::string& pName);

public:
  void dump() const;

  const std::string& name() const { return m_Name; }

  uint64_t value() const
  {
    assert(0);
    return 0;
  }

  static bool classof(const Operand* pOperand)
  {
    return pOperand->type() == Operand::SECTION;
  }

  /* factory method */
  static SectOperand* create(const std::string& pName);
  static void destroy(SectOperand*& pOperand);
  static void clear();

private:
  std::string m_Name;
};

} // namespace of mcld

#endif

