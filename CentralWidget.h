#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include "ui_CentralWidget.h"
#include <QSqlRecord>
#include <QJsonObject>
#include <QItemDelegate>
#include <QComboBox>
#include <QFileDialog>
#include <QDateEdit>
#include <QLineEdit>
#include "DataType.h"

class FileDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	FileDelegate(QObject *parent = 0) : QItemDelegate(parent) { }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const  QModelIndex &index)  const
	{
		QFileDialog* pDlg = new QFileDialog(parent);
		return  pDlg;
	}
	void  setEditorData(QWidget *editor, const  QModelIndex &index)  const
	{
		QString text = index.model()->data(index, Qt::EditRole).toString();
		QFileDialog *pDlg = static_cast <QFileDialog*>(editor);
		pDlg->selectFile(text);
	}
	void  setModelData(QWidget *editor, QAbstractItemModel *model,
		const  QModelIndex &index)  const
	{
		QFileDialog *pDlg = static_cast <QFileDialog*>(editor);
		QStringList strList = pDlg->selectedFiles();
		if (strList.isEmpty())
			return;

		QString text = strList[0];
		model->setData(index, text, Qt::EditRole);
	}
	void  updateEditorGeometry(QWidget *editor,
		const  QStyleOptionViewItem &option, const  QModelIndex &index) const
	{
		//editor->setGeometry(option.rect);
	}
};

class DateDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	DateDelegate(QObject *parent = 0) : QItemDelegate(parent) { }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const  QModelIndex &index)  const
	{
		QDateEdit* dateEdit = new QDateEdit(parent);
		dateEdit->setCalendarPopup(true);
		return  dateEdit;
	}
	void  setEditorData(QWidget *editor, const  QModelIndex &index)  const
	{
		QDateEdit *dateEdit = static_cast <QDateEdit*>(editor);
		dateEdit->setDate(QDate::currentDate());
	}
	void  setModelData(QWidget *editor, QAbstractItemModel *model,
		const  QModelIndex &index)  const
	{
		QDateEdit *dateEdit = static_cast <QDateEdit*>(editor);
		QString strDateTime = dateEdit->dateTime().toString("yyyy-MM-dd");
		model->setData(index, strDateTime, Qt::EditRole);
	}
	void  updateEditorGeometry(QWidget *editor,
		const  QStyleOptionViewItem &option, const  QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
	}
};

class IntDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	IntDelegate(QObject *parent = 0) : QItemDelegate(parent) { }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const  QModelIndex &index)  const
	{
		QLineEdit* lineEdit = new QLineEdit(parent);
		lineEdit->setValidator(new QIntValidator);
		
		return  lineEdit;
	}
	void  setEditorData(QWidget *editor, const  QModelIndex &index)  const
	{
		QString text = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit *lineEdit = static_cast <QLineEdit*>(editor);
		lineEdit->setText(text);
	}
	void  setModelData(QWidget *editor, QAbstractItemModel *model,
		const  QModelIndex &index)  const
	{
		QLineEdit *lineEdit = static_cast <QLineEdit*>(editor);
		model->setData(index, lineEdit->text(), Qt::EditRole);
	}
	void  updateEditorGeometry(QWidget *editor,
		const  QStyleOptionViewItem &option, const  QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
	}
};

class DoubleDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	DoubleDelegate(QObject *parent = 0) : QItemDelegate(parent) { }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const  QModelIndex &index)  const
	{
		QLineEdit* lineEdit = new QLineEdit(parent);
		lineEdit->setValidator(new QDoubleValidator);

		return  lineEdit;
	}
	void  setEditorData(QWidget *editor, const  QModelIndex &index)  const
	{
		QString text = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit *lineEdit = static_cast <QLineEdit*>(editor);
		lineEdit->setText(text);
	}
	void  setModelData(QWidget *editor, QAbstractItemModel *model,
		const  QModelIndex &index)  const
	{
		QLineEdit *lineEdit = static_cast <QLineEdit*>(editor);
		model->setData(index, lineEdit->text(), Qt::EditRole);
	}
	void  updateEditorGeometry(QWidget *editor,
		const  QStyleOptionViewItem &option, const  QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
	}
};

class BoolDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	BoolDelegate(QObject *parent = 0) : QItemDelegate(parent) { }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const  QModelIndex &index)  const
	{
		QComboBox* pBox = new QComboBox(parent);
		pBox->addItem("0");
		pBox->addItem("1");

		return  pBox;
	}
	void  setEditorData(QWidget *editor, const  QModelIndex &index)  const
	{
		QString text = index.model()->data(index, Qt::EditRole).toString();
		QComboBox *comboBox = static_cast <QComboBox*>(editor);
		comboBox->setCurrentText(text);
	}
	void  setModelData(QWidget *editor, QAbstractItemModel *model,
		const  QModelIndex &index)  const
	{
		QComboBox *comboBox = static_cast <QComboBox*>(editor);
		model->setData(index, comboBox->currentText(), Qt::EditRole);
	}
	void  updateEditorGeometry(QWidget *editor,
		const  QStyleOptionViewItem &option, const  QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
	}
};

class EnumDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	EnumDelegate(const QString& strFieldType, QObject *parent = 0) : QItemDelegate(parent) 
	{
		m_strFieldType = strFieldType;
	}

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const  QModelIndex &index)  const
	{
		QComboBox* pBox = new QComboBox(parent);
		for (int m = 0; m < DataType::enumTypeCount(m_strFieldType); m++)
		{
			pBox->addItem(DataType::enumTypeDescription(m_strFieldType, m));
		}

		return  pBox;
	}
	void  setEditorData(QWidget *editor, const  QModelIndex &index)  const
	{
		QString text = index.model()->data(index, Qt::EditRole).toString();
		QComboBox *comboBox = static_cast <QComboBox*>(editor);
		comboBox->setCurrentText(text);
	}
	void  setModelData(QWidget *editor, QAbstractItemModel *model,
		const  QModelIndex &index)  const
	{
		QComboBox *comboBox = static_cast <QComboBox*>(editor);
		model->setData(index, comboBox->currentText(), Qt::EditRole);
	}
	void  updateEditorGeometry(QWidget *editor,
		const  QStyleOptionViewItem &option, const  QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
	}

protected:

	QString m_strFieldType;
};

class CentralWidget : public QWidget
{
	Q_OBJECT

public:
	CentralWidget(QWidget *parent = 0);
	~CentralWidget();

	void showBlank();

	void showDBTable(const QString& strCurrentDir, const QJsonObject& currentObj);

	void showSelectDBTable(const QString& strCurrentDir, const QJsonObject& currentObj, const QString& strSQL);

	void changeDatabase();

public slots:

	void slotSubmit();

	void slotRevert();

	void slotDelete();

	void slotBeforeDelete(int);

	void slot3DView();

protected:

	void setDelegate(const QJsonObject& currentObj);

public:
	Ui::CentralWidget ui;

private:

	//记录当前的类型。用于判断哪个是“文件”类型。
	QJsonObject m_currentObject;

	QString m_strCurrentDir;
};

#endif // CENTRALWIDGET_H
