#include "BWEntry.h"

BWEntry::BWEntry(QString label) : BWButton(label)
{
  m_colBg = BWS::item;
  m_colBgOver = BWS::itemOver;
}
