#include "OpenProjectDlg.h"
#include <QCoreApplication>

OpenProjectDlg::OpenProjectDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/open.png"));

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(accept()));
}

OpenProjectDlg::~OpenProjectDlg()
{

}
