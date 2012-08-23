//===- MCLDOutput.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  Output class inherits MCLDFile, which is used to represent a output file.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_OUTPUT_H
#define MCLD_OUTPUT_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <mcld/MC/MCLDFile.h>

namespace mcld {

/** \class MCLDOutput
 *  \brief MCLDOutput provides the information about the output.
 *
 *  @see MCLDFile
 */
class Output : public MCLDFile
{
public:
  Output();

  ~Output();
};

} // namespace of mcld

#endif

