#include <QHBoxLayout>
#include <QLabel>

#include "VaultWidget.h"
#include "BWButton.h"
#include "BWEntry.h"

VaultWidget::VaultWidget()
{
  QList<QString> pswEntries = {"One", "Two", "Three"};

  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  topbar->setFixedHeight(50);

  /* Left pane */
  auto lp = new QVBoxLayout();
  lp->setAlignment(Qt::AlignmentFlag::AlignTop);
  lp->setSpacing(0);
  lp->setContentsMargins(0, 0, 0, 0);
  lp->addWidget(new BWButton("All items"));
  lp->addWidget(new BWButton("Favorites"));
  lp->addWidget(new BWButton("Trash"));
  auto leftPane = new QWidget();
  leftPane->setLayout(lp);
  leftPane->setMinimumWidth(200);
  leftPane->setMaximumWidth(300);

  /* Middle pane */
  auto mp = new QVBoxLayout();
  mp->setAlignment(Qt::AlignmentFlag::AlignTop);
  mp->setSpacing(0);
  mp->setContentsMargins(0, 0, 0, 0);
  for (auto entry : pswEntries) {
    auto w = new BWEntry(entry);
    mp->addWidget(w);
  }
  auto midPane = new QWidget();
  midPane->setStyleSheet("background-color: white;");
  midPane->setLayout(mp);
  midPane->setMinimumWidth(200);
  midPane->setMaximumWidth(300);

  /* Right pane */
  auto rightPane = new QWidget();
  rightPane->setMinimumWidth(400);

  /* Main layout */
  auto layout = new QHBoxLayout();
  layout->addWidget(leftPane);
  layout->addWidget(midPane);
  layout->addWidget(rightPane);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);
}
