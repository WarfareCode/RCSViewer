#include "OBEnumInputDlg.h"
#include "DataType.h"

OBEnumInputDlg::OBEnumInputDlg(QString strFieldType, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("枚举类型输入"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	for (int m = 0; m < DataType::enumTypeCount(strFieldType); m++)
	{
		ui.comboBox->addItem(DataType::enumTypeDescription(strFieldType, m));
	}

	QObject::connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

OBEnumInputDlg::~OBEnumInputDlg()
{

}

QString OBEnumInputDlg::text()
{
	return ui.comboBox->currentText();
}

void OBEnumInputDlg::accept()
{
	QDialog::accept();
}

void OBEnumInputDlg::reject()
{
	QDialog::reject();
}