#include "game.h"

Game::Game(QWidget* parent /* = nullptr */ ) : QWidget(parent), messageBox(this) {
	this->selectedIndex = -1;
	this->score = 0;
	this->remainingLives = 0;
	// technically the background tiles and the foreground isn't the same size
	// but using the background size (46x56) will cause qt to fuck itself up
	// when rendering so i have no idea lol
	this->tileSize = QSize(40, 50);
	// measured this from the original game xd
	this->positionOffset = QSize(61, 56);
	
	this->currentScene = Scenes::TITLE;

	this->InitializeTextures();
	this->InitializeActions();
	this->InitializeSoundEffects();
	this->InitializeMusics();

	this->refreshTimer.setInterval(0);
	QObject::connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(update()));
	this->refreshTimer.start();

	gameTimer.setInterval(3000);
	QObject::connect(&gameTimer, &QTimer::timeout, [this]()->void { if (this->remainingSeconds) this->remainingSeconds --; });
}

Game::~Game() {
	delete this->newGameAct;
	delete this->exitGameAct;
	delete this->closeWindowAct;
}

/*
 * 	INITIALIZERS
 */

// load a bunch of textures
void Game::InitializeTextures() {
	this->foreground.load(":/foreground.png");
	this->background.load(":/background.png");
	this->selection.load(":/selection.png");
	this->cross.load(":/cross.png");
	this->font.load(":/font.png");
	this->backgroundImage.load(":/titlescreen.png");

	// foreground texture coordinate
	for (int i = 1; i < 36; i ++) {
		// subtract from 1 because annoyingly the texture starts at 0,0 and not 1
		int x = ((i < 18) ? i - 1 : i - 18) 	* this->tileSize.width();
		int y = ((i < 18) ? 0 	: 1) 		* this->tileSize.height();

		this->foregroundTextureMap[i] = QRect(x, y, this->tileSize.width(), this->tileSize.height());
	}

	// background texture coordinate
	for (int i = 0; i < 5; i ++) {
		// for convenience, the active selection background and the inactive selection background
		// is stored in the same map, access the active one by simply adding 5 to the current color index
		this->backgroundTextureMap[i] 		= 	QRect(i * 46, 0, 46, 56);
		this->backgroundTextureMap[i + 5] 	=	QRect(i * 46, 56, 46, 56);
	}

	// font texture coordinates
	for (int i = 0; i < 10; i ++) {
		this->fontTextureMap[i] = QRect(i * 11, 0, 11, 19);
	}

}

void Game::InitializeActions() {
	this->newGameAct = new QAction("New game");
	QObject::connect(newGameAct, SIGNAL(triggered()), this, SLOT(CreateNewGame()));
	
	this->exitGameAct = new QAction("Exit game");
	QObject::connect(this->exitGameAct, SIGNAL(triggered()), this, SLOT(ExitGame()));
	this->exitGameAct->setDisabled(true); // since a game hasn't been started yet, we disable this to prevent shit from fucking up

	this->closeWindowAct = new QAction("Close");
	QObject::connect(this->closeWindowAct, SIGNAL(triggered()), this, SLOT(CloseWindow()));
}

void Game::InitializeSoundEffects() {
	this->chooseSound.setSource(QUrl("qrc:///choose.wav"));
	this->cancelSound.setSource(QUrl ("qrc:///cancel.wav"));
	this->connectSound.setSource(QUrl ("qrc:///connect.wav"));
	this->shuffleSound.setSource(QUrl ("qrc:///shuffle.wav"));
	this->bellSound.setSource(QUrl ("qrc:///ding.wav"));
	this->errorSound.setSource(QUrl ("qrc:///uh_oh.wav"));
}

