#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Defining constant values for stuff because otherwise its causing mem errors and I don't know why
// grid and window shit i dont know the math for becausee hehe
#define GRID            10          
#define CELL            36        
#define WINDOWWIDTH     1024
#define WINDOWHEIGHT    640
#define BOARDPIXELS     (GRID * CELL)  
#define PANELWIDTH      120                       
#define BOARDGAP        20                         

#define SIDEMARGIN      ((WINDOWWIDTH - 2*BOARDPIXELS - PANELWIDTH - 2*BOARDGAP) / 2)
#define LBOARDX         SIDEMARGIN
#define PANELX          (LBOARDX + BOARDPIXELS + BOARDGAP)
#define RBOARDX         (PANELX + PANELWIDTH + BOARDGAP)
#define BOARDTOPY       ((WINDOWHEIGHT - BOARDPIXELS - 40) / 2) 

// COLORS for the ui

#define BACKGROUND          ((Color){ 15,  20,  30, 255})
#define OCEANBLUE           ((Color){ 20,  50,  90, 255})
#define GRIDLINE            ((Color){ 30,  65, 110, 255})
#define FSHIPCOLOR          ((Color){ 80,  90, 100, 255})
#define EXPLOSION           ((Color){220,  60,  60, 255})
#define SPLASH              ((Color){160, 170, 185, 255})
#define PREVIEWV            ((Color){255, 230,  80, 140})
#define PREVIEWB            ((Color){220,  60,  60, 140})
#define AIM                 ((Color){255, 255, 255,  80})
#define LASTSHOT            ((Color){255, 220,  50, 200}) 
#define PANELBACKGROUND     ((Color){ 22,  30,  45, 255})
#define PANELBORDERCOLOR    ((Color){ 50,  70, 110, 255})
#define TEXT                ((Color){200, 210, 225, 255})
#define ACCENT              ((Color){ 80, 160, 255, 255})
#define WARNING             ((Color){255, 180,  40, 255})
#define VICTORY             ((Color){ 60, 200, 100, 255})
#define DANGER              ((Color){220,  60,  60, 255})

// variable time :3

// initboard variables

char p1Board[GRID][GRID];
char p2Board[GRID][GRID];

int p1ShipsRemaining = 0;
int p2ShipsRemaining = 0;
int p1ShotsFired     = 0;
int p2ShotsFired     = 0;

// game state (and my mental state :D)
// 0=main menu   1=p1 placing   2=p2 placing  3=battle  4=game over  5=switch screen

int gameState       = 0;
int Turn            = 1;   
int twoplayermode   = 1;      

// THE ROYAL FLEET 

int fleetSizes[]     = {5, 4, 3};
int nextShip         = 0;          
int orientation      = 0;   //BOOL KYUN NAI CHALRHA

int lastShotreport   = 0;  
int lastShotRow      = -1;  
int lastShotCol      = -1;

// AI delay thingy─
#define AIDELAY           1.2f  
int   waitingForAI      = 0;
float aiTimer           = 0.0f;

int switchscreen = 0;  

// FUCNTIONS YAHAN SE START HAIN

// initboard
void initBoard(char board[GRID][GRID]) {
    for (int row = 0; row < GRID; row++){
        for (int col = 0; col < GRID; col++){
            board[row][col] = '~';
        }    
    }  
}

void drawBoard(char board[GRID][GRID],int startX, int startY, int hideShips){
    for (int row = 0; row < GRID; row++){
        for (int col = 0; col < GRID; col++){
           
            Color cellColor = OCEANBLUE;

            if(board[row][col] == 'X'){
                cellColor = EXPLOSION;
            }
            else if(board[row][col] == 'O'){
                cellColor = SPLASH;
            }
            else if(board[row][col] == 'S' && !hideShips){
                cellColor = FSHIPCOLOR;
            }    

            int pixelX = startX + col * CELL;
            int pixelY = startY + row * CELL;
            DrawRectangle(pixelX, pixelY, CELL, CELL, cellColor);
            DrawRectangleLines(pixelX, pixelY, CELL, CELL, GRIDLINE);
        }
    }
}

