#include "DateInputDlg.h"

DateInputDlg::DateInputDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("ÈÕÆÚ"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui.dateEdit->setDate(QDate::currentDate());

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

DateInputDlg::~DateInputDlg()
{

}

QDate DateInputDlg::date()
{
	return ui.dateEdit->date();
}

void DateInputDlg::accept()
{
	QDialog::accept();
}

void DateInputDlg::reject()
{
	QDialog::reject();
}