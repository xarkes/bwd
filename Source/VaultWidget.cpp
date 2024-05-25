#include <QHBoxLayout>
#include <QLabel>

#include "VaultWidget.h"

VaultWidget::VaultWidget()
{
  QList<QString> pswEntries = {"One", "Two", "Three"};

  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  topbar->setFixedHeight(50);

  /* Left pane */
  auto lp = new QVBoxLayout();
  lp->addWidget(new QLabel("All items"));
  lp->addWidget(new QLabel("Favorites"));
  lp->addWidget(new QLabel("Trash"));
  auto leftPane = new QWidget(this);
  leftPane->setLayout(lp);

  /* Middle pane */
  auto mp = new QVBoxLayout();
  for (auto entry : pswEntries) {
    mp->addWidget(new QLabel(entry));
  }
  auto midPane = new QWidget(this);
  midPane->setLayout(mp);

  /* Right pane */
  auto rightPane = new QWidget();

  /* Main layout */
  auto layout = new QHBoxLayout();
  layout->addWidget(leftPane);
  layout->addWidget(midPane);
  layout->addWidget(rightPane);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);
}