void drawLastShot(int boardStartX, int boardStartY) {
    if (lastShotRow < 0 || lastShotCol < 0){
        return;
    };
    int pixelX = boardStartX + lastShotCol * CELL;
    int pixelY = boardStartY + lastShotRow * CELL;
    
    DrawRectangleLines(pixelX - 1, pixelY - 1, CELL + 2, CELL + 2, LASTSHOT);
    DrawRectangleLines(pixelX - 2, pixelY - 2, CELL + 4, CELL + 4, LASTSHOT);
    DrawRectangleLines(pixelX - 3, pixelY - 3, CELL + 6, CELL + 6, LASTSHOT);
}


// cool trick i learned we can use ? and : as basically a smoll if statement
// Syntax = condition ? value_if_true : value_if_false

void drawPlacementGhost(char board[GRID][GRID],int startX, int startY,int hoverRow, int hoverCol,
    int shipLength, int vertical){
    for (int segment = 0; segment < shipLength; segment++){

        int targetRow = hoverRow + (vertical ? segment : 0);
        int targetCol = hoverCol + (vertical ? 0 : segment);

        if (targetRow >= GRID || targetCol >= GRID){
            continue;
        } 

        Color ghostColor = (board[targetRow][targetCol] != '~') ? PREVIEWB : PREVIEWV;
        DrawRectangle(startX + targetCol * CELL , startY + targetRow * CELL, CELL, CELL, ghostColor);
    }
}

int IsValidPlacement(char board[GRID][GRID],int startRow, int startCol,int shipLength, int vertical){
    for (int segment = 0; segment < shipLength; segment++){
        
        int checkRow = startRow + (vertical ? segment : 0);
        int checkCol = startCol + (vertical ? 0 : segment);
        if (checkRow >= GRID || checkCol >= GRID){
            return 0;
        } 
        if (board[checkRow][checkCol] != '~'){
            return 0;   
        }      
    }
    return 1;
}

void placeShipOnBoard(char board[GRID][GRID],int startRow, int startCol,int shipLength, int vertical,
    int *shipCellCount){
    for (int segment = 0; segment < shipLength; segment++){
        
        int targetRow = startRow + (vertical ? segment : 0);
        int targetCol = startCol + (vertical ? 0 : segment);

        board[targetRow][targetCol] = 'S';
        (*shipCellCount)++;
    }
}

int shoot(char board[GRID][GRID],int targetRow, int targetCol,int *shipsRemaining) {
    
    if (board[targetRow][targetCol] == 'S'){
        board[targetRow][targetCol] = 'X';
        (*shipsRemaining)--;
        return 1;
    }

    if (board[targetRow][targetCol] == '~') {
        board[targetRow][targetCol] = 'O';
    }

    return 0;
} 

void AIShoot() {
    int targetRow, targetCol;

    do {
        targetRow = rand() % GRID;
        targetCol = rand() % GRID;
    } while (p1Board[targetRow][targetCol] == 'X' ||p1Board[targetRow][targetCol] == 'O');
    
    shoot(p1Board, targetRow, targetCol, &p1ShipsRemaining);
    p2ShotsFired++;

    lastShotRow = targetRow;
    lastShotCol = targetCol;
}

void fullReset() {
    initBoard(p1Board);
    initBoard(p2Board);
    p1ShipsRemaining = 0;
    p2ShipsRemaining = 0;
    p1ShotsFired     = 0;
    p2ShotsFired     = 0;
    nextShip         = 0;
    orientation      = 0;
    gameState        = 0;
    Turn             = 1;
    lastShotreport   = 0;
    lastShotRow      = -1;
    lastShotCol      = -1;
    waitingForAI     = 0;
    aiTimer          = 0.0f;
    switchscreen     = 0;
}

