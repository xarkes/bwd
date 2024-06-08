#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QButtonGroup>

#include "VaultWidget.h"
#include "BWEntry.h"
#include "BWLineEdit.h"
#include "BWCategory.h"
#include "BWCategoryEntry.h"
#include "BWNetworkService.h"

VaultWidget::VaultWidget()
{
  QWidget* topbar = new QWidget();
  topbar->setStyleSheet("background-color: blue;");
  auto topLayout = new QHBoxLayout();
  auto searchBar = new QLineEdit();
  searchBar->setPlaceholderText("Search vault");
  searchBar->setStyleSheet("color: white; background-color: rgb(0, 0, 178)");
  searchBar->setTextMargins(5, 5, 5, 5);
  searchBar->setMaximumWidth(width() / 2);
  topLayout->addWidget(searchBar);
  topbar->setLayout(topLayout);

  /* Show panes */
  onSynced();

  /* Main layout */
  auto layout = new QHBoxLayout();
  layout->addWidget(m_leftPane);
  layout->addWidget(m_midPane);
  layout->addWidget(m_rightPane);

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(topbar);
  mainLayout->addLayout(layout);

  connect(Net(), &BWNetworkService::synced, this, &VaultWidget::onSynced);
}

void VaultWidget::updateLeftPane()
{
  if (!m_leftPane) {
    m_leftPane = new QWidget();
    m_leftPane->setStyleSheet("background-color: rgb(32, 47, 63)");
    m_leftPane->setMinimumWidth(200);
    m_leftPane->setMaximumWidth(300);
    m_leftPane->setLayout(new QVBoxLayout());
  }
  auto ll = m_leftPane->layout();
  while (ll->count()) {
    auto w = ll->takeAt(0);
    w->widget()->setParent(nullptr);
    // XXX: Does this leak memory?
    // XXX: shouldnt we call deleteLater
  }
  auto lpgroup = new QButtonGroup(ll);
  for (auto s : { "All items", "Favorites", "Whatever" }) {
    auto w = new BWCategoryEntry(s);
    ll->addWidget(w);
    lpgroup->addButton(w);
  }
  ll->addWidget(new BWCategory("Test"));
}

void VaultWidget::updateMidPane()
{
  if (!m_midPane) {
    m_midPane = new QWidget();
    m_midPane->setStyleSheet("background-color: white;");
    m_midPane->setMinimumWidth(200);
    m_midPane->setMaximumWidth(300);
    m_midPane->setLayout(new QVBoxLayout());
  }
  auto mp = m_midPane->layout();
  while (mp->count()) {
    auto w = mp->takeAt(0);
    w->widget()->setParent(nullptr);
    // XXX: Does this leak memory?
    // XXX: shouldnt we call deleteLater
  }
  mp->setAlignment(Qt::AlignmentFlag::AlignTop);
  mp->setSpacing(0);
  mp->setContentsMargins(0, 0, 0, 0);
  auto idx = 0;
  auto mpgroup = new QButtonGroup(mp);
  mpgroup->setExclusive(true);
  // TODO: Decryption should be delegated to another thread, we don't want to block it
  for (auto entry : Net()->db().entries) {
    // auto w = new BWEntry(entry.name, entry.notes.length() ? entry.notes : "");

    QString name = entry.name.decryptToBytes(Net()->m_key, "");
    // QString notes = entry.notes ?

    auto w = new BWEntry(name, "TODO");
    connect(w, &QPushButton::released, this, [this, idx]{ onEntryClicked(idx); });
    mp->addWidget(w);
    mpgroup->addButton(w);
    idx++;

    // XXX
    if (idx > 5) {
      break;
    }
  }
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

  BWDatabaseEntry* entry = nullptr;
  if (idx != -1 && idx < Net()->db().entries.length()) {
    entry = &Net()->db().entries[idx];
  }

  auto r0l = new QHBoxLayout();
  r0l->setAlignment(Qt::AlignLeft);
  auto r0 = new QWidget();
  r0->setLayout(r0l);
  r0l->addWidget(new QLabel("Icon"));
  r0l->addWidget(new QLabel(entry ? entry->name.decryptToBytes(Net()->m_key, "") : "Entry name"));
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
  // TODO
  // if (entry && entry->notes.length()) {
  //   auto r2 = new QWidget();
  //   auto r2l = new QVBoxLayout();
  //   r2->setLayout(r2l);
  //   auto w = new BWLineEdit("notes");
  //   w->setText(entry->notes);
  //   w->setDisabled(true);
  //   r2l->addWidget(w);
  //   rl->addWidget(r2);
  // }
}

void VaultWidget::onEntryClicked(size_t idx)
{
  updateRightPane(idx);
}

void VaultWidget::onSynced()
{
  updateLeftPane();
  updateMidPane();
  updateRightPane();
}
