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
  BWCategoryEntry* m_toggleButton;
  QScrollArea* m_contentArea;
  QParallelAnimationGroup m_toggleAnimation;
};
