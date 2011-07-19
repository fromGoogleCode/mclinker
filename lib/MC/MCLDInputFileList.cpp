/*****************************************************************************
 *   The MCLinker Project, Copyright (C), 2011 -                             *
 *   Embedded and Web Computing Lab, National Taiwan University              *
 *   MediaTek, Inc.                                                          *
 *                                                                           *
 *   Duo <pinronglu@gmail.com>                                               *
 ****************************************************************************/
#include <mcld/MC/MCLDInputFileList.h>
#include <mcld/MC/MCLDFile.h>


using namespace mcld;

//==========================
// MCLDInputFileList

MCLDInputFileList::MCLDInputFileList () 
  : m_pHead(0),
    m_pTail(0) { 
  MCLDFile *NullFile = new MCLDFile();
  m_pEnd= new Node(*NullFile, UNKNOWN);
}

MCLDInputFileList::~MCLDInputFileList() {
  MCLDInputFileList::iterator it = begin() ;
  while (it != end()){
    iterator temp_it = it++;
    delete temp_it.m_pNode; 
  } 
  delete m_pEnd;
}

MCLDInputFileList &MCLDInputFileList::insert(MCLDFile &file, FileAttribute attr) {
  
  Node *newNodePtr = new Node(file, attr);

  if(!m_pHead) { 
    m_pHead = newNodePtr;
  }
  else if (m_pTail->m_Attr == MCLDInputFileList::GROUP_START) {
    m_Stack.push(m_pTail);
    m_pTail->m_pChild = newNodePtr;  
  }
  else if (m_pTail->m_Attr == MCLDInputFileList::GROUP_END) { 
    m_pTail->m_pNext = 0;
    m_pTail = m_Stack.top();
    m_Stack.pop();
    m_pTail->m_pNext = newNodePtr;
  }
  else {
    m_pTail->m_pNext = newNodePtr; 
  }

  m_pTail = newNodePtr; 
  m_pTail->m_pNext = m_pEnd;
  return *this;
}

MCLDInputFileList &MCLDInputFileList::insert(iterator position,
                                             iterator it_begin,
                                             iterator it_end) {
  if(position.m_pNode->m_pChild) {
    std::cerr << "error : m_pChild already exist\n";
    return *this;
  }
  if (!position.m_pNode->m_pChild) {
    position.m_pNode->m_pChild = it_begin.m_pNode;  
    it_end.m_pNode->m_pNext = 0;
  }
  return *this;
}


/// -------  MCLDInputFileList::Node  -------
MCLDInputFileList::Node::Node(MCLDFile &file,FileAttribute attr) 
  : m_File(file), 
    m_pNext(0),
    m_pChild(0),
    visited(false),
    m_Attr(attr) {
}

MCLDInputFileList::Node::~Node() {
  delete &m_File;
}


/// ------- iterator ---------
MCLDInputFileList::iterator &MCLDInputFileList::iterator::operator++() {

  if (m_pNode->m_pChild) {
    m_ITStack.push(m_pNode) ;
    m_pNode = m_pNode->m_pNext;
  }
  else if (!m_pNode->m_pNext) { 
    m_pNode = m_ITStack.top();
    m_ITStack.pop() ;
  }
  else 
    m_pNode = m_pNode->m_pNext;

  return *this;
}

MCLDInputFileList::iterator MCLDInputFileList::iterator::operator++(int) {
  iterator tmp = *this;
  ++*this;
  return tmp;
}

