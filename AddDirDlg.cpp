#include "AddDirDlg.h"

AddDirDlg::AddDirDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("Ìí¼ÓÄ¿Â¼"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AddDirDlg::~AddDirDlg()
{

}

void AddDirDlg::accept()
{
	QString strDir = ui.lineEdit->text();
	if (strDir.isNull() || strDir.isEmpty())
		return;

	if (strDir.contains("/") || strDir.contains("\\"))
		return;

	QDialog::accept();
}

void AddDirDlg::reject()
{
	QDialog::reject();
}

QString AddDirDlg::strDir()
{
	return ui.lineEdit->text();
}