void Game::InitializeMusics() {
	this->musicList.addMedia(QUrl ("qrc:///Track 1.ogg"));
	this->musicList.addMedia(QUrl ("qrc:///Track 2.ogg"));
	this->musicList.addMedia(QUrl ("qrc:///Track 3.ogg"));
	this->musicList.addMedia(QUrl ("qrc:///Track 4.ogg"));
	this->musicList.addMedia(QUrl ("qrc:///Track 5.ogg"));
	
	this->musicList.setPlaybackMode(QMediaPlaylist::Loop);
	this->musicPlayer.setPlaylist(&this->musicList);
	this->musicPlayer.play();

	QObject::connect(&musicList, &QMediaPlaylist::currentIndexChanged, [this](int index)->void { 	
		if (index == 4 && this->currentScene == Scenes::IN_GAME) {
			std::uniform_int_distribution<int> distribution(9, 10);
			std::default_random_engine generator;
			generator.seed(this->seed);

			int random = distribution(generator);
			std::string fileName = ":/";
			fileName += ((random == 9) ? "0" : "") + std::to_string(random) + ".jpg";
			
			this->backgroundImage.load(QString::fromStdString(fileName));
			this->backgroundImage = this->backgroundImage.scaled(QSize(this->backgroundImage.size() / 5));
		}
	});
}

/*
 * 	EVENTS
 */

void Game::paintEvent(QPaintEvent*) {
	// you CANNOT create one single QPainter and use it, but you have to create one each event, no idea
	// what the deal with this it's just qt's quirks
	QPainter painter(this);

	this->UpdateMousePosition();
	this->Render(&painter);
}

void Game::mousePressEvent(QMouseEvent* event) {
	if (!this->IsMouseInBounds())
		return;

	int index = this->GetIndexUnderMouse();
	bool isIndexEmpty = !this->matrix.data[index];

	switch (event->button()) {
		// left click to select
		case Qt::LeftButton: {
			// if clicked on a non empty index and the player hasn't selected an index yet 
			// (remember that if that's the case, this->selectedIndex will always be -1)
			if (this->selectedIndex < 0 && !isIndexEmpty) {
				this->chooseSound.play(); // play choose sound
				this->selectedIndex = index;
			// if the player didn't click on the same index as the already selected one, and of course if the index is not empty
			} else if (this->selectedIndex != index && !isIndexEmpty) {
				// checks if the connection is actually valid, if so, start the connection 
				if (this->matrix.IsConnectionValid(this->selectedIndex, index)) {
					this->connectSound.play(); // play connect sound
					this->StartConnection(this->selectedIndex, index);
				// if the connection isn't valid, play the error sound (uh_oh)
				// and check if the player has run out of lives because we don't want them to make too many errors
				// hahahaahuwheuawheuawheuwahueahwueawhurawhuewaheawhuehwauawheuawe
				} else {
					this->errorSound.play(); // play error sound
								 
					this->remainingLives --;
					this->selectedIndex = -1;

					if (!this->remainingLives)
						this->GameOver();
				}
			}
			break;	
		}
		// right click is for... you guess it, de-select ;)
		case Qt::RightButton: {
			// checking if the player has selected anything
			if (this->selectedIndex >= 0) {
				this->cancelSound.play(); // play cancel sound
				this->selectedIndex = -1;
			}
			break;
		}
		default:
			break;
	}
}

/*
 * 	RENDER
 */

void Game::Render(QPainter* painter) {
	switch (this->currentScene) {
		case Scenes::TITLE: {
			this->RenderBackground(painter);
			break;
		}
		case Scenes::IN_GAME: {
			this->RenderBackground(painter);
			this->RenderTimer(painter);
			this->RenderLives(painter);
			this->RenderScore(painter);
			this->RenderTiles(painter);
			this->RenderSelection(painter);
			this->RenderConnection(painter);
			break;
		}
		default:
			break;
	}
}

void Game::RenderBackground(QPainter* painter) {
	switch (this->currentScene) {
		case Scenes::TITLE: {
			painter->drawPixmap(QPoint(0, 0), this->backgroundImage);
			break;
		}
		case Scenes::IN_GAME: {
			QSize windowSize = this->parentWidget()->size();
			QSize imageSize = this->backgroundImage.size();
			
			int x = (windowSize.width() - imageSize.width()) / 2;
			int y = (windowSize.height() - imageSize.height()) / 2;

			painter->fillRect(0, 0, windowSize.width(), windowSize.height(), Qt::black);
			painter->drawPixmap(QPoint(x, y), this->backgroundImage);
			break;
		}
		default:
			break;
	}
}

