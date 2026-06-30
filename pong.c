/*
* Napisati kratko uputstvo za koriscenje igrice
*
*/

#define _USE_MATH_DEFINES

#include <math.h>		
#include <stdio.h>
#include <stdlib.h>		
#include <time.h>
#include "Glut.h"

// Game state variables

typedef enum {
	MENU,
	PLAYING,
	PAUSED,
	GAME_OVER
} GameState;

GameState gameState = MENU;

int selectedMenuItem = 0; // 0 for Easy, 1 for Medium, 2 for Hard

// Global variables

const double Xmin = 0.0, Xmax = 3.0;
const double Ymin = 0.0, Ymax = 3.0;

int windowWidth = 360, windowHeight = 360;

double worldUnitsPerPixelX, worldUnitsPerPixelY;

double charWidthWorld, charHeightWorld;

int numLines = 30; // cosmetic value for drawing the center line

// Player variables

int playerScores[2] = { 0, 0 };
double playerYs[2] = { 1.5, 1.5 };
double verticalStep = 0.08;

int winner = 0;

// Paddle variables

double paddleWidth;
double paddleHeight;
double spaceBehindPaddle = 0.02;

// Ball variables

double ballX = 1.5, ballY = 1.5;
double startBallVX = 0.03, startBallVY = 0.0;
double ballVX = 0.03, ballVY = 0.0;
double ballSize = 0.025;
double speed;
double maxAngle = 60.0;

// 7-segment display variables

double displayWidth, displayHeight;
double verticalBlockWidth, verticalBlockHeight;
double horizontalBlockWidth, horizontalBlockHeight;

// Difficulty-related variables

typedef enum {
	EASY,
	MEDIUM,
	HARD,
	UNASSIGNED
} Difficulty;

Difficulty difficulty = UNASSIGNED;

double targetRegion;

// Update the target region of the paddle that player 0 should aim to hit the ball with

void updateTargetRegion() {
	targetRegion = (rand() % 50) / 100.0; // should produce numbers between 0 and 0.5
	printf("Trying to achieve bounce angle: +-%f\n", maxAngle - targetRegion * 2 * maxAngle);
}

// Resetting the game

void resetGame() {
	ballX = 1.5, ballY = 1.5;
	ballVX = startBallVX, ballVY = startBallVY;
	playerYs[0] = 1.5, playerYs[1] = 1.5;
	playerScores[0] = 0, playerScores[1] = 0;
}

// Check functions

void checkPlayerBounds(int playerId) {
	if (playerYs[playerId] >= Ymax - paddleHeight / 2) playerYs[playerId] = Ymax - paddleHeight / 2;
	if (playerYs[playerId] <= Ymin + paddleHeight / 2) playerYs[playerId] = Ymin + paddleHeight / 2;
}

int checkBallWithinPaddle(int playerId) {
	return (ballY + ballSize >= playerYs[playerId] - paddleHeight / 2) && (ballY - ballSize <= playerYs[playerId] + paddleHeight / 2);
}

// Keyboard functions

void myKeyboardFunc(unsigned char key, int x, int y)
{
	printf("Key pressed: %d\n", key);
	if (gameState == PLAYING) {
		switch (key) {
		case 27:
			// Escape key
			exit(0);
			break;
		case 'r':
		case 'R':
			// Reset scores
			resetGame();
			glutPostRedisplay();
			break;
		case 'p':
		case 'P':
			// Pause game
			gameState = PAUSED;
			glutPostRedisplay();
			break;
		}
	}
	else if (gameState == PAUSED) {
		switch (key) {
		case 'p':
		case 'P':
			gameState = PLAYING;
			break;
		case 'M':
		case 'm':
		case 27:
			gameState = MENU;
			resetGame();
			break;
		}
	}
	else if (gameState == GAME_OVER) {
		printf("Key pressed in GAME_OVER state: %d\n", key);
		switch (key) {
		case 27:		
			// Escape key
			exit(0);
			break;
		case 'm':
		case 'M':
			// Return to menu
			gameState = MENU;
			glutPostRedisplay();
			break;
		case 'r':
		case 'R':
			// Reset scores and start new game
			resetGame();
			gameState = PLAYING;
			break;
		}
	}
	else {
		switch (key) {
		case 27:
			// Escape key
			exit(0);
			break;
		case 13:
			// Enter key
			difficulty = (Difficulty)selectedMenuItem;
			printf("Selected difficulty: %d\n", difficulty);
			playerScores[0] = 0;
			playerScores[1] = 0;
			gameState = PLAYING;
			break;
		}
	}
}

