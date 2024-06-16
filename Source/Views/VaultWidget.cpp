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
  for (auto entry : m_shownEntries) {
    QString name = entry->name.decrypt();
    QString uri = entry->uri.decrypt();
    QString subtext = entry->username.decrypt();
    auto w = new BWEntry(name, subtext);
    w->setIcon(m_iconGlobe);
    connect(w, &QPushButton::released, this, [this, idx]{ onEntryClicked(idx); });
    mp->addWidget(w);
    mpgroup->addButton(w);
    idx++;
  }
}

void VaultWidget::updateRightPane(size_t idx, bool edit)
{
  if (!m_rightPane) {
    m_rightPane = new QWidget();
    m_rightPane->setMinimumWidth(400);
    m_rightPane->setLayout(new QVBoxLayout());
  }
  auto rightLayout = m_rightPane->layout();
  while (rightLayout->count()) {
    auto w = rightLayout->takeAt(0);
    w->widget()->setParent(nullptr);
    // XXX: Does this leak memory?
  }

  if (edit) {
    showRightPaneEdit(idx);
  } else {
    showRightPane(idx);
  }
}

void VaultWidget::showRightPaneEdit(size_t idx) {
  auto title = new BWLineEdit("title");
  auto username = new BWLineEdit("username");
  auto password = new BWLineEdit("password");
  auto uri = new BWLineEdit("uri");

  BWDatabaseEntry* entry = nullptr;
  if (idx != -1 && idx < m_shownEntries.length()) {
    entry = m_shownEntries[idx];
  }

  auto row0 = new QWidget();
  row0->setLayout(new QHBoxLayout());
  row0->layout()->setAlignment(Qt::AlignRight);
  row0->layout()->setContentsMargins(0, 0, 0, 0);
  auto editButton = new QPushButton("Save");
  connect(editButton, &QPushButton::released, [this, idx, entry, title, username, password, uri](){
    entry->name.setClear(title->text());
    entry->username.setClear(username->text());
    entry->password.setClear(password->text());
    entry->uri.setClear(uri->text());
    Net()->editEntry(entry);
    updateRightPane(idx, false);
  });
  row0->layout()->addWidget(editButton);

  if (entry) {
    title->setText(entry->name.decrypt());
    username->setText(entry->username.decrypt());
    password->setText(entry->password.decrypt());
    uri->setText(entry->uri.decrypt());
  }

  m_rightPane->layout()->addWidget(row0);
  m_rightPane->layout()->addWidget(title);
  m_rightPane->layout()->addWidget(username);
  m_rightPane->layout()->addWidget(password);
  m_rightPane->layout()->addWidget(uri);
}

void VaultWidget::showRightPane(size_t idx) {
  BWDatabaseEntry* entry = nullptr;
  if (idx != -1 && idx < m_shownEntries.length()) {
    entry = m_shownEntries[idx];
  }
  if (!entry) {
    return;
  }

  auto row0 = new QWidget();
  row0->setLayout(new QHBoxLayout());
  row0->layout()->setAlignment(Qt::AlignRight);
  row0->layout()->setContentsMargins(0, 0, 0, 0);
  auto editButton = new QPushButton("Edit");
  editButton->setText("Edit");
  connect(editButton, &QPushButton::released, [this, idx](){
    updateRightPane(idx, true);
  });
  row0->layout()->addWidget(editButton);
  m_rightPane->layout()->addWidget(row0);

  auto row1 = new QWidget();
  row1->setLayout(new QHBoxLayout());
  row1->layout()->setAlignment(Qt::AlignLeft);
  auto icon = new QLabel();
  icon->setPixmap(m_iconGlobe.pixmap(QSize{50, 50}));
  row1->layout()->addWidget(icon);
  auto title = new QLabel(entry->name.decrypt());
  auto font = QFont(title->font());
  font.setBold(true);
  font.setPointSize(font.pointSize() + 10);
  title->setFont(font);
  row1->layout()->addWidget(title);
  m_rightPane->layout()->addWidget(row1);

  auto row2 = new QWidget();
  row2->setLayout(new QVBoxLayout());
  if (entry->username.decrypt().length()) {
    row2->layout()->setSpacing(0);
    auto user = new BWField("username");
    user->setText(entry ? entry->username.decrypt() : "");
    row2->layout()->addWidget(user);
  }
  if (entry->password.decrypt().length()) {
    auto password = new BWFieldConfidential("password");
    password->setText(entry->password.decrypt());
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
    row2->layout()->addWidget(pswRow);
  }
  m_rightPane->layout()->addWidget(row2);
  m_rightPane->layout()->setAlignment(Qt::AlignTop);

  if (entry->notes.decrypt().length()) {
    auto row = new QWidget();
    row->setLayout(new QVBoxLayout());
    auto w = new BWField("notes");
    w->setText(entry->notes.decrypt());
    row->layout()->addWidget(w);
    m_rightPane->layout()->addWidget(row);
  }

  if (entry->uri.decrypt().length()) {
    auto row = new QWidget();
    row->setLayout(new QVBoxLayout());
    auto w = new BWField("uri");
    w->setText(entry->uri.decrypt());
    row->layout()->addWidget(w);
    m_rightPane->layout()->addWidget(row);
  }
}

void VaultWidget::onEntryClicked(size_t idx)
{
  updateRightPane(idx);
}

void VaultWidget::onSynced()
{
  m_shownEntries.clear();
  for (auto& entry : Net()->db().entries) {
    m_shownEntries.append(&entry);
  }
  updateLeftPane();
  updateMidPane();
  updateRightPane();
}

void VaultWidget::filter(const QString& text)
{
  m_filter = text.toLower();
  m_shownEntries.clear();
  for (auto& entry : Net()->db().entries) {
    QString name = entry.name.decrypt();
    QString uri = entry.uri.decrypt();
    if (m_filter.length() && !(name.toLower().contains(m_filter) || uri.toLower().contains(m_filter))) {
      continue;
    }
    m_shownEntries.append(&entry);
  }
  updateMidPane();
}
