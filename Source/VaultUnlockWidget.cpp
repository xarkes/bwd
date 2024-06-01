#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPalette>
#include <qpushbutton.h>

#include "BWNetworkService.h"

#include "VaultUnlockWidget.h"
#include "BWLineEdit.h"

VaultUnlockWidget::VaultUnlockWidget()
{
  auto inputPassword = new BWLineEdit("Password", this);
  inputPassword->setEchoMode(QLineEdit::EchoMode::Password);
  QLabel* label = new QLabel("Enter password", this);

  auto buttonBack = new QPushButton("Back", this);
  buttonBack->setStyleSheet("color: blue; background-color: white;");
  auto buttonUnlock = new QPushButton("Unlock", this);
  buttonUnlock->setStyleSheet("color: blue; background-color: white;");

  auto layout = new QVBoxLayout();
  layout->setContentsMargins(50, 50, 50, 200);
  layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
  layout->addWidget(label, 0, Qt::AlignHCenter);
  layout->addWidget(inputPassword);
  layout->addWidget(buttonBack);
  layout->addWidget(buttonUnlock);

  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  topbar->setFixedHeight(50);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);

  connect(buttonUnlock, &QPushButton::released, [inputPassword](){
    Net()->login(inputPassword->text());
  });
  connect(buttonBack, &QPushButton::released, [this](){
    emit back();
  });
  connect(Net(), &BWNetworkService::loginDone, [this](){
    emit unlocked();
  });
}