void mySpecialKeyFunc(int key, int x, int y)
{
	if (gameState == MENU) {
		switch (key) {
		case GLUT_KEY_DOWN:
			// Move selection down
			selectedMenuItem = (selectedMenuItem + 1) % 3;
			break;
		case GLUT_KEY_UP:
			// Move selection up
			selectedMenuItem = (selectedMenuItem + 2) % 3; // adding 2 is equivalent to subtracting 1 modulo 3
			break;
		}
	} 
	else if (gameState == PLAYING) {
		switch (key) {
		case GLUT_KEY_UP:
			// Move player 1 up
			playerYs[1] += verticalStep;
			checkPlayerBounds(1);
			break;
		case GLUT_KEY_DOWN:
			// Move player 1 down
			playerYs[1] -= verticalStep;
			checkPlayerBounds(1);
			break;
		}
	}
}

// Drawing functions

void draw7SegmentDisplay(int playerId, int value) {
	int horizontalSegments[3] = { 0, 0, 0 };
	int verticalSegments[2][2] = { {0, 0}, {0, 0} };

	switch (value) {
	case 0:
		horizontalSegments[0] = 1, horizontalSegments[1] = 0, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 1, verticalSegments[0][1] = 1, verticalSegments[1][0] = 1, verticalSegments[1][1] = 1;
		break;
	case 1:
		horizontalSegments[0] = 0, horizontalSegments[1] = 0, horizontalSegments[2] = 0;
		verticalSegments[0][0] = 0, verticalSegments[0][1] = 1, verticalSegments[1][0] = 0, verticalSegments[1][1] = 1;
		break;
	case 2:
		horizontalSegments[0] = 1, horizontalSegments[1] = 1, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 0, verticalSegments[0][1] = 1, verticalSegments[1][0] = 1, verticalSegments[1][1] = 0;
		break;
	case 3:
		horizontalSegments[0] = 1, horizontalSegments[1] = 1, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 0, verticalSegments[0][1] = 1, verticalSegments[1][0] = 0, verticalSegments[1][1] = 1;
		break;
	case 4:
		horizontalSegments[0] = 0, horizontalSegments[1] = 1, horizontalSegments[2] = 0;
		verticalSegments[0][0] = 1, verticalSegments[0][1] = 1, verticalSegments[1][0] = 0, verticalSegments[1][1] = 1;
		break;
	case 5:
		horizontalSegments[0] = 1, horizontalSegments[1] = 1, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 1, verticalSegments[0][1] = 0, verticalSegments[1][0] = 0, verticalSegments[1][1] = 1;
		break;
	case 6:
		horizontalSegments[0] = 1, horizontalSegments[1] = 1, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 1, verticalSegments[0][1] = 0, verticalSegments[1][0] = 1, verticalSegments[1][1] = 1;
		break;
	case 7:
		horizontalSegments[0] = 1, horizontalSegments[1] = 0, horizontalSegments[2] = 0;
		verticalSegments[0][0] = 0, verticalSegments[0][1] = 1, verticalSegments[1][0] = 0, verticalSegments[1][1] = 1;
		break;
	case 8:
		horizontalSegments[0] = 1, horizontalSegments[1] = 1, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 1, verticalSegments[0][1] = 1, verticalSegments[1][0] = 1, verticalSegments[1][1] = 1;
		break;
	case 9:
		horizontalSegments[0] = 1, horizontalSegments[1] = 1, horizontalSegments[2] = 1;
		verticalSegments[0][0] = 1, verticalSegments[0][1] = 1, verticalSegments[1][0] = 0, verticalSegments[1][1] = 1;
		break;
	}

	double xStart = 0.0;
	double yStart = 0.0;

	double xCurrent = xStart;
	double yCurrent = yStart;

	glColor3f(1.0, 1.0, 1.0);

	// Draw horizontal blocks

	for (int i = 0; i < 3; i++) {
		if (horizontalSegments[i]) {
			glBegin(GL_QUADS);

			glVertex2f(xCurrent, yCurrent);
			glVertex2f(xCurrent + horizontalBlockWidth, yCurrent);
			glVertex2f(xCurrent + horizontalBlockWidth, yCurrent - horizontalBlockHeight);
			glVertex2f(xCurrent, yCurrent - horizontalBlockHeight);

			glEnd();
		}

		yCurrent -= 2*horizontalBlockHeight;
	}

	// Draw vertical blocks

	yCurrent = yStart;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			if (verticalSegments[i][j]) {
				glBegin(GL_QUADS);

				glVertex2f(xCurrent, yCurrent);
				glVertex2f(xCurrent + verticalBlockWidth, yCurrent);
				glVertex2f(xCurrent + verticalBlockWidth, yCurrent - verticalBlockHeight);
				glVertex2f(xCurrent, yCurrent - verticalBlockHeight);

				glEnd();
			}

			xCurrent = xStart + 2 * verticalBlockWidth;
		}
		xCurrent = xStart;
		yCurrent -= verticalBlockHeight;
	}
}

