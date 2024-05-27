#include <QHBoxLayout>
#include <QLabel>
#include <qboxlayout.h>
#include <QTreeWidget>

#include "VaultWidget.h"
#include "BWEntry.h"
#include "BWLineEdit.h"

#include <QPushButton>


VaultWidget::VaultWidget()
{
  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  auto topLayout = new QHBoxLayout();
  auto searchBar = new BWLineEdit("Search vault");
  searchBar->setMaximumWidth(width() / 2);
  topLayout->addWidget(searchBar);
  topbar->setLayout(topLayout);

  /* Left pane */
  auto lp = new QVBoxLayout();
  lp->setAlignment(Qt::AlignmentFlag::AlignTop);
  lp->setSpacing(0);
  lp->setContentsMargins(0, 0, 0, 0);
  auto treeFav = new QTreeWidget();
  treeFav->setColumnCount(1);
  QList<QTreeWidgetItem*> favItems;
  favItems.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(QString("All items"))));
  favItems.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(QString("Favorites"))));
  favItems.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(QString("Trash"))));
  treeFav->addTopLevelItems(favItems);

  auto treeFolders = new QTreeWidget();
  treeFolders->setColumnCount(1);
  QList<QTreeWidgetItem*> folderItems;
  folderItems.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(QString("Cat0"))));
  folderItems.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(QString("Cat1"))));
  treeFolders->addTopLevelItems(folderItems);

  lp->addWidget(treeFav);
  lp->addWidget(treeFolders);
  auto leftPane = new QWidget();
  leftPane->setLayout(lp);
  leftPane->setMinimumWidth(200);
  leftPane->setMaximumWidth(300);

  /* Middle pane */
  auto mp = new QVBoxLayout();
  mp->setAlignment(Qt::AlignmentFlag::AlignTop);
  mp->setSpacing(0);
  mp->setContentsMargins(0, 0, 0, 0);
  auto idx = 0;
  for (auto entry : pswEntries) {
    auto w = new QPushButton(entry.name);
    connect(w, &QPushButton::released, this, [this, idx]{ onEntryClicked(idx); });
    mp->addWidget(w);
    idx++;
  }
  auto midPane = new QWidget();
  midPane->setStyleSheet("background-color: white;");
  midPane->setLayout(mp);
  midPane->setMinimumWidth(200);
  midPane->setMaximumWidth(300);

  /* Right pane */
  updateRightPane();

  /* Main layout */
  auto layout = new QHBoxLayout();
  layout->addWidget(leftPane);
  layout->addWidget(midPane);
  layout->addWidget(m_rightPane);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);
}

void VaultWidget::updateRightPane(size_t idx)
{
  if (!m_rightPane) {
    m_rightPane = new QWidget();
    m_rightPane->setMinimumWidth(400);
    m_rightPane->setLayout(new QVBoxLayout());
  }
  auto rl = m_rightPane->layout();
  while (rl->count()) {
    auto w = rl->takeAt(0);
    w->widget()->setParent(nullptr);
    // XXX: Does this leak memory?
  }

  PasswordEntry* entry = nullptr;
  if (idx != -1 && idx < pswEntries.length()) {
    entry = &pswEntries[idx];
  }

  auto r0l = new QHBoxLayout();
  r0l->setAlignment(Qt::AlignLeft);
  auto r0 = new QWidget();
  r0->setLayout(r0l);
  r0l->addWidget(new QLabel("Icon"));
  r0l->addWidget(new QLabel(entry ? entry->name : "Entry name"));
  auto r1 = new QWidget();
  auto r1l = new QVBoxLayout();
  r1l->setSpacing(0);
  r1->setLayout(r1l);
  auto w = new BWLineEdit("username");
  w->setDisabled(true);
  r1l->addWidget(w);
  w = new BWLineEdit("password");
  w->setDisabled(true);
  r1l->addWidget(w);
  w = new BWLineEdit("otp");
  w->setDisabled(true);
  r1l->addWidget(w);
  rl->setAlignment(Qt::AlignTop);

  rl->addWidget(r0);
  rl->addWidget(r1);
  if (entry && entry->notes.length()) {
    auto r2 = new QWidget();
    auto r2l = new QVBoxLayout();
    r2->setLayout(r2l);
    auto w = new BWLineEdit("notes");
    w->setText(entry->notes);
    w->setDisabled(true);
    r2l->addWidget(w);
    rl->addWidget(r2);
  }
}

void VaultWidget::onEntryClicked(size_t idx)
{
  qDebug() << "Clicked";
  updateRightPane(idx);
}