void Game::RenderTimer(QPainter* painter) {
	// setting up the timer brush, it has a gradient so sidjasiodjsaidsad
   	QLinearGradient gradient;

	gradient.setCoordinateMode(QGradient::ObjectMode);
	gradient.setStart(0, 1);
	gradient.setFinalStop(1, 1);

    	gradient.setColorAt(0, Qt::red);
	gradient.setColorAt(0.3, Qt::yellow);
    	gradient.setColorAt(1, Qt::green);

	painter->setPen(QPen (Qt::yellow));

	painter->drawRect(232, 6, 304, 20);
	painter->fillRect(234, 8, this->remainingSeconds, 17, QBrush(gradient));

	painter->setPen(Qt::NoPen);
}

void Game::RenderLives(QPainter* painter) {
	// the cross thingy, no idea what it's for
	painter->drawPixmap(QPoint(0, 0), this->cross);
	// the level counter number
	this->DrawNumber(painter, std::to_string(this->remainingLives), QPoint(43, 13), false);
}

void Game::RenderScore(QPainter* painter) {
	this->DrawNumber(painter, std::to_string(this->score), QPoint(681, 13), true);
}

void Game::RenderTiles(QPainter* painter) {
	for (int i = 0; i < 144; i ++) {
		int tileValue = this->matrix.data[i];	
		
		if (tileValue > 0) {
			QRect& foregroundPortion = this->foregroundTextureMap[tileValue];
			QRect& backgroundPortion = (this->selectedIndex == i) ? this->backgroundTextureMap[this->backgroundColorIndex + 5] 
										: this->backgroundTextureMap[this->backgroundColorIndex];

			QPoint position = this->matrix.IndexToPoint(i);
        		position.setX(position.x() * this->tileSize.width() + this->positionOffset.width());
			position.setY(position.y() * this->tileSize.height() + this->positionOffset.height());
       	
			painter->drawPixmap(position, this->background, backgroundPortion);
			painter->drawPixmap(position, this->foreground, foregroundPortion);
		}
	}
}

void Game::RenderSelection(QPainter* painter) {
	// IsMouseInBounds gives higher precision than Matrix's IsIndexOutOfBounds so we used that here
	if (!this->IsMouseInBounds())
		return;

	int index = this->GetIndexUnderMouse();
	int value = this->matrix.data[index];

	if (value > 0) {
		QPoint position = this->matrix.IndexToPoint(index);
        	position.setX(position.x() * this->tileSize.width() + this->positionOffset.width());
		position.setY(position.y() * this->tileSize.height() + this->positionOffset.height());
		
		painter->drawPixmap(position, this->selection);
	}
}

