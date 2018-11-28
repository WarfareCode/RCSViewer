#include "NewProjectDlg.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QFileDialog>
#include "Document.h"

NewProjectDlg::NewProjectDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/new.png"));

	ui.lineEdit_ProjectDir->setEnabled(false);

	connect(ui.pushButton_OK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui.pushButton_Dir, SIGNAL(clicked()), this, SLOT(slotSetPrjDir()));
}

NewProjectDlg::~NewProjectDlg()
{

}

QString NewProjectDlg::projectDir()
{
	return ui.lineEdit_ProjectDir->text();
}

QString NewProjectDlg::projectName()
{
	return ui.lineEdit_ProjectName->text() + ".rcsprj";
}

void NewProjectDlg::slotSetPrjDir()
{
	QString strDir = QFileDialog::getExistingDirectory();
	if (strDir.isEmpty() || strDir.isNull())
		return;

	ui.lineEdit_ProjectDir->setText(strDir);
}

void NewProjectDlg::accept()
{
	if (ui.lineEdit_ProjectName->text().isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"),
			QStringLiteral("请输入工程名称"));
		return;
	}

	if (ui.lineEdit_ProjectDir->text().isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"),
			QStringLiteral("请选择工程路径"));
		return;
	}

	QDialog::accept();
}

void NewProjectDlg::reject()
{
	QDialog::reject();
}
