#pragma once

#include "BWNetworkService.h"
#include <QWidget>
#include <QScrollArea>

class QPushButton;
class QLineEdit;
class BWDatabaseEntry;

class VaultWidget : public QWidget {
  Q_OBJECT;
public:
  VaultWidget();

private:
  QScrollArea* m_leftPane = nullptr;
  QScrollArea* m_midPane = nullptr;
  QWidget* m_rightPane = nullptr;
  QLineEdit* m_searchBar = nullptr;

  QIcon m_iconHide;
  QIcon m_iconGlobe;
  QIcon m_iconIdentity;
  QIcon m_iconNote;
  QIcon m_iconCreditCard;

  QList<BWDatabaseEntry*> m_shownEntries;

  bool m_editing = false;

  void updateLeftPane();
  void updateMidPane();
  void updateRightPane(qsizetype idx=-1, bool edit=false);
  void showRightPane(qsizetype idx=-1);
  void showRightPaneEdit(qsizetype idx=-1);
  void onEntryClicked(qsizetype idx=-1);

  QIcon& getIconForEntry(BWDatabaseEntry*);
  void filter(const QString& text, QString folder="");

private slots:
  void onSynced();
};