void Game::RenderConnection(QPainter* painter) {
	if (!this->isConnecting)
		return;

	QPainterPath line;

	bool isEdgeCase = (this->activeConnectionPath.size() == 5);
	bool isAdjacentCase = (this->activeConnectionPath.size() == 2);

	// used to center the point that is about to be used to draw lines
	const QSize centerOffset (this->tileSize.width() / 2.f, this->tileSize.height() / 2.f);
	// there's another positionOffset that is used to actually make the entire matrix centered, so we also need to account for that as well
	const QSize totalOffset (centerOffset.width() + this->positionOffset.width(), centerOffset.height() + this->positionOffset.height());

	// not much to say, the rest of the code is pretty self explainatory
	int startIndex = this->activeConnectionPath.front();
	int targetIndex = this->activeConnectionPath.back();

	QPoint startPoint 	 = 	this->matrix.IndexToPoint(startIndex);
	startPoint 		 = 	QPoint (startPoint.x() * this->tileSize.width() + totalOffset.width(), startPoint.y() * this->tileSize.height() + totalOffset.height());

	line.moveTo(startPoint);

	if (isAdjacentCase) {
		QPoint targetPoint 	= 	this->matrix.IndexToPoint(targetIndex);
		targetPoint 		= 	QPoint (targetPoint.x() * this->tileSize.width() + totalOffset.width(), targetPoint.y() * this->tileSize.height() + totalOffset.height());
	
		line.lineTo(targetPoint);
	} else {
		int stepIndex = this->activeConnectionPath[1];
		int parallelIndex = this->activeConnectionPath[2];
		
		targetIndex = this->activeConnectionPath[3];

		QPoint stepPoint = this->matrix.IndexToPoint(stepIndex);
		QPoint parallelPoint = this->matrix.IndexToPoint(parallelIndex);
		QPoint targetPoint = this->matrix.IndexToPoint(targetIndex);
		
		stepPoint 		= 	QPoint (stepPoint.x() * this->tileSize.width() + totalOffset.width(), stepPoint.y() * this->tileSize.height() + totalOffset.height());
		parallelPoint 		= 	QPoint (parallelPoint.x() * this->tileSize.width() + totalOffset.width(), parallelPoint.y() * this->tileSize.height() + totalOffset.height());
		targetPoint 		= 	QPoint (targetPoint.x() * this->tileSize.width() + totalOffset.width(), targetPoint.y() * this->tileSize.height() + totalOffset.height());

		if (isEdgeCase) {
			int directionIndex = this->activeConnectionPath[4];
	
			QSize edgeOffset = (directionIndex == 0 || directionIndex == 1) ? QSize(0, 50) : QSize(40, 0);
			edgeOffset 	*= (directionIndex == 1 || directionIndex == 2) ? -1 : 1;

			stepPoint 	+= QPoint (edgeOffset.width(), edgeOffset.height());
			parallelPoint	+= QPoint (edgeOffset.width(), edgeOffset.height());

		}

		line.lineTo(stepPoint);
		line.lineTo(parallelPoint);
		line.lineTo(targetPoint);
	}

	// the actual drawing part
	QPen linePen(QColor("#ffff00"));
	linePen.setWidth(2);

	QPainterPathStroker stroker;
	stroker.setCapStyle(Qt::RoundCap);	
	stroker.setJoinStyle(Qt::RoundJoin);
	stroker.setCurveThreshold(0.15);
	stroker.setWidth(3);	
	
	QPainterPath firstOutline = stroker.createStroke(line);
	
	stroker.setWidth(2);
	QPainterPath secondOutline = stroker.createStroke(firstOutline);

	painter->setPen(QPen(Qt::white));
	painter->drawPath(secondOutline);
	
	painter->setPen(QPen (QColor ("#ff0000")));
	painter->drawPath(firstOutline);
	
	painter->setPen(linePen);
	painter->drawPath(line);	
	
	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
}

/*
 *	UPDATES
 */


void Game::UpdateMousePosition() {
	QPoint globalMousePosition = QCursor::pos();
	this->mousePosition = this->mapFromGlobal(globalMousePosition);

	// we need to take in to account the offset we used to make the tiles align in the center
	// so that the top left actually starts at top left xd
	this->mousePosition.setX(this->mousePosition.x() - this->positionOffset.width());
	this->mousePosition.setY(this->mousePosition.y() - this->positionOffset.height());
}

/*
 *	GAME STATES
 */ 

void Game::GameOver() {
	if (!this->remainingLives) {	
		std::ostringstream stringStream;
		stringStream << "You've made too many errors and ran out of lives! Be careful next time :(";
		
		this->messageBox.setText(QString::fromStdString(stringStream.str()));
		this->messageBox.setWindowTitle("Game");
		this->messageBox.resize(QSize(100, 50));
		this->messageBox.setIcon(QMessageBox::Warning);

		this->messageBox.exec();
		this->ExitGame();
	}
}

void Game::AdvanceLevel() {
	this->bellSound.play();
	std::ostringstream stringStream;
	stringStream << "You've finished the game with " << this->remainingSeconds << " seconds to spare. You may go to level " << this->matrix.currentLevel + 1 << " now.";
	
	this->messageBox.setText(QString::fromStdString(stringStream.str()));
	this->messageBox.setWindowTitle("Game");
	this->messageBox.resize(QSize(100, 50));
	this->messageBox.setIcon(QMessageBox::Warning);

	this->messageBox.exec();

	this->GenerateSeed();
	this->SelectBackgroundColor();
	this->SelectBackgroundImage();

	// resetting the clock
	this->remainingSeconds = 301;
	// giving the player an additional live
	this->remainingLives ++;
	this->matrix.currentLevel ++;
	// initialize the matrix
	this->matrix.Initialize(this->seed);
}

