#ifndef FILETREEVIEW_H
#define FILETREEVIEW_H

#include <QtWidgets/QTreeView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileSystemModel>

class CentralWidget;
class FileTreeView : public QTreeView
{
	Q_OBJECT

public:

	FileTreeView(QWidget *parent);
	~FileTreeView();

	void setBuddy(CentralWidget* pWidget);

	void setWorkSpace(QString strWorkSpace);

	QSize sizeHint() const
	{
		return QSize(700, 800); /* 在这里定义dock的初始大小 */
	}

protected slots:

	void slotAddDir();
	void slotAddDirOnEmpty();
	void slotDeleteDir();
	void slotAddClass();
	void slotAddClassUseTemplate();
	void slotDeleteClass();

	void slotAddRecord();

	void slotAddRecords();

	void slotSearch();

	void slotEditClass();

protected:

	void contextMenuEvent(QContextMenuEvent *event) override;

	void mouseDoubleClickEvent(QMouseEvent *event) override;

private:

	QFileSystemModel* m_pModel;

	QMenu* m_pMenuOnDir;
	QMenu* m_pMenuOnClass;
	QMenu* m_pMenuOnEmpty;

	QModelIndex m_currentIndex;
	
	CentralWidget* m_pBuddy;
};

#endif // FILETREEVIEW_H
