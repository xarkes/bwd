#pragma once

#include <QWidget>

class QPushButton;
class BWLineEdit;

struct PasswordEntry
{
  QString name;
  QString notes;
};
static QList<PasswordEntry> pswEntries = {{"One", ""}, {"Two", "This one has a note"}, {"Three", ""}};

class VaultWidget : public QWidget {
  Q_OBJECT;
public:
  VaultWidget();

private:
  QWidget* m_rightPane = nullptr;

  void updateRightPane(size_t idx=-1);

  void onEntryClicked(size_t idx=-1);
};
