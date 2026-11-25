#include <gui/screen2_screen/Screen2View.hpp>
#include "game_ipc.h"
#include "highscore_flash.h"

const int16_t Screen2View::LANE_X[LANE_COUNT] = { 36, 102, 168 };

static const int16_t ROAD_MIN_X = 24;
static const int16_t ROAD_MAX_X = 216;
static const int16_t HITBOX_MARGIN = 4;
static const int16_t BASE_SPEED = 3;
static const int16_t MAX_SPEED = 11;
static const int16_t TRACK_STEP = 6;

Screen2View::Screen2View()
    : trackFrame(0), trackAccum(0), carX(102), carY(240),
      speed(BASE_SPEED), score(0), highScore(0), tickCounter(0),
      rngState(123456789U), gameOver(false), lastDx(0), prevBtn(0)
{
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    tracks[0] = &track0;
    tracks[1] = &track1;
    tracks[2] = &track2;
    tracks[3] = &track3;
    tracks[4] = &track4;

    obstacles[0] = &lamb;
    obstacles[1] = &lamb2;

    carY = 320 - image1.getHeight() - 10;
    if (carY < 160)
    {
        carY = 220;
    }

    highScore = HighScore_Load();
    rngState = osKernelGetTickCount() ^ 0xA5A55A5AU;
    resetGame();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::ExitFromScreen2()
{
    application().gotoScreen1ScreenNoTransition();
}

uint32_t Screen2View::nextRand()
{
    rngState = rngState * 1664525U + 1013904223U;
    return rngState;
}

void Screen2View::spawnObstacle(int idx, int16_t startY)
{
    int other = 1 - idx;
    int lane = nextRand() % LANE_COUNT;
    if (obsY[other] < 80 && LANE_X[lane] == obsX[other])
    {
        lane = (lane + 1) % LANE_COUNT;
    }

    obsX[idx] = LANE_X[lane];
    obsY[idx] = startY;
    obstacles[idx]->setXY(obsX[idx], obsY[idx]);
}

void Screen2View::updateScoreUI()
{
    Unicode::snprintf(txtCurrentScoreBuffer, TXTCURRENTSCORE_SIZE, "%u", (unsigned int)score);
    txtCurrentScore.invalidate();

    Unicode::snprintf(txtHighScoreBuffer, TXTHIGHSCORE_SIZE, "%u", (unsigned int)highScore);
    txtHighScore.invalidate();
}

void Screen2View::resetGame()
{
    gameOver = false;
    score = 0;
    speed = BASE_SPEED;
    tickCounter = 0;
    trackFrame = 0;
    trackAccum = 0;
    carX = (240 - image1.getWidth()) / 2;
    lastDx = 0;

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        tracks[i]->setVisible(i == 0);
    }

    image1.setXY(carX, carY);

    for (int i = 0; i < NUM_OBS; i++)
    {
        obsX[i] = -100;
        obsY[i] = 1000;
    }
    spawnObstacle(0, -obstacles[0]->getHeight() - 20);
    spawnObstacle(1, -obstacles[1]->getHeight() - 200);

    txtGameOver.setVisible(false);
    btnRestart.setVisible(false);
    updateScoreUI();
    invalidate();
}

bool Screen2View::checkCollision(int idx)
{
    int16_t cL = carX + HITBOX_MARGIN;
    int16_t cR = carX + image1.getWidth() - HITBOX_MARGIN;
    int16_t cT = carY + HITBOX_MARGIN;
    int16_t cB = carY + image1.getHeight() - HITBOX_MARGIN;

    int16_t oL = obsX[idx] + HITBOX_MARGIN;
    int16_t oR = obsX[idx] + obstacles[idx]->getWidth() - HITBOX_MARGIN;
    int16_t oT = obsY[idx] + HITBOX_MARGIN;
    int16_t oB = obsY[idx] + obstacles[idx]->getHeight() - HITBOX_MARGIN;

    return (cL < oR) && (cR > oL) && (cT < oB) && (cB > oT);
}

void Screen2View::restartClicked()
{
    if (gameOver)
    {
        resetGame();
    }
}

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();

    InputMsg_t msg;
    uint8_t btnPressed = 0;
    while (osMessageQueueGetCount(joyQueueHandle) > 0)
    {
        if (osMessageQueueGet(joyQueueHandle, &msg, NULL, 0) == osOK)
        {
            lastDx = msg.dx;
            btnPressed |= msg.btn;
            rngState ^= msg.raw;
        }
    }

    uint8_t btnRising = btnPressed && !prevBtn;
    prevBtn = btnPressed;

    if (gameOver)
    {
        if (btnRising)
        {
            resetGame();
        }
        return;
    }

    tickCounter++;

    trackAccum += speed;
    if (trackAccum >= TRACK_STEP)
    {
        trackAccum -= TRACK_STEP;
        tracks[trackFrame]->setVisible(false);
        trackFrame = (trackFrame + 1) % NUM_TRACKS;
        tracks[trackFrame]->setVisible(true);
        tracks[trackFrame]->invalidate();
    }

    if (lastDx != 0)
    {
        image1.invalidate();
        carX += lastDx;
        int16_t maxCarX = ROAD_MAX_X - image1.getWidth();
        if (carX < ROAD_MIN_X) carX = ROAD_MIN_X;
        if (carX > maxCarX) carX = maxCarX;
        image1.setX(carX);
        image1.invalidate();
    }

    bool scoreChanged = false;
    if ((tickCounter % 15U) == 0U)
    {
        score++;
        scoreChanged = true;
    }

    for (int i = 0; i < NUM_OBS; i++)
    {
        obstacles[i]->invalidate();
        obsY[i] += speed;

        if (obsY[i] > 320)
        {
            score += 5;
            scoreChanged = true;
            int16_t jitter = (int16_t)(nextRand() % 70U);
            spawnObstacle(i, -obstacles[i]->getHeight() - jitter);
        }
        else
        {
            obstacles[i]->setY(obsY[i]);
        }
        obstacles[i]->invalidate();

        if (checkCollision(i))
        {
            gameOver = true;
            if (score > highScore)
            {
                highScore = score;
                HighScore_Save(highScore);
            }
            updateScoreUI();
            txtGameOver.setVisible(true);
            btnRestart.setVisible(true);
            txtGameOver.invalidate();
            btnRestart.invalidate();
            return;
        }
    }

    int16_t newSpeed = BASE_SPEED + (int16_t)(score / 25U);
    speed = (newSpeed > MAX_SPEED) ? MAX_SPEED : newSpeed;

    if (scoreChanged)
    {
        updateScoreUI();
    }
}
