#include "SetPathDlg.h"
#include <QtWidgets/QFileDialog>

SetPathDlg::SetPathDlg(QString strPath, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("ÉèÖÃ±£´æÂ·¾¶"));

	ui.lineEdit->setText(strPath);
}

SetPathDlg::~SetPathDlg()
{

}

void SetPathDlg::slotSetPath()
{
	QString strFileName = QFileDialog::getSaveFileName(this, tr("Save File")
		, ui.lineEdit->text()
		, tr("Videos (*.mkv *.avi *.mp4)"));

	if (strFileName.isEmpty())
		return;

	ui.lineEdit->setText(strFileName);
}

QString SetPathDlg::path()
{
	return ui.lineEdit->text();
}