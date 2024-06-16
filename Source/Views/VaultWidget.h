#pragma once

#include "BWNetworkService.h"
#include <QWidget>

class QPushButton;
class QLineEdit;
class BWDatabaseEntry;

class VaultWidget : public QWidget {
  Q_OBJECT;
public:
  VaultWidget();

private:
  QWidget* m_leftPane = nullptr;
  QWidget* m_midPane = nullptr;
  QWidget* m_rightPane = nullptr;
  QLineEdit* m_searchBar = nullptr;
  QString m_filter;

  QIcon m_iconHide;
  QIcon m_iconGlobe;
  QIcon m_iconIdentity;
  QIcon m_iconNote;
  QIcon m_iconCreditCard;

  QList<BWDatabaseEntry*> m_shownEntries;

  bool m_editing = false;

  void updateLeftPane();
  void updateMidPane();
  void updateRightPane(size_t idx=-1, bool edit=false);
  void showRightPane(size_t idx=-1);
  void showRightPaneEdit(size_t idx=-1);
  void onEntryClicked(size_t idx=-1);

  QIcon& getIconForEntry(BWDatabaseEntry*);

private slots:
  void onSynced();
  void filter(const QString& text);
};
