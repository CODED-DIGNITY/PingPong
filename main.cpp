#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stdio.h>

// Game State Definition
typedef enum GameState {
    Title_Screen = 0,
    Gameplay,
    Game_Over
} GameState;

// Resets ball
void ResetBall(Vector2 *ballPosition, Vector2 *ballVelocity, int screenW, int screenH, float initialSpeed, bool Serve_Left) {
    ballPosition->x = (float)screenW / 2.0f;
    ballPosition->y = (float)screenH / 2.0f;

    float speedX = Serve_Left ? -initialSpeed : initialSpeed;
    float speedY = initialSpeed * (GetRandomValue(-5, 5) / 10.0f);

    Vector2 initialDirection = Vector2Normalize((Vector2){speedX, speedY});
    if (fabsf(initialDirection.y) < 0.05f) { // Avoid straight ball
        initialDirection.y = (GetRandomValue(0, 1) == 0 ? -0.1f : 0.1f);
        initialDirection = Vector2Normalize(initialDirection);
    }
    *ballVelocity = Vector2Scale(initialDirection, initialSpeed);
}

int main() {
    InitWindow(0, 0, "Chaos Pong");
    SetTargetFPS(120);
    if (!IsWindowFullscreen()) {
        ToggleFullscreen();
    }
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    InitAudioDevice();

    //Sounds
    Sound soundBounce = LoadSound("bounce.wav");
    Sound soundScore = LoadSound("score.wav");
    Sound soundGameOver = LoadSound("gameover.wav");

    // Game Constants
    const float Paddle_W = 25.0f;
    const float Paddle_L = 250.0f;
    const float Paddle_Speed = 1200.0f;
    const float Paddle_Border = 60.0f;
    const float Ball_R = 30.0f;
    const float Ball_Start_Speed = 750.0f;
    const float Ball_Bounce_factor_max = 0.8f;
    const float Ball_Speed_Multiplyer = 1.15f;
    const int Winning_Score = 5;

    const Color Paddle_Colours[] = { WHITE, SKYBLUE, LIME, GOLD, PINK, ORANGE };
    const int Num_Paddle_Colours = sizeof(Paddle_Colours) / sizeof(Paddle_Colours[0]);

    GameState currentGameState = Title_Screen;
    Rectangle leftPaddle = { Paddle_Border, (float)screenHeight / 2.0f - Paddle_L / 2.0f, Paddle_W, Paddle_L };
    int LP_Colour_Index = 1;
    Color LP_Colour = Paddle_Colours[LP_Colour_Index];
    Rectangle rightPaddle = { (float)screenWidth - Paddle_W - Paddle_Border, (float)screenHeight / 2.0f - Paddle_L / 2.0f, Paddle_W, Paddle_L };
    int RP_Colour_Index = 2;
    Color RP_Colour = Paddle_Colours[RP_Colour_Index];
    Vector2 ballPos = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    Vector2 ballVel = { 0.0f, 0.0f };
    Color ballColor = WHITE;
    int leftPlayerScore = 0;
    int rightPlayerScore = 0;
    Vector2 menuBallPos = { (float)screenWidth / 4.0f, (float)screenHeight / 3.0f };
    Vector2 menuBallVel = { Ball_Start_Speed * 0.8f, Ball_Start_Speed * 0.6f };
    const float Menu_Ball_R = 40.0f;
    char gameOverMessage[64] = {0};


    // Main Game Loop
    while (!WindowShouldClose()) {

        float deltaTime = GetFrameTime();

        switch (currentGameState) {

            case Title_Screen: {
                //menu animation
                menuBallPos = Vector2Add(menuBallPos, Vector2Scale(menuBallVel, deltaTime));
                if (menuBallPos.x - Menu_Ball_R <= 0 || menuBallPos.x + Menu_Ball_R >= screenWidth) menuBallVel.x *= -1;
                if (menuBallPos.y - Menu_Ball_R <= 0 || menuBallPos.y + Menu_Ball_R >= screenHeight) menuBallVel.y *= -1;

                //color selection 
                if (IsKeyPressed(KEY_W)) {
                    LP_Colour_Index--; 
                    if (LP_Colour_Index < 0) {
                        LP_Colour_Index = Num_Paddle_Colours - 1;
                    }
                    LP_Colour = Paddle_Colours[LP_Colour_Index];
                }
                
                if (IsKeyPressed(KEY_S)) {
                    LP_Colour_Index++; 
                    if (LP_Colour_Index >= Num_Paddle_Colours) {
                        LP_Colour_Index = 0;
                    }
                    LP_Colour = Paddle_Colours[LP_Colour_Index];
                }
                
                if (IsKeyPressed(KEY_UP)) {
                    RP_Colour_Index--; 
                    if (RP_Colour_Index < 0) {
                        RP_Colour_Index = Num_Paddle_Colours - 1;
                    }
                    RP_Colour = Paddle_Colours[RP_Colour_Index];
                }
                
                if (IsKeyPressed(KEY_DOWN)) {
                    RP_Colour_Index++; 
                    if (RP_Colour_Index >= Num_Paddle_Colours) {
                        RP_Colour_Index = 0;
                    }
                    RP_Colour = Paddle_Colours[RP_Colour_Index];
                }

                // start game
                if (IsKeyPressed(KEY_ENTER)) {
                    currentGameState = Gameplay;
                    leftPlayerScore = 0; rightPlayerScore = 0;
                    leftPaddle.y = (float)screenHeight / 2.0f - Paddle_L / 2.0f;
                    rightPaddle.y = (float)screenHeight / 2.0f - Paddle_L / 2.0f;
                    ResetBall(&ballPos, &ballVel, screenWidth, screenHeight, Ball_Start_Speed, GetRandomValue(0, 1));
                    ballColor = WHITE;
                    gameOverMessage[0] = '\0';
                }
            } break;

            case Gameplay: {
                if (IsKeyDown(KEY_W)) leftPaddle.y -= Paddle_Speed * deltaTime;
                if (IsKeyDown(KEY_S)) leftPaddle.y += Paddle_Speed * deltaTime;
                if (IsKeyDown(KEY_UP)) rightPaddle.y -= Paddle_Speed * deltaTime;
                if (IsKeyDown(KEY_DOWN)) rightPaddle.y += Paddle_Speed * deltaTime;

                if (leftPaddle.y < 0) leftPaddle.y = 0;
                if (leftPaddle.y + leftPaddle.height > screenHeight) leftPaddle.y = screenHeight - leftPaddle.height;
                if (rightPaddle.y < 0) rightPaddle.y = 0;
                if (rightPaddle.y + rightPaddle.height > screenHeight) rightPaddle.y = screenHeight - rightPaddle.height;

                ballPos = Vector2Add(ballPos, Vector2Scale(ballVel, deltaTime));

                // Ball-Paddle Collision
                if (CheckCollisionCircleRec(ballPos, Ball_R, leftPaddle) && ballVel.x < 0) {
                    ballVel.x *= -1.0f;
                    // Calculate normalized impact
                    float impact = (ballPos.y - (leftPaddle.y + Paddle_L / 2.0f)) / (Paddle_L / 2.0f);
                    ballVel.y = fmaxf(-1.0f, fminf(1.0f, impact)) * fabsf(ballVel.x) * Ball_Bounce_factor_max;
                    ballVel = Vector2Scale(ballVel, Ball_Speed_Multiplyer);
                    ballColor = LP_Colour;
                    ballPos.x = leftPaddle.x + leftPaddle.width + Ball_R; 
                    PlaySound(soundBounce);
                }
                if (CheckCollisionCircleRec(ballPos, Ball_R, rightPaddle) && ballVel.x > 0) {
                    ballVel.x *= -1.0f;
                    float impact = (ballPos.y - (rightPaddle.y + Paddle_L / 2.0f)) / (Paddle_L / 2.0f);
                     ballVel.y = fmaxf(-1.0f, fminf(1.0f, impact)) * fabsf(ballVel.x) * Ball_Bounce_factor_max;
                    ballVel = Vector2Scale(ballVel, Ball_Speed_Multiplyer);
                    ballColor = RP_Colour;
                    ballPos.x = rightPaddle.x - Ball_R; 
                    PlaySound(soundBounce);
                }

                // ball collition top bottom
                if (ballPos.y - Ball_R <= 0) { ballPos.y = Ball_R; ballVel.y *= -1; }
                if (ballPos.y + Ball_R >= screenHeight) { ballPos.y = screenHeight - Ball_R; ballVel.y *= -1; }

                // Ball collition left right for scoring
                bool scored = false;
                bool serveLeftNext = false;
                if (ballPos.x - Ball_R <= 0) { // Right player scores
                    rightPlayerScore++; scored = true; serveLeftNext = false;
                    if (rightPlayerScore >= Winning_Score) {
                        currentGameState = Game_Over;
                        snprintf(gameOverMessage, sizeof(gameOverMessage), "Right Player Wins!");
                        PlaySound(soundGameOver); }
                    else { PlaySound(soundScore); }
                }
                if (ballPos.x + Ball_R >= screenWidth) { // Left player scores
                    leftPlayerScore++; scored = true; serveLeftNext = true;
                     if (leftPlayerScore >= Winning_Score) {
                        currentGameState = Game_Over;
                        snprintf(gameOverMessage, sizeof(gameOverMessage), "Left Player Wins!");
                        PlaySound(soundGameOver); }
                     else { PlaySound(soundScore); }
                }

                // Reset ball
                if (scored && currentGameState == Gameplay) {
                    ResetBall(&ballPos, &ballVel, screenWidth, screenHeight, Ball_Start_Speed, serveLeftNext);
                    ballColor = WHITE;
                }

            } break;

            case Game_Over: {
                if (IsKeyPressed(KEY_ENTER)) {
                    currentGameState = Title_Screen;
                }
            } break;

            default: break;
        } 

        // Draw Phase
        BeginDrawing();
            ClearBackground(BLACK);

            switch (currentGameState) {
                case Title_Screen: {
                    DrawCircleV(menuBallPos, Menu_Ball_R, LIGHTGRAY);
                    const char* titleText = "Chaos Pong";
                    int titleTextWidth = MeasureText(titleText, 70);
                    DrawText(titleText, screenWidth / 2 - titleTextWidth / 2, screenHeight / 5, 70, WHITE);
                    int boxWidth = 180; int boxHeight = 90; int boxY = screenHeight * 2 / 4 + 20; int boxPadding = 60;
                    int leftBoxX = screenWidth / 2 - boxWidth - boxPadding / 2;
                    DrawRectangleLines(leftBoxX, boxY, boxWidth, boxHeight, WHITE);
                    DrawText("Left Color", leftBoxX + 10, boxY + 10, 18, WHITE);
                    DrawRectangle(leftBoxX + boxWidth / 4, boxY + 35, boxWidth / 2, 30, LP_Colour);
                    DrawText("W/S", leftBoxX + boxWidth / 2 - MeasureText("W/S", 18) / 2, boxY + boxHeight + 5, 18, GRAY);
                    int rightBoxX = screenWidth / 2 + boxPadding / 2;
                    DrawRectangleLines(rightBoxX, boxY, boxWidth, boxHeight, WHITE);
                    DrawText("Right Color", rightBoxX + 10, boxY + 10, 18, WHITE);
                    DrawRectangle(rightBoxX + boxWidth / 4, boxY + 35, boxWidth / 2, 30, RP_Colour);
                    DrawText("Up/Down", rightBoxX + boxWidth / 2 - MeasureText("Up/Down", 18) / 2, boxY + boxHeight + 5, 18, GRAY);
                    const char* startText = "Press ENTER to Play";
                    int startTextWidth = MeasureText(startText, 25);
                    DrawText(startText, screenWidth / 2 - startTextWidth / 2, screenHeight * 4 / 5, 25, GRAY);
                } break;

                case Gameplay: {
                    DrawRectangleRec(leftPaddle, LP_Colour);
                    DrawRectangleRec(rightPaddle, RP_Colour);
                    DrawCircleV(ballPos, Ball_R, ballColor);
                    char scoreStr[16];
                    snprintf(scoreStr, sizeof(scoreStr), "%d", leftPlayerScore);
                    int leftScoreWidth = MeasureText(scoreStr, 50);
                    DrawText(scoreStr, screenWidth / 4 - leftScoreWidth / 2, 15, 50, LIGHTGRAY);
                    snprintf(scoreStr, sizeof(scoreStr), "%d", rightPlayerScore);
                    int rightScoreWidth = MeasureText(scoreStr, 50);
                    DrawText(scoreStr, screenWidth * 3 / 4 - rightScoreWidth / 2, 15, 50, LIGHTGRAY);
                    DrawFPS(10, 10);
                } break;

                case Game_Over: {
                    int winnerTextWidth = MeasureText(gameOverMessage, 60);
                    DrawText(gameOverMessage, screenWidth / 2 - winnerTextWidth / 2, screenHeight / 4, 60, GOLD);
                    char finalScoreStr[64];
                    snprintf(finalScoreStr, sizeof(finalScoreStr), "%d - %d", leftPlayerScore, rightPlayerScore);
                    int finalScoreWidth = MeasureText(finalScoreStr, 40);
                    DrawText(finalScoreStr, screenWidth / 2 - finalScoreWidth / 2, screenHeight / 2, 40, LIGHTGRAY);
                    const char* restartText = "Press ENTER for Title Screen";
                    int restartTextWidth = MeasureText(restartText, 25);
                    DrawText(restartText, screenWidth / 2 - restartTextWidth / 2, screenHeight * 3 / 4, 25, GRAY);
                } break;

                default: break;
            }
        EndDrawing();
    } 

    UnloadSound(soundBounce);
    UnloadSound(soundScore);
    UnloadSound(soundGameOver);
    CloseAudioDevice();
    CloseWindow();

    return 0;
} 