void drawStatsPanel() {
    int panelLeft   = PANELX;
    int panelTop    = BOARDTOPY;
    int panelCentre = panelLeft + PANELWIDTH / 2;
    int textCursor  = panelTop + 12;

    DrawRectangle(panelLeft, panelTop, PANELWIDTH, BOARDPIXELS, PANELBACKGROUND);
    DrawRectangleLines(panelLeft, panelTop, PANELWIDTH, BOARDPIXELS, PANELBORDERCOLOR);

    // title
    const char *title = "Battle Report";
    DrawText(title, panelCentre - MeasureText(title, 16)/2,textCursor, 16, ACCENT); 
    textCursor += 26;

    DrawLine(panelLeft+8, textCursor, panelLeft+PANELWIDTH-8, textCursor, PANELBORDERCOLOR);
    textCursor += 10;

    // Player 1 stats 
    Color player1Color = (Turn == 1 && gameState == 3)? ACCENT : TEXT;
    DrawText("P1", panelCentre - MeasureText("P1", 14)/2, textCursor, 14, player1Color);
    textCursor += 20;

    char statLine[32];
    sprintf(statLine, "Ships: %d", p1ShipsRemaining);
    DrawText(statLine, panelCentre - MeasureText(statLine, 13)/2, textCursor, 13, TEXT);
    textCursor += 18;

    sprintf(statLine, "Shots: %d", p1ShotsFired);
    DrawText(statLine, panelCentre - MeasureText(statLine, 13)/2,textCursor, 13, TEXT);
    textCursor += 24;

    DrawLine(panelLeft+8, textCursor, panelLeft+PANELWIDTH-8,textCursor, PANELBORDERCOLOR);
    textCursor += 10;

    // Player 2 stats
    Color player2Color = (Turn == 2 && gameState == 3)? ACCENT : TEXT;
    const char *player2Label = twoplayermode ? "P2" : "AI";
    DrawText(player2Label, panelCentre - MeasureText(player2Label, 14)/2, textCursor, 14, player2Color);
    textCursor += 20;

    sprintf(statLine, "Ships: %d", p2ShipsRemaining);
    DrawText(statLine, panelCentre - MeasureText(statLine, 13)/2,textCursor, 13, TEXT);
    textCursor += 18;

    sprintf(statLine, "Shots: %d", p2ShotsFired);
    DrawText(statLine, panelCentre - MeasureText(statLine, 13)/2,textCursor, 13, TEXT);
    textCursor += 24;

    DrawLine(panelLeft+8, textCursor, panelLeft+PANELWIDTH-8,textCursor, PANELBORDERCOLOR);
    textCursor += 10;

    // Active turn
    if (gameState == 3) {
        const char *turnBadge = (Turn == 1) ? "P1 FIRES": (twoplayermode   ? "P2 FIRES" : "AI...");
        DrawText(turnBadge,panelCentre - MeasureText(turnBadge, 13)/2,textCursor, 13, WARNING);
    }
}

// MAIN GAME LOOP :D (I will cry)

