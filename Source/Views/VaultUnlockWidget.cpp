#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPalette>

#include "VaultUnlockWidget.h"
#include "BWNetworkService.h"
#include <Components/BWLineEdit.h>
#include <Components/BWBusyIndicator.h>

VaultUnlockWidget::VaultUnlockWidget()
{
  m_inputPassword = new BWLineEdit("Password", this);
  m_inputPassword->setEchoMode(QLineEdit::EchoMode::Password);
  QLabel* label = new QLabel("Enter password", this);

  // XXX Remove me
  m_inputPassword->setText("qweqweqwe123");

  m_buttonBack = new QPushButton("Back", this);
  m_buttonBack->setStyleSheet("color: blue; background-color: white;");
  m_buttonUnlock = new QPushButton("Unlock", this);
  m_buttonUnlock->setStyleSheet("color: blue; background-color: white;");

  m_busyIndicator = new BWBusyIndicator();
  m_busyIndicator->setVisible(false);

  auto layout = new QVBoxLayout();
  layout->setContentsMargins(50, 50, 50, 200);
  layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
  layout->addWidget(label, 0, Qt::AlignHCenter);
  layout->addWidget(m_inputPassword);
  layout->addWidget(m_buttonBack);
  layout->addWidget(m_buttonUnlock);
  layout->addWidget(m_busyIndicator);

  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  topbar->setFixedHeight(50);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);
  
  connect(m_buttonUnlock, &QPushButton::released, [this]() {
    Net()->login(m_inputPassword->text());
    setLoading(true);
  });
  connect(m_buttonBack, &QPushButton::released, [this]() {
    emit back();
  });
  connect(Net(), &BWNetworkService::loginDone, [this](bool success) {
    setLoading(false);
    if (success) {
      emit unlocked();
    }
    // TOOD: Print error message
  });
}

void VaultUnlockWidget::setLoading(bool loading)
{
  m_inputPassword->setDisabled(loading);
  m_busyIndicator->setVisible(loading);
}
