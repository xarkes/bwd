#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QShortcut>

#include "VaultWidget.h"
#include "BWNetworkService.h"
#include "Components/BWEntry.h"
#include "Components/BWLineEdit.h"
#include "Components/BWField.h"
#include "Components/BWCategory.h"
#include "Components/BWCategoryEntry.h"

VaultWidget::VaultWidget()
{
  QWidget* topbar = new QWidget();
  topbar->setLayout(new QHBoxLayout());
  topbar->setStyleSheet("background-color: blue;");

  m_searchBar = new QLineEdit();
  m_searchBar->setPlaceholderText("Search vault");
  m_searchBar->setStyleSheet("color: white; background-color: rgb(0, 0, 178)");
  m_searchBar->setTextMargins(5, 5, 5, 5);
  m_searchBar->setMaximumWidth(width() / 2);
  topbar->layout()->addWidget(m_searchBar);

  m_iconHide = QIcon(":/Images/hide.png");
  m_iconGlobe = QIcon(":/Images/globe.png");

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

  /* Shortcuts */
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this, [this](){
    m_searchBar->setFocus();
  });

  /* Handlers */
  connect(Net(), &BWNetworkService::synced, this, &VaultWidget::onSynced);
  connect(m_searchBar, &QLineEdit::textChanged, this, &VaultWidget::filter);
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
  for (auto& entry : Net()->db().entries) {
    QString name = entry.name.decrypt();
    QString uri = entry.uri.decrypt();
    if (m_filter.length() && !(name.toLower().contains(m_filter) || uri.toLower().contains(m_filter))) {
      continue;
    }
    QString subtext = entry.username.decrypt();
    auto w = new BWEntry(name, subtext);
    w->setIcon(m_iconGlobe);
    connect(w, &QPushButton::released, this, [this, idx]{ onEntryClicked(idx); });
    mp->addWidget(w);
    mpgroup->addButton(w);
    idx++;
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
  auto title = new QLabel(entry ? entry->name.decrypt() : "Entry name");
  auto font = QFont(title->font());
  font.setBold(true);
  title->setFont(font);
  r0l->addWidget(title);
  rl->addWidget(r0);

  auto r1 = new QWidget();
  r1->setLayout(new QVBoxLayout());
  if (entry && entry->username.decrypt().length()) {
    r1->layout()->setSpacing(0);
    auto user = new BWField("username");
    user->setText(entry ? entry->username.decrypt() : "");
    r1->layout()->addWidget(user);
  }
  if (entry && entry->password.decrypt().length()) {
    auto password = new BWFieldConfidential("password");
    password->setText(entry ? entry->password.decrypt() : "");
    auto passwordRevealButton = new QPushButton();
    passwordRevealButton->setIcon(m_iconHide);
    passwordRevealButton->setMaximumWidth(20);
    auto pswRow = new QWidget();
    pswRow->setLayout(new QHBoxLayout());
    pswRow->layout()->setContentsMargins(0, 0, 0, 0);
    pswRow->layout()->addWidget(password);
    pswRow->layout()->addWidget(passwordRevealButton);
    connect(passwordRevealButton, &QPushButton::released, [password](){
      password->toggle();
    });
    r1->layout()->addWidget(pswRow);
  }
  rl->addWidget(r1);
  rl->setAlignment(Qt::AlignTop);

  if (entry && entry->notes.decrypt().length()) {
    auto r2 = new QWidget();
    auto r2l = new QVBoxLayout();
    r2->setLayout(r2l);
    auto w = new BWField("notes");
    w->setText(entry->notes.decrypt());
    r2l->addWidget(w);
    rl->addWidget(r2);
  }

  if (entry && entry->uri.decrypt().length()) {
    auto row = new QWidget();
    auto r2l = new QVBoxLayout();
    row->setLayout(r2l);
    auto w = new BWField("uri");
    w->setText(entry->uri.decrypt());
    r2l->addWidget(w);
    rl->addWidget(row);
  }
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

void VaultWidget::filter(const QString& text)
{
  m_filter = text.toLower();
  updateMidPane();
}
