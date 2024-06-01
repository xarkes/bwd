#include "BWCategory.h"
#include <QPropertyAnimation>
#include <qpropertyanimation.h>


BWCategory::BWCategory(QString text, QWidget* parent) : QWidget(parent)
{
  m_toggleButton = new BWCategoryEntry(text, "", this);
  m_toggleButton->setChecked(true);
  m_contentArea = new QScrollArea(this);

  const auto contentHeight = m_contentArea->height();
  const auto defaultHeight = 500;
  const auto collapsedHeight = 50;

  auto cl = new QVBoxLayout();
  m_contentArea->setLayout(cl);
  m_contentArea->setMaximumHeight(contentHeight);
  cl->addWidget(new BWCategoryEntry("inner item"));

  auto animThis = new QPropertyAnimation(this, "maximumHeight");
  animThis->setDuration(300);
  animThis->setStartValue(defaultHeight);
  animThis->setEndValue(collapsedHeight);
  m_toggleAnimation.addAnimation(animThis);
  auto animContent = new QPropertyAnimation(m_contentArea, "maximumHeight");
  animContent->setDuration(300);
  animContent->setStartValue(cl->sizeHint().height());
  animContent->setEndValue(0);
  m_toggleAnimation.addAnimation(animContent);

  m_layout.addWidget(m_toggleButton);
  m_layout.addWidget(m_contentArea);
  m_layout.setContentsMargins(0, 0, 0, 0);
  setLayout(&m_layout);

  QObject::connect(m_toggleButton, &QPushButton::clicked, [this](const bool checked) {
    m_toggleAnimation.setDirection(checked ? QAbstractAnimation::Direction::Backward : QAbstractAnimation::Direction::Forward);
    m_toggleAnimation.start();
  });
}