void drawPlayerScore(int playerId) {
	int playerScore = playerScores[playerId];
	double displayPositionOffset = 0.0;

	displayWidth = (Xmax - Xmin) / 10;
	displayHeight = (Ymax - Ymin) / 6;

	horizontalBlockHeight = displayHeight / 5;
	horizontalBlockWidth = displayWidth;

	verticalBlockHeight = displayHeight / 2;
	verticalBlockWidth = displayWidth / 3;

	if (playerId) {
		displayPositionOffset = (Xmax - Xmin) / 2;
	}

	int leadDigit = playerScore / 10;
	int lastDigit = playerScore % 10;

	double xStart = displayPositionOffset + (Xmax - Xmin) / 4;
	double yStart = Ymax - displayHeight / 4;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(xStart, yStart, 0);

	if (playerScore >= 10) {
		glTranslatef(-horizontalBlockWidth - verticalBlockWidth/2, 0.0, 0.0);
		draw7SegmentDisplay(playerId, leadDigit);
		glTranslatef(verticalBlockWidth + horizontalBlockWidth, 0, 0);
		draw7SegmentDisplay(playerId, lastDigit);
	} else {
		glTranslatef(-horizontalBlockWidth / 2, 0.0, 0.0);
		draw7SegmentDisplay(playerId, lastDigit);
	}

	glPopMatrix();
}

void drawCenterLine() {
	double x = (Xmax - Xmin) / 2;
	double step = (Ymax - Ymin) / (2 * numLines + 1);
	double y = Ymin + step;

	glBegin(GL_LINES);

	glColor3f(1.0, 1.0, 1.0);

	while (y <= Ymax - step) {
		glVertex2f(x, y);
		y += step;
		glVertex2f(x, y);
		y += step;
	}

	glEnd();
}

void drawPaddle(int playerId) {
	double playerX; // left edge of the paddle (in both cases)

	if (playerId) {
		playerX = Xmax - spaceBehindPaddle - paddleWidth;
	}
	else {
		playerX = Xmin + spaceBehindPaddle;
	}

	double playerY = playerYs[playerId];

	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(playerX, playerY + paddleHeight / 2);
	glVertex2f(playerX + paddleWidth, playerY + paddleHeight / 2);
	glVertex2f(playerX + paddleWidth, playerY - paddleHeight / 2);
	glVertex2f(playerX, playerY - paddleHeight / 2);
	glEnd();
}

void drawBall() {
	glBegin(GL_QUADS);

	glColor3f(1.0, 1.0, 1.0);
	glVertex2f(ballX - ballSize, ballY + ballSize);
	glVertex2f(ballX + ballSize, ballY + ballSize);
	glVertex2f(ballX + ballSize, ballY - ballSize);
	glVertex2f(ballX - ballSize, ballY - ballSize);

	glEnd();
}

void calculateTextPosition(char* text, double x, double y, double* outX, double* outY) {
	int textWidthPixels = 0;
	for (char* c = text; *c != '\0'; c++) {
		textWidthPixels += glutBitmapWidth(GLUT_BITMAP_9_BY_15, *c);
	}
	double textWidthWorld = textWidthPixels * worldUnitsPerPixelX;
	double textHeightWorld = 15 * worldUnitsPerPixelY;
	*outX = x - textWidthWorld / 2;
	*outY = y - textHeightWorld / 2;
}

void drawText(float x, float y, char* text)
{
	double tx, ty;
	calculateTextPosition(text, x, y, &tx, &ty);

	glRasterPos2f(tx, ty);

	while (*text)
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15,
			*text
		);

		text++;
	}
}

