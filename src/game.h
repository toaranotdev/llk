#pragma once

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QRect>
#include <QBrush>
#include <QPixmap>
#include <QSize>
#include <QMouseEvent>
#include <QPen>
#include <QAction>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QMessageBox>
#include <QBitmap>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSoundEffect>

#include <sstream>
#include <chrono>

#include "matrix.h"

// TODO: Flipping the coordinate upside down because it's getting asodjsapidjsaiodsadiosajdiosajdiosadjsaidjsaiodjsaiojdisajdisajd
// ok thanks

class Game : public QWidget {
	Q_OBJECT

	public:
		Game(QWidget* parent = nullptr);
		~Game();
		// override stuff
		void paintEvent(QPaintEvent*);
		void mousePressEvent(QMouseEvent* event);

		// this timer is responsible for calling update() each frame
		QTimer refreshTimer;
		// this timer is responsible for decreasing the remaining time the player has left
		QTimer gameTimer;
		// game data object 
		Matrix matrix;

		// resources
		QPixmap foreground;
		QPixmap background;
		QPixmap selection;
		QPixmap cross;
		QPixmap font;
		QPixmap backgroundImage;

		QSoundEffect chooseSound;
		QSoundEffect cancelSound;
		QSoundEffect connectSound;
		QSoundEffect shuffleSound;
		QSoundEffect bellSound;
		QSoundEffect errorSound;

		QMediaPlayer musicPlayer;
		QMediaPlaylist musicList;

		QAction* newGameAct;
		QAction* exitGameAct;
		QAction* closeWindowAct;

		QMessageBox messageBox;
	private:
		// initializers
		void InitializeTextures();
		void InitializeActions();
		void InitializeSoundEffects();
		void InitializeMusics();
		// updates
		void UpdateMousePosition();
		// getters
		int GetIndexUnderMouse();
		// checks
		bool IsMouseInBounds();
		// other stuff i guess
		void DrawNumber(QPainter* painter, std::string numberString, QPoint location, bool reversed);

		void GameOver();
		void AdvanceLevel();
	
		void GenerateSeed();
		void SelectBackgroundColor();
		void SelectBackgroundImage();

		void StartConnection(int startIndex, int targetIndex);

		// render functions
		void Render(QPainter* painter);
		void RenderBackground(QPainter* painter);
		void RenderTimer(QPainter* painter);
		void RenderLives(QPainter* painter);
		void RenderScore(QPainter* painter);
		void RenderTiles(QPainter* painter);
		void RenderSelection(QPainter* painter);
		void RenderConnection(QPainter* painter);

		QPoint mousePosition;

		QSize tileSize;
		QSize positionOffset;

		std::map<int, QRect> foregroundTextureMap;
		std::map<int, QRect> backgroundTextureMap;
		std::map<int, QRect> fontTextureMap;

		std::vector<int> activeConnectionPath;
		int currentBgColorIndex;

		int remainingSeconds;
		int remainingLives;
		int score;

		int backgroundColorIndex;
		int backgroundImageIndex;

		bool isConnecting;
		int selectedIndex;

		int currentScene;
		int seed;

		enum Scenes {
			TITLE,
			IN_GAME
		};

	private slots:
		void ClearConnection();

		void CreateNewGame();
		void ExitGame();
		void CloseWindow();
};
