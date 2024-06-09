#pragma once

#include <QWidget>

class QPushButton;
class BWLineEdit;

class VaultWidget : public QWidget {
  Q_OBJECT;
public:
  VaultWidget();

private:
  QWidget* m_leftPane = nullptr;
  QWidget* m_midPane = nullptr;
  QWidget* m_rightPane = nullptr;

  void updateLeftPane();
  void updateMidPane();
  void updateRightPane(size_t idx=-1);
  void onEntryClicked(size_t idx=-1);

private slots:
  void onSynced();
};
