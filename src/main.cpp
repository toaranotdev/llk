#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>

#include "game.h"

int main (int argc, char* argv[]) {
	QApplication application (argc, argv);
	QMainWindow mainWindow;

	Game game(&mainWindow);

	QMenuBar* menuBar = mainWindow.menuBar();
	QMenu* gameMenu = menuBar->addMenu("Game");

	gameMenu->addAction(game.newGameAct);
	gameMenu->addAction(game.exitGameAct);
	gameMenu->addSeparator();
	gameMenu->addAction(game.closeWindowAct);

	#ifdef Q_OS_WIN
	int menuBarHeight = menuBar->size().height() - 9;
	#elif def Q_OS_LINUX
	int menuBarHeight = menuBar->size().height() - 3;
	#endif

	mainWindow.resize(QSize (768, 550 + menuBarHeight));
	game.resize(QSize(768, 550));

	mainWindow.setCentralWidget(&game);
	mainWindow.show();
	game.show();

	return application.exec();
}
