#pragma once

#include <QWidget>

class QPushButton;
class BWLineEdit;

class VaultUnlockWidget : public QWidget {
  Q_OBJECT;
public:
  VaultUnlockWidget();

signals:
  void back();
  void unlocked();

private:
  void setLoading(bool);

  BWLineEdit* m_inputPassword;
  QPushButton* m_buttonBack;
  QPushButton* m_buttonUnlock;
};
