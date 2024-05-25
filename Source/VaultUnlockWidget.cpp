#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPalette>

#include "VaultUnlockWidget.h"
#include "BWLineEdit.h"

VaultUnlockWidget::VaultUnlockWidget()
{
  auto *inputPassword = new BWLineEdit("Password", this);
  inputPassword->setEchoMode(QLineEdit::EchoMode::Password);
  QLabel* label = new QLabel("Enter password", this);

  auto layout = new QVBoxLayout();
  layout->setContentsMargins(50, 50, 50, 200);
  layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
  layout->addWidget(label, 0, Qt::AlignHCenter);
  layout->addWidget(inputPassword);

  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  topbar->setFixedHeight(50);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);
}
