#pragma once

#include <QListWidget>

class BWListItem
{
public:
  BWListItem(QString label);
  QString m_label;
};

class BWListWidget : public QWidget
{
public:
  BWListWidget(QString label, QWidget* parent=nullptr);
  void addItem(BWListItem* item) { m_items.append(item); }
  void paintEvent(QPaintEvent* event) override;

private:
  bool m_expanded = true;
  QString m_label;
  QList<BWListItem*> m_items;
};
