#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QSizePolicy>
#include <qsizepolicy.h>

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

	mainWindow.resize(QSize (768, 550));
	game.resize(QSize(768, 550));
	game.setMinimumSize(game.size());

	mainWindow.setCentralWidget(&game);

	mainWindow.show();
	game.show();

	return application.exec();
}
