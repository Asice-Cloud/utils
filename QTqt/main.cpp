#include <QApplication>
#include <QWidget>
#include <QDebug>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QWidget window;
	window.setWindowTitle("Hello Qt");
	window.resize(400, 300);
	window.show();

	qDebug() << "Hello World";
	return a.exec();
}