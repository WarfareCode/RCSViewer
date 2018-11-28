#include "ValueInputDlg.h"
#include "DataType.h"

ValueInputDlg::ValueInputDlg(QString strFieldType, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("ÊýÖµ"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	if (strFieldType.compare(FieldType_Int_Des) == 0)
	{
		ui.lineEdit->setValidator(new QIntValidator);
	}
	else if (strFieldType.compare(FieldType_Double_Des) == 0)
	{
		ui.lineEdit->setValidator(new QDoubleValidator);
	}
	else if (strFieldType.compare(FieldType_Bool_Des) == 0)
	{
		ui.lineEdit->setValidator(new QIntValidator(0, 1));
	}

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

ValueInputDlg::~ValueInputDlg()
{

}

QString ValueInputDlg::editText()
{
	return ui.lineEdit->text();
}

void ValueInputDlg::accept()
{
	QDialog::accept();
}

void ValueInputDlg::reject()
{
	QDialog::reject();
}