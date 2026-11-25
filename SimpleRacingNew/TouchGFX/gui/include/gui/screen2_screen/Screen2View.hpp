#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void restartClicked();
    virtual void ExitFromScreen2();

protected:
    static const int NUM_TRACKS = 5;
    static const int NUM_OBS = 2;
    static const int LANE_COUNT = 3;
    static const int16_t LANE_X[LANE_COUNT];

    touchgfx::Image* tracks[NUM_TRACKS];
    touchgfx::Image* obstacles[NUM_OBS];
    int16_t obsX[NUM_OBS];
    int16_t obsY[NUM_OBS];

    int trackFrame;
    int16_t trackAccum;
    int16_t carX;
    int16_t carY;
    int16_t speed;
    uint32_t score;
    uint32_t highScore;
    uint32_t tickCounter;
    uint32_t rngState;
    bool gameOver;
    int8_t lastDx;
    uint8_t prevBtn;

    void resetGame();
    void spawnObstacle(int idx, int16_t startY);
    bool checkCollision(int idx);
    void updateScoreUI();
    uint32_t nextRand();
};

#endif