int main() {
    InitWindow(WINDOWWIDTH, WINDOWHEIGHT, "Weird Battleship Game");
    SetTargetFPS(60);
    srand(time(NULL));

    InitAudioDevice();// BACKGROUND MUSIC GOIZ
    Music bgmusic = LoadMusicStream("Soundtrack.mp3");
    if (!IsMusicStreamPlaying(bgmusic)) {
        PlayMusicStream(bgmusic); //loop the music
    }

    initBoard(p1Board);
    initBoard(p2Board);

    while (!WindowShouldClose()) {

        float secondsThisFrame = GetFrameTime();
        Vector2 mousePos = GetMousePosition();
        int mouseClicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        UpdateMusicStream(bgmusic);//buffers without this random update tick thing. DIE C and raylib
        //Mouse input ko grid coordinates mein convert karne ke liye hai DO NOT TOUCH
        int hoverRow  = ((int)mousePos.y - BOARDTOPY)  / CELL;
        int leftCol   = ((int)mousePos.x - LBOARDX)   / CELL;
        int rightCol  = ((int)mousePos.x - RBOARDX)  / CELL;

        int mouseOnLeftBoard  = (hoverRow  >= 0 && hoverRow  < GRID&&
                                 leftCol   >= 0 && leftCol   < GRID &&
                                 mousePos.y >= BOARDTOPY &&
                                 mousePos.y <  BOARDTOPY + BOARDPIXELS &&
                                 mousePos.x >= LBOARDX &&
                                 mousePos.x <  LBOARDX + BOARDPIXELS);

        int mouseOnRightBoard = (hoverRow  >= 0 && hoverRow  < GRID &&
                                 rightCol  >= 0 && rightCol  < GRID &&
                                 mousePos.y >= BOARDTOPY &&
                                 mousePos.y <  BOARDTOPY + BOARDPIXELS &&
                                 mousePos.x >= RBOARDX &&
                                 mousePos.x <  RBOARDX + BOARDPIXELS);

        if (IsKeyPressed(KEY_R)) orientation = !orientation;

        //state 0 (main menu)
        if (gameState == 0 && mouseClicked) {
            int buttonY = WINDOWHEIGHT/2 - 25;
            if (mousePos.y > buttonY && mousePos.y < buttonY + 50) {
                // IF (Button pressed){
                //   gamestate = 1;
                //   twoplayermode = 1 or 0 depending on button
                //}
                if (mousePos.x > WINDOWWIDTH/2 - 175 && mousePos.x < WINDOWWIDTH/2 - 25) {
                    twoplayermode = 1;
                    gameState     = 1;
                }
                if (mousePos.x > WINDOWWIDTH/2 + 25 && mousePos.x < WINDOWWIDTH/2 + 175) {
                    twoplayermode = 0;
                    gameState     = 1;
                }
            }
        }

        // State 1 and 2 (ship placement)
        if ((gameState == 1 || gameState == 2) && mouseClicked) {
            if (mouseOnLeftBoard) {
                char (*activeBoard)[GRID] = (gameState == 1)? p1Board : p2Board;
                int *activeCellCount = (gameState == 1)? &p1ShipsRemaining: &p2ShipsRemaining;
                int shipLength = fleetSizes[nextShip];

                if (IsValidPlacement(activeBoard, hoverRow, leftCol, shipLength, orientation)) {
                    placeShipOnBoard(activeBoard, hoverRow, leftCol,shipLength, orientation, activeCellCount);
                    nextShip++;

                    if (nextShip == 3) {
                            nextShip = 0;
                            orientation = 0;

                        if (gameState == 1) {
                            if (twoplayermode) {
                                gameState = 2;
                            } else {
                                for (int shipIdx = 0; shipIdx < 3; shipIdx++) {
                                    int placed = 0;
                                    
                                    while (!placed) {
                                        int randomRow  = rand() % GRID;
                                        int randomCol  = rand() % GRID;
                                        int randomDir  = rand() % 2;
                                        int len        = fleetSizes[shipIdx];
                                        if (IsValidPlacement(p2Board, randomRow,randomCol, len, randomDir)) {
                                            placeShipOnBoard(p2Board, randomRow,randomCol, len, randomDir,&p2ShipsRemaining);
                                            placed = 1;
                                        }
                                    }
                                }
                                Turn = 1;
                                gameState = 3;
                            }
                        } 
                        else {
                            
                            Turn = 1;
                            gameState = 3;
                        }
                    }
                }
            }
        }

      
        // State 3 (WE CAN FINNALY PLAY THE GAME YAY)
        if (gameState == 3) {

            if (waitingForAI) {
                aiTimer -= secondsThisFrame;
                if (aiTimer <= 0.0f) {
                    waitingForAI = 0;
                    AIShoot();
                    lastShotreport = (p1Board[lastShotRow][lastShotCol] == 'X');
                }
            }

            if (!waitingForAI && mouseClicked && mouseOnRightBoard) {

                if (twoplayermode) {

                    char (*enemyBoard)[GRID] = (Turn == 1)? p2Board : p1Board;
                    int *enemyFleetSize = (Turn == 1)? &p2ShipsRemaining : &p1ShipsRemaining;
                    int *myTotalShots = (Turn == 1)? &p1ShotsFired : &p2ShotsFired;

                    if (enemyBoard[hoverRow][rightCol] != 'X' && enemyBoard[hoverRow][rightCol] != 'O'){

                        lastShotreport = shoot(enemyBoard, hoverRow, rightCol, enemyFleetSize);
                        lastShotRow = hoverRow;
                        lastShotCol = rightCol;
                        (*myTotalShots)++;

                        
                        Turn = (Turn == 1) ? 2 : 1;
                        switchscreen = 0;  
                        gameState = 5;
                    }

                } else {

                    if (p2Board[hoverRow][rightCol] != 'X' && p2Board[hoverRow][rightCol] != 'O') {

                        lastShotreport = shoot(p2Board, hoverRow, rightCol, &p2ShipsRemaining);
                        lastShotRow = hoverRow;
                        lastShotCol = rightCol;
                        p1ShotsFired++;

                        //AI delay
                        if (p2ShipsRemaining > 0) {
                            waitingForAI     = 1;
                            aiTimer = AIDELAY;
                        }
                    }
                }
            }
        }


        // state 5 SwitchScreen 
        if (gameState == 5 && mouseClicked) {
            if (!switchscreen) {
                switchscreen = 1;
            } else {
                switchscreen = 0;
                lastShotRow  = -1; 
                lastShotCol  = -1;
                gameState    = 3;
            }
        }

        // Win check
        if (gameState == 3 && (p1ShipsRemaining == 0 || p2ShipsRemaining == 0)) gameState = 4;

        // drawing the ui and everything (this is gonna be a mess TT)
        
        BeginDrawing();
        ClearBackground(BACKGROUND);

        // menu 
        if (gameState == 0) {
            // title splash
            const char *gameTitle = "BATTLESHIP";
            DrawText(gameTitle,WINDOWWIDTH/2 - MeasureText(gameTitle, 52)/2,WINDOWHEIGHT/2 - 120, 52, ACCENT);
            const char *titlesplash = "By : Jarar Khan, Mool Bhoomika and Ahad Kashif";
            DrawText(titlesplash,WINDOWWIDTH/2 - MeasureText(titlesplash, 32)/2,WINDOWHEIGHT/2 - 60, 32, PINK);

            int buttonY = WINDOWHEIGHT/2;// pushed it down by 25 more pixels (removed the - 25 y)

            //buttons 
            DrawRectangle(WINDOWWIDTH/2 - 175, buttonY, 150, 50, PANELBACKGROUND);
            DrawRectangleLines(WINDOWWIDTH/2 - 175, buttonY, 150, 50, ACCENT);
            DrawText("PASS & PLAY",WINDOWWIDTH/2 - 175 + 75 - MeasureText("PASS & PLAY", 16)/2,buttonY + 17, 16, TEXT);

            DrawRectangle(WINDOWWIDTH/2 + 25, buttonY, 150, 50, PANELBACKGROUND);
            DrawRectangleLines(WINDOWWIDTH/2 + 25, buttonY, 150, 50, ACCENT);
            DrawText("VS AI",WINDOWWIDTH/2 + 25 + 75 - MeasureText("VS AI", 16)/2,buttonY + 17, 16, TEXT);

            const char *subText = "Choose a game mode";
            DrawText(subText,WINDOWWIDTH/2 - MeasureText(subText, 16)/2,WINDOWHEIGHT/2 + 80, 16, TEXT);
            const char *subsubText = "This is a semi faithfull recreaton albiet without many graphical or sound elements as of now ~ the dev team";
            DrawText(subsubText,WINDOWWIDTH/2 - MeasureText(subsubText, 16)/2,WINDOWHEIGHT/2 + 200, 16, TEXT);
        }

        // SHIP PLACEMENT (states 1 & 2) 
        if (gameState == 1 || gameState == 2) {

            char (*boardBeingSetUp)[GRID] = (gameState == 1) ? p1Board : p2Board;

            drawBoard(boardBeingSetUp, LBOARDX, BOARDTOPY, 0);

            if (mouseOnLeftBoard) drawPlacementGhost(boardBeingSetUp, LBOARDX, BOARDTOPY ,hoverRow, leftCol,fleetSizes[nextShip], orientation);

            const char *placementHeading = (gameState == 1) ? "PLAYER 1 : Place your ships" : "PLAYER 2 : Place your ships";
            DrawText(placementHeading, LBOARDX, BOARDTOPY - 22, 16, WARNING);

            // ships ky list
            int checklistX = RBOARDX;
            int checklistY = BOARDTOPY;
            DrawText("Ships to place:", checklistX, checklistY, 15, ACCENT);

            const char *shipNames[] = {
                "Monkey Buccaneer  (5)",
                "Bismark  (4)",
                "U-Boat  (3)"
            };
            
            for (int shipIdx = 0; shipIdx < 3; shipIdx++) {
                Color lineColor = (shipIdx < nextShip) ? SPLASH: (shipIdx == nextShip) ? WARNING:TEXT;
                const char *marker = (shipIdx < nextShip) ? "[X] ": (shipIdx == nextShip) ? "[>] " : "[ ] ";
                char checkLine[32];
                sprintf(checkLine, "%s%s", marker, shipNames[shipIdx]);
                DrawText(checkLine, checklistX,checklistY + 24 + shipIdx * 22, 15, lineColor);
            }

            char bottomHint[80];
            sprintf(bottomHint, "Size: %d   |   R = rotate   |   %s", fleetSizes[nextShip], orientation ? "VERTICAL" : "HORIZONTAL");
            DrawText(bottomHint, LBOARDX, BOARDTOPY + BOARDPIXELS + 8, 14, TEXT);
        }

        // battle screen (state 3)
        if (gameState == 3) {
            char (*myFleet)[GRID] = twoplayermode ? ((Turn==1) ? p1Board : p2Board) : p1Board;
            char (*enemyFleet)[GRID] = twoplayermode ? ((Turn==1) ? p2Board : p1Board) : p2Board;

            const char *myLabel    = (Turn == 1) ? "P1 : YOUR FLEET" : "P2 : YOUR FLEET";
            const char *enemyLabel = (Turn == 1) ? (twoplayermode ? "P2 : ENEMY" : "AI : ENEMY") : "P1 : ENEMY";

            DrawText(myLabel,    LBOARDX,  BOARDTOPY - 22, 15, ACCENT);
            DrawText(enemyLabel, RBOARDX, BOARDTOPY - 22, 15, DANGER);

            drawBoard(myFleet,    LBOARDX,  BOARDTOPY, 0);
            drawBoard(enemyFleet, RBOARDX, BOARDTOPY, 1);

            if (!twoplayermode && lastShotRow >= 0) {
                drawLastShot(LBOARDX, BOARDTOPY);
            }

            // Aim reticle on the right board when hovering
            if (!waitingForAI && mouseOnRightBoard &&
                (enemyFleet[hoverRow][rightCol] == '~' ||
                 enemyFleet[hoverRow][rightCol] == 'S')) {
                DrawRectangle(RBOARDX + rightCol * CELL,BOARDTOPY + hoverRow * CELL, CELL, CELL, AIM);
            }

            // AI thinking overlay
            if (waitingForAI) {
                const char *thinkingText = "AI is thinking...";
                DrawText(thinkingText, WINDOWWIDTH/2 - MeasureText(thinkingText, 18)/2,BOARDTOPY + BOARDPIXELS + 8, 18, WARNING);
            }

            drawStatsPanel();

            if (!waitingForAI) {
                const char *fireHint = "Click the RIGHT board to fire";
                DrawText(fireHint, WINDOWWIDTH/2 - MeasureText(fireHint, 14)/2,BOARDTOPY + BOARDPIXELS + 8, 14, TEXT);
            }
        }

        // switch screen 
        if (gameState == 5) {
            // hides boards
            DrawRectangle(0, 0, WINDOWWIDTH, WINDOWHEIGHT, BACKGROUND);

            if (!switchscreen) {

                char handHeading[48];
                sprintf(handHeading, "PLAYER %d's TURN", Turn);
                DrawText(handHeading,WINDOWWIDTH/2 - MeasureText(handHeading, 40)/2,WINDOWHEIGHT/2 - 80, 40, ACCENT);

                const char *handMsg = "Hand the device to the next player";
                DrawText(handMsg,WINDOWWIDTH/2 - MeasureText(handMsg, 18)/2,WINDOWHEIGHT/2 - 10, 18, TEXT);

                const char *revealPrompt = "Click to reveal the shot result";
                DrawText(revealPrompt,WINDOWWIDTH/2 - MeasureText(revealPrompt, 18)/2, WINDOWHEIGHT/2 + 40, 18, WARNING);

            } else {

                const char *resultText  = lastShotreport ? "DIRECT HIT!"  : "MISS!";
                Color resultColor = lastShotreport ? EXPLOSION : SPLASH;
                DrawText(resultText,WINDOWWIDTH/2 - MeasureText(resultText, 48)/2, WINDOWHEIGHT/2 - 110, 48, resultColor);

                DrawRectangle(WINDOWWIDTH/4, WINDOWHEIGHT/2 - 48,WINDOWWIDTH/2, 2, PANELBORDERCOLOR);

                char whoIsNext[48];
                sprintf(whoIsNext, "PLAYER %d's TURN", Turn);
                DrawText(whoIsNext, WINDOWWIDTH/2 - MeasureText(whoIsNext, 32)/2, WINDOWHEIGHT/2 - 36, 32, ACCENT);

                const char *continuePrompt = "Click anywhere to start your turn";
                DrawText(continuePrompt,WINDOWWIDTH/2 - MeasureText(continuePrompt, 17)/2,WINDOWHEIGHT/2 + 30, 17, WARNING);
            }
        }

        // Game Over
        if (gameState == 4) {
            int winnerNumber = (p1ShipsRemaining == 0) ? 2 : 1;
            char victoryText[32];
            sprintf(victoryText, "PLAYER %d WINS!", winnerNumber);
            Color victoryColor = (winnerNumber == 1) ? VICTORY : DANGER;
            DrawText(victoryText,WINDOWWIDTH/2 - MeasureText(victoryText, 48)/2,WINDOWHEIGHT/2 - 100, 48, victoryColor);

            char player1Summary[56], player2Summary[56];
            sprintf(player1Summary, "P1  :  Ships left: %d   Shots fired: %d",p1ShipsRemaining, p1ShotsFired);
            sprintf(player2Summary, "%s  :  Ships left: %d   Shots fired: %d",twoplayermode ? "P2" : "AI",p2ShipsRemaining, p2ShotsFired);
            DrawText(player1Summary,WINDOWWIDTH/2 - MeasureText(player1Summary, 17)/2,WINDOWHEIGHT/2 - 28, 17, TEXT);
            DrawText(player2Summary,WINDOWWIDTH/2 - MeasureText(player2Summary, 17)/2,WINDOWHEIGHT/2 + 5,  17, TEXT);

            int restartButtonX = WINDOWWIDTH/2  - 100;
            int restartButtonY = WINDOWHEIGHT/2 + 50;

            DrawRectangle(restartButtonX, restartButtonY, 200, 50, PANELBACKGROUND);
            DrawRectangleLines(restartButtonX, restartButtonY, 200, 50, ACCENT);
            DrawText("RESTART",restartButtonX + 100 - MeasureText("RESTART", 20)/2,restartButtonY + 15, 20,TEXT);

            if (mouseClicked && mousePos.x > restartButtonX && mousePos.x < restartButtonX + 200 && mousePos.y > restartButtonY && mousePos.y < restartButtonY + 50){
                fullReset();
            }
        }

        EndDrawing();
    }

    UnloadMusicStream(bgmusic); // again music thingy
    CloseAudioDevice();

    CloseWindow();
    return 0;
}