void drawPauseScreen()
{
	double xCenter = (Xmax - Xmin) / 2;
	glPushMatrix();
	drawText(xCenter, 2.0, "PAUSED");
	drawText(xCenter, 1.5, "PRESS P TO RESUME");
	drawText(xCenter, 1.0, "PRESS M TO RETURN TO MENU");
	glPopMatrix();
}

void drawMenu() {
	double xCenter = (Xmax - Xmin) / 2;
	glPushMatrix();

	drawText(xCenter, 2.5, "SELECT DIFFICULTY:");
	if (selectedMenuItem == EASY) {
		drawText(xCenter, 2.0, "> EASY <");
		drawText(xCenter, 1.5, "MEDIUM");
		drawText(xCenter, 1.0, "HARD");
	}
	else if (selectedMenuItem == MEDIUM) {
		drawText(xCenter, 2.0, "EASY");
		drawText(xCenter, 1.5, "> MEDIUM <");
		drawText(xCenter, 1.0, "HARD");
	}
	else {
		drawText(xCenter, 2.0, "EASY");
		drawText(xCenter, 1.5, "MEDIUM");
		drawText(xCenter, 1.0, "> HARD <");
	}

	glPopMatrix();
}

void drawEndScreen()
{
	double xCenter = (Xmax - Xmin) / 2;
	char* endText[2] = {"YOU LOSE!", "YOU WIN!"};
	glPushMatrix();
	drawText(xCenter, 2.5, "GAME OVER");
	drawText(xCenter, 2.0, endText[winner]);
	drawText(xCenter, 1.5, "PRESS R TO RESTART");
	drawText(xCenter, 1.0, "PRESS M TO RETURN TO MENU");
	glPopMatrix();
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	switch (gameState) {
	case MENU:
		drawMenu();
		break;
	case PLAYING:
		drawCenterLine();

		for (int i = 0; i < 2; i++) {
			drawPlayerScore(i);
			drawPaddle(i);
		}

		drawBall();
		break;
	case PAUSED:
		drawCenterLine();

		for (int i = 0; i < 2; i++) {
			drawPlayerScore(i);
			drawPaddle(i);
		}

		drawBall();

		drawPauseScreen();
		break;
	case GAME_OVER:
		drawEndScreen();
		break;
	}

	glutSwapBuffers();
}

// Ball collision checking and movement functions

void calculateBounce(int playerId) {
	//printf("Calculating bounce... \n");
	int direction = (playerId) ? -1 : 1;
	double normalizedYRelation = (ballY - playerYs[playerId]) / (paddleHeight / 2);
	if (normalizedYRelation > 1.0) normalizedYRelation = 1.0;
	else if (normalizedYRelation < -1.0) normalizedYRelation = -1.0;
	double bounceAngle = normalizedYRelation * maxAngle * M_PI / 180.0; // convert to radians
	if (ballVX < 0) printf("Bounce angle: %f degrees\n", bounceAngle * 180.0 / M_PI);
	speed *= 1.03;
	ballVX = direction * speed * cos(bounceAngle);
	ballVY = speed * sin(bounceAngle);
}

// Algorithm for player 1 movement

void movePlayer1() {
	double ballTop = ballY + ballSize, ballBottom = ballY - ballSize, ballLeft = ballX - ballSize;
	double paddleTop = playerYs[0] + paddleHeight / 2, paddleBottom = playerYs[0] - paddleHeight / 2;
	double paddleRightEdge = Xmin + spaceBehindPaddle + paddleWidth;
	switch (difficulty) {
	case EASY:
		if (ballBottom > paddleTop - 0.45*paddleHeight) {
			playerYs[0] += (rand() % 2) * (verticalStep * 0.6);
			checkPlayerBounds(0);
		}
		else if (ballTop < paddleBottom + 0.45*paddleHeight) {
			playerYs[0] -= (rand() % 2) * (verticalStep * 0.6);
			checkPlayerBounds(0);
		}
		break;
	case MEDIUM:
		if (ballBottom > paddleTop - 0.35*paddleHeight) {
			playerYs[0] += (rand() % 2) * (verticalStep * 0.8);
			checkPlayerBounds(0);
		}
		else if (ballTop < paddleBottom + 0.35*paddleHeight) {
			playerYs[0] -= (rand() % 2) * (verticalStep * 0.8);
			checkPlayerBounds(0);
		}
		break;
	case HARD:
		if (ballVX > 0) {
			return;
		}
		if (rand() % 7 < 2) {
			return;
		}
		if (ballBottom > paddleTop - targetRegion*paddleHeight) {
			playerYs[0] += verticalStep * 0.9;
			checkPlayerBounds(0);
		}
		else if (ballTop < paddleBottom + targetRegion * paddleHeight) {
			playerYs[0] -= verticalStep * 0.9;
			checkPlayerBounds(0);
		}
		break;
	}
}

// Timer function

void myTimer(int value) {
	if (gameState != PLAYING) {
		glutPostRedisplay();
		glutTimerFunc(16, myTimer, 0);
		return;
	}

	// Move ball
	speed = sqrt(pow(ballVX, 2) + pow(ballVY, 2));
	ballX += ballVX;
	ballY += ballVY;
	movePlayer1();
	// Check paddle collision
	if ((ballX + ballSize >= Xmax - spaceBehindPaddle - paddleWidth) && checkBallWithinPaddle(1)) {
		//printf("Player 2 hit the ball!\n");
		calculateBounce(1);
		if (difficulty == HARD) updateTargetRegion();
	}
	if ((ballX - ballSize <= Xmin + spaceBehindPaddle + paddleWidth) && checkBallWithinPaddle(0)) {
		printf("Player 1 hit the ball!\n");
		calculateBounce(0);
	}
	// Check if ball out of bounds - award point
	if (ballX + ballSize >= Xmax) {
		ballX = 1.5;
		ballY = 1.5;
		ballVX = startBallVX;
		ballVY = startBallVY;
		playerScores[0]++;
	}
	else if (ballX - ballSize <= Xmin) {
		ballX = 1.5;
		ballY = 1.5;
		ballVX = -startBallVX;
		ballVY = startBallVY;
		playerScores[1]++;
	}
	for (int i = 0; i < 2; i++) {
		if (playerScores[i] > 10) {
			winner = i;
			resetGame();
			gameState = GAME_OVER;
		}
	}
	// Check if ball hits top or bottom wall
	if (ballY + ballSize >= Ymax) {
		ballY = Ymax - ballSize;
		ballVY = -fabs(ballVY);
	}
	else if (ballY - ballSize <= Ymin) {
		ballY = Ymin + ballSize;
		ballVY = fabs(ballVY);
	}
	glutPostRedisplay();
	glutTimerFunc(16, myTimer, 0);
}

// Window functions

void initRendering()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void resizeWindow(int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	double scale, center;
	double windowXmin, windowXmax, windowYmin, windowYmax;

	glViewport(0, 0, w, h);
	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;

	if ((Xmax - Xmin) / w < (Ymax - Ymin) / h) {
		scale = ((Ymax - Ymin) / h) / ((Xmax - Xmin) / w);
		center = (Xmax + Xmin) / 2;
		windowXmin = center - (center - Xmin) * scale;
		windowXmax = center + (Xmax - center) * scale;
		windowYmin = Ymin;
		windowYmax = Ymax;
	}
	else {
		scale = ((Xmax - Xmin) / w) / ((Ymax - Ymin) / h);
		center = (Ymax + Ymin) / 2;
		windowYmin = center - (center - Ymin) * scale;
		windowYmax = center + (Ymax - center) * scale;
		windowXmin = Xmin;
		windowXmax = Xmax;
	}

	// Calculate conversion AFTER knowing the real visible bounds
	worldUnitsPerPixelX = (windowXmax - windowXmin) / windowWidth;
	worldUnitsPerPixelY = (windowYmax - windowYmin) / windowHeight;
	charWidthWorld = 9 * worldUnitsPerPixelX;
	charHeightWorld = 15 * worldUnitsPerPixelY;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(windowXmin, windowXmax, windowYmin, windowYmax, -1, 1);
}
// Main function

int main(int argc, char** argv)
{
	srand(time(NULL));

	paddleWidth = (Xmax - Xmin) / 42;
	paddleHeight = 7 * (Ymax - Ymin) / (2 * numLines + 1);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition(10, 60);
	glutInitWindowSize(360, 360);

	glutCreateWindow("Atari Pong [1972]");

	initRendering();

	glutKeyboardFunc(myKeyboardFunc);
	glutSpecialFunc(mySpecialKeyFunc);


	glutReshapeFunc(resizeWindow);


	glutDisplayFunc(drawScene);

	glutTimerFunc(16, myTimer, 0);

	glutMainLoop();

	return(0);
}

