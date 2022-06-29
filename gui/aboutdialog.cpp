#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include "aboutdialog.h"
#include "common.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	QTabWidget *tabWidget = new QTabWidget(this);

	this->setWindowTitle("About looqs");

	QHBoxLayout *closeLayout = new QHBoxLayout();
	QPushButton *closeButton = new QPushButton(this);
	closeButton->setText("Close");
	closeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

	closeLayout->setMargin(10);
	closeLayout->addStretch(10);
	closeLayout->addWidget(closeButton);

	layout->addWidget(tabWidget);
	layout->addLayout(closeLayout);
	QFrame *frame = new QFrame(this);
	QVBoxLayout *aboutLayout = new QVBoxLayout(frame);
	frame->setLayout(aboutLayout);

	QLabel *aboutLooqs = new QLabel(this);
	QString html = "<h2>looqs</h2>";
	html += "Full-text search with previews for your files<br><br>";
	html += "Copyright (c) 2018-2022: Albert Schwarzkopf<br><br>";
	html += QString("Version: %1<br><br>").arg(Common::versionText());
	html += "Contact: looqs at quitesimple dot org<br><br>";
	html += "Website: <a href=\"https://quitesimple.org\">https://quitesimple.org</a><br><br>";
	html += "Source code: <a "
			"href=\"https://github.com/quitesimpleorg/looqs\">https://github.com/quitesimpleorg/looqs</a><br><br>";
	html += "License: GPLv3<br><br>";
	html += "looqs is open source and free of charge in the hope it will be useful. The author(s) do not "
			"give any warranty. In the unlikely event of any damage, the author(s) cannot be held responsible. "
			"You are using looqs at your own risk<br><br>";
	html += "looqs uses third-party libraries, please see the corresponding tab.";
	aboutLooqs->setText(html);
	aboutLooqs->setTextFormat(Qt::RichText);

	QLabel *logo = new QLabel(this);
	QImage image(QString(":/looqs.svg"));
	logo->setPixmap(QPixmap::fromImage(image));

	aboutLayout->addWidget(logo);
	aboutLayout->addWidget(aboutLooqs);

	tabWidget->addTab(frame, "About");

	QFile license(QString(":/LICENSE"));
	license.open(QIODevice::ReadOnly);
	QString licenseText = license.readAll();

	QFile thirdPartyLicense(QString(":./LICENSE-3RD-PARTY"));
	thirdPartyLicense.open(QIODevice::ReadOnly);
	QString thirdPartyLicenseInfo = thirdPartyLicense.readAll();

	QTextBrowser *licenseTextBrowser = new QTextBrowser(this);
	licenseTextBrowser->setText(licenseText);

	QTextBrowser *thirdPartyLicenceTextBrowser = new QTextBrowser(this);
	thirdPartyLicenceTextBrowser->setText(thirdPartyLicenseInfo);

	tabWidget->addTab(licenseTextBrowser, "License");
	tabWidget->addTab(thirdPartyLicenceTextBrowser, "Third-party libraries");
}
