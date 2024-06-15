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

  QList<BWDatabaseEntry*> m_shownEntries;

  void updateLeftPane();
  void updateMidPane();
  void updateRightPane(size_t idx=-1);
  void onEntryClicked(size_t idx=-1);

private slots:
  void onSynced();
  void filter(const QString& text);
};