/*
 * 	OTHER HELPFUL STUFF I GUESS
 */

// please let me know perhaps a better way of doing this... i'm not sure if this is standard practice
void Game::DrawNumber(QPainter* painter, std::string numberString, QPoint location, bool reversed) {
	const int spacing = 3;
	int offset = (reversed) ? -1 : 1;

	if (reversed) { std::reverse(numberString.begin(), numberString.end()); }
	
	QPoint position(location.x(), location.y());
	for (int i = 0; i < numberString.size(); i ++) {
		// subtract from 48 due to ascii stuff
		// https://stackoverflow.com/questions/5029840/convert-char-to-int-in-c-and-c
		int number = (numberString[i] - 48);
		QRect portion = this->fontTextureMap[number];
		position.setX(position.x() + (portion.width() + spacing) * offset);
		painter->drawPixmap(position, this->font, portion);
	}
}

int Game::GetIndexUnderMouse() {
	return ((this->mousePosition.y() / this->tileSize.height()) * 16 + (this->mousePosition.x() / this->tileSize.width()));
}

bool Game::IsMouseInBounds() {
	int x = this->mousePosition.x();
	int y = this->mousePosition.y();
	return x < 640 && x > 0 && y < 450 && y > 0; 
}

// generate a seed for the random number generators
void Game::GenerateSeed() {
	this->seed = std::chrono::system_clock::now().time_since_epoch().count();
}

// pick a random background color
void Game::SelectBackgroundColor() {
	std::uniform_int_distribution<int> distribution(0, 4);
	std::default_random_engine generator;
	generator.seed(this->seed);

	this->backgroundColorIndex = distribution(generator);
}

void Game::SelectBackgroundImage() {	
	std::uniform_int_distribution<int> distribution(1, 8);
	std::default_random_engine generator;
	generator.seed(this->seed);

	int imageIndex = distribution(generator);
	std::string fileName =  ":/";
	fileName += "0" + std::to_string(imageIndex) + ".jpg";

	this->backgroundImage.load(QString::fromStdString(fileName));
	this->backgroundImage = this->backgroundImage.scaled(QSize(this->backgroundImage.size() / 5));
}

void Game::StartConnection(int startIndex, int targetIndex) {
	this->isConnecting = true;
	this->activeConnectionPath = this->matrix.GetConnectionData(startIndex, targetIndex);

	QTimer::singleShot(250, this, SLOT(ClearConnection()));
}

void Game::ClearConnection() {
	bool isAdjacentCase = (this->activeConnectionPath.size() == 2);

	int startIndex = this->activeConnectionPath[0];
	int targetIndex = (isAdjacentCase) ? this->activeConnectionPath[1] : this->activeConnectionPath[3];
	
	this->selectedIndex = -1;
	this->isConnecting = false;
	this->activeConnectionPath.clear();
	this->score += 20;

	this->matrix.Update(startIndex, targetIndex, this->seed);
	
	if (!this->matrix.connectionPaths.size() && this->matrix.numConnectedIndices != 144) {
		this->shuffleSound.play();
		this->matrix.UpdateData(this->seed);
	} else if (!this->matrix.connectionPaths.size()){
		this->AdvanceLevel();
	}
}

void Game::CreateNewGame() {
	// disable new game and enable exit game options
	this->newGameAct->setDisabled(true);
	this->exitGameAct->setDisabled(false);

	this->GenerateSeed();
	this->SelectBackgroundColor();
	this->SelectBackgroundImage();

	this->remainingSeconds = 301;
	this->remainingLives = 10;
	this->score = 0;

	this->matrix.Initialize(this->seed);
	gameTimer.start();

	this->currentScene = Scenes::IN_GAME;
}

void Game::ExitGame() {
	// re-enable new game and disable exit game
	this->newGameAct->setDisabled(false);
	this->exitGameAct->setDisabled(true);
	
	this->gameTimer.stop();
	// clear all the game data for that current session
	this->matrix.ClearAll();

	this->backgroundImage.load(":/titlescreen.png");
	this->currentScene = Scenes::TITLE;
}

void Game::CloseWindow() {
	this->parentWidget()->close();
}
