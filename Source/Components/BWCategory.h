#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QParallelAnimationGroup>

#include "BWCategoryEntry.h"

class BWCategory : public QWidget
{
  Q_OBJECT;
public:
  BWCategory(QString label, QWidget* parent=nullptr);
  
private:
  QGridLayout m_layout;
  BWCategoryEntry* m_toggleButton;
  QScrollArea* m_contentArea;
  QParallelAnimationGroup m_toggleAnimation;
};
