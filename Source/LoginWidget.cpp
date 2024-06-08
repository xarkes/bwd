#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPalette>

#include "LoginWidget.h"
#include "BWLineEdit.h"
#include "BWNetworkService.h"

LoginWidget::LoginWidget()
{
  m_inputEmail = new BWLineEdit("Email address", this);
  m_inputServer = new BWLineEdit("Server", this);
  m_buttonLogin = new QPushButton("Continue", this);

  // XXX: Remove me
  m_inputEmail->setText("test@test.com");

  // QPalette palette = m_buttonLogin->palette();
  // palette.setColor(QPalette::ColorRole::ButtonText, QColorConstants::Blue);
  // m_buttonLogin->setPalette(palette);
  m_buttonLogin->setStyleSheet("color: blue; background-color: white;");

  QPixmap image(":/Images/logo-dark@2x.png");
  QLabel *logoLabel = new QLabel();
  logoLabel->setPixmap(image);

  QLabel* label = new QLabel("Log in to access your secure vault", this);

  auto layout = new QVBoxLayout();
  layout->setContentsMargins(50, 50, 50, 200);
  layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
  layout->addWidget(logoLabel, 0, Qt::AlignHCenter);
  layout->addWidget(label, 0, Qt::AlignHCenter);
  layout->addWidget(m_inputEmail);
  layout->addWidget(m_inputServer);
  layout->addWidget(m_buttonLogin);

  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  topbar->setFixedHeight(50);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);

  connect(m_buttonLogin, &QPushButton::released, [this](){
    setLoadingScreen(true);
    Net()->preLogin(m_inputEmail->text(), m_inputServer->text());
  });
  connect(Net(), &BWNetworkService::preLoginDone, [this](bool success){
    setLoadingScreen(false);
    qDebug() << "prelogin " << success;
    if (success) {
      emit loginOk();
      Net()->sync();
    }
  });
}

void LoginWidget::setLoadingScreen(bool loading)
{
  m_buttonLogin->setDisabled(loading);
  m_inputEmail->setDisabled(loading);
  m_inputServer->setDisabled(loading);
}
