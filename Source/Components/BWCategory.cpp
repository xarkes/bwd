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

  m_contentArea->setLayout(new QVBoxLayout());
  m_contentArea->layout()->setContentsMargins(0, 0, 0, 0);
  m_contentArea->layout()->setAlignment(Qt::AlignTop);
  m_contentArea->setMaximumHeight(contentHeight);

  auto animThis = new QPropertyAnimation(this, "maximumHeight");
  animThis->setDuration(300);
  animThis->setStartValue(defaultHeight);
  animThis->setEndValue(collapsedHeight);
  m_toggleAnimation.addAnimation(animThis);
  auto animContent = new QPropertyAnimation(m_contentArea, "maximumHeight");
  animContent->setDuration(300);
  animContent->setStartValue(m_contentArea->layout()->sizeHint().height());
  animContent->setEndValue(0);
  m_toggleAnimation.addAnimation(animContent);

  setLayout(new QVBoxLayout());
  layout()->addWidget(m_toggleButton);
  layout()->addWidget(m_contentArea);
  layout()->setContentsMargins(0, 0, 0, 0);

  QObject::connect(m_toggleButton, &QPushButton::clicked, [this](const bool checked) {
    m_toggleAnimation.setDirection(checked ? QAbstractAnimation::Direction::Backward : QAbstractAnimation::Direction::Forward);
    m_toggleAnimation.start();
  });
}
