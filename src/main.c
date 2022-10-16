/**
 * Tic-Tac-Toe
 * Megadrive / Genesis version of tic tac toe
 **/
#include <genesis.h>
#include <string.h>
#include <resources.h>

enum GAME_STATE { MAIN_MENU, CREDITS, SELECT_OPPONENT, SELECT_PLAYER2_INPUT, GAME_PLAYING, GAME_RESULT, PAUSE_MENU };
enum SQUARE_STATE { EMPTY, NOUGHT, CROSS };
enum OPPONENT_TYPE { PLAYER_2, CPU_EASY, CPU_MEDIUM, CPU_HARD };

const int MAIN_MENU_1_PLAY = 0;
const int MAIN_MENU_2_PLAY = 1;
const int MAIN_MENU_CREDITS = 2;

const int TOP_LEFT = 0;
const int TOP_MIDDLE = 1;
const int TOP_RIGHT = 2;
const int CENTER_LEFT = 3;
const int CENTER_MIDDLE = 4;
const int CENTER_RIGHT = 5;
const int BOTTOM_LEFT = 6;
const int BOTTOM_MIDDLE = 7;
const int BOTTOM_RIGHT = 8;

const int winning_lines[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

// current state
enum GAME_STATE current_game_state = MAIN_MENU;
int selected_menu_option;
u8 moves_remaining;
u8 selected_square;
enum OPPONENT_TYPE game_opponent = CPU_EASY;
enum SQUARE_STATE game_board[9];
enum SQUARE_STATE current_player = CROSS;
u16 player2_pad;
int moves_history[9];

// player scores
u32 games_played = 0;
u32 games_won = 0;
u32 games_lost = 0;

// gfx
u16 square_basetile;
u16 cross_basetile;
u16 nought_basetile;
u16 title_basetile;
u16 bg_basetile;
u16 border_basetile;
u16 crossbg_basetile;
u16 noughtbg_basetile;
Sprite* user_cursor[4];
Sprite* pause_sprite[2];

// background scrolling
int scroll_offset_x = 0;
int scroll_offset_y = 0;

// system startup
void loadTiles();
void loadPallete();
void loadSprites();

// general
int randomRange(int min, int max);
bool randTest(int percentage);
void scrollBackground();
void handleJoypadInput(u16 joy, u16 changed, u16 state);

// main menu handling
void startMainMenu();
void updateMainMenu();
void clearMainMenu();
void handleMenuInput(u16 joy, u16 changed, u16 state);

// credits
void startCredits();
void clearCredits();
void handleCreditsInput(u16 joy, u16 changed, u16 state);

// select opponent
void startSelectOpponentMenu();
void updateSelectOpponentMenu();
void clearSelectOpponentMenu();
void handleSelectOpponentInput(u16 joy, u16 changed, u16 state);

// select player 2 input
void StartSelectPlayer2Menu();
void UpdateSelectPlayer2Menu();
void clearSelectPlayer2Menu();
void handleSelectPlayer2Input(u16 joy, u16 changed, u16 state);

// save state
void loadScores(enum OPPONENT_TYPE opponent);
void saveScores(enum OPPONENT_TYPE opponent);

// game handling
void startGame(enum OPPONENT_TYPE opponent);
void closeGame();
void setBackground();
void setForeground();
void setUserCursorVisibility(SpriteVisibility value);
void drawGameScores();
void drawGameBoardSquare(int square, enum SQUARE_STATE state);
void takeSquare(int squareindex);
void resetGame();
void setSelectedSquare(int idx);
void handleGameInput(u16 joy, u16 changed, u16 state);

// game result screen
void showGameResult();
void handleGameResultInput(u16 joy, u16 changed, u16 state);

// pause menu
void startPauseMenu();
void hidePauseMenu();
void setPauseMenuVisibility(SpriteVisibility value);
void handlePauseMenuInput(u16 joy, u16 changed, u16 state);

// game logic
bool isCorner(int idx);
bool isSide(int idx);
bool isMiddle(int idx);
bool isAdjacentCorner(int idx, int cornerIdx);
bool isOppositeCorner(int idx, int cornerIdx);
int countTilesOnWinLine(int line_index, enum SQUARE_STATE state);

bool testWin();

int randomMove();
bool takeWin(int* squareIndex, enum SQUARE_STATE player);
bool stopLoss(int* squareIndex, enum SQUARE_STATE player);
bool addToWinnableLine(int* squareIndex, enum SQUARE_STATE player);
bool blockLoseableLine(int* squareIndex, enum SQUARE_STATE player);
int cpuTurn();

/////////////////////////////////////////////////////////////////////////////////////
// system startup
/////////////////////////////////////////////////////////////////////////////////////

void loadTiles()
{
    VDP_loadFont(&tileset_font, DMA);

    title_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_title.tileset, title_basetile, DMA);
    curTileInd += img_title.tileset->numTile;

    square_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_empty.tileset, square_basetile, DMA);
    curTileInd += img_empty.tileset->numTile;
    
    cross_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_cross.tileset, cross_basetile, DMA);
    curTileInd += img_cross.tileset->numTile;
    
    nought_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_nought.tileset, nought_basetile, DMA);
    curTileInd += img_nought.tileset->numTile;

    bg_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_bg.tileset, bg_basetile, DMA);
    curTileInd += img_bg.tileset->numTile;

    border_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(&tileset_border, border_basetile, DMA);
    curTileInd += tileset_border.numTile;

    crossbg_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_crossbg.tileset, crossbg_basetile, DMA);
    curTileInd += img_crossbg.tileset->numTile;

    noughtbg_basetile = TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, curTileInd);
    VDP_loadTileSet(img_noughtbg.tileset, noughtbg_basetile, DMA);
    curTileInd += img_noughtbg.tileset->numTile;
}

void loadPallete()
{
    VDP_setPalette(PAL0, pal_pico8.data);
    VDP_setPalette(PAL1, pal_pico8.data);
    VDP_setPalette(PAL2, pal_pico8.data);
    VDP_setPalette(PAL3, pal_pico8.data);
    // Set background
    VDP_setPaletteColor(0, PAL_getColor(1));
    // Set text colour
    //VDP_setPaletteColor(15, RGB24_TO_VDPCOLOR(0x000000));
}

void loadSprites()
{
    SPR_init();
    // create the user cursor spite(s)
    user_cursor[0] = SPR_addSprite(&cursor, 0, 0, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    user_cursor[1] = SPR_addSprite(&cursor, 32, 0, TILE_ATTR(PAL1, 0, FALSE, TRUE));
    user_cursor[2] = SPR_addSprite(&cursor, 0, 32, TILE_ATTR(PAL1, 0, TRUE, FALSE));
    user_cursor[3] = SPR_addSprite(&cursor, 32, 32, TILE_ATTR(PAL1, 0, TRUE, TRUE));

    // hide the user cursor
    setUserCursorVisibility(HIDDEN);

    // pause menu
    pause_sprite[0] = SPR_addSprite(&spr_pause1, 132, 100, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    pause_sprite[1] = SPR_addSprite(&spr_pause2, 164, 100, TILE_ATTR(PAL1, 1, FALSE, FALSE));
    setPauseMenuVisibility(HIDDEN);
}


/////////////////////////////////////////////////////////////////////////////////////
// general
/////////////////////////////////////////////////////////////////////////////////////

int randomRange(int min, int max)
{
    return (random() % (max - min + 1)) + min;
}

bool randTest(int percentage)
{
    return randomRange(0, 100) < percentage;
}

void scrollBackground()
{
    if ((current_game_state != GAME_PLAYING) && (current_game_state != GAME_RESULT)) return;

    VDP_setVerticalScroll(BG_B, scroll_offset_y--);
    VDP_setHorizontalScroll(BG_B, scroll_offset_x--);

    if (scroll_offset_y <= -4096) scroll_offset_y = 0;
    if (scroll_offset_x <= -4096) scroll_offset_x = 0;
}

void handleJoypadInput(u16 joy, u16 changed, u16 state)
{
    switch (current_game_state)
    {
        case MAIN_MENU:
            handleMenuInput(joy, changed, state);
            break;
        case CREDITS:
            handleCreditsInput(joy, changed, state);
            break;
        case SELECT_OPPONENT:
            handleSelectOpponentInput(joy, changed, state);
            break;
        case SELECT_PLAYER2_INPUT:
            handleSelectPlayer2Input(joy, changed, state);
            break;
        case GAME_PLAYING:
            handleGameInput(joy, changed, state);
            break;
        case GAME_RESULT:
            handleGameResultInput(joy, changed, state);
            break;
        case PAUSE_MENU:
            handlePauseMenuInput(joy, changed, state);
            break;
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// main menu handling
/////////////////////////////////////////////////////////////////////////////////////

void startMainMenu()
{
    current_game_state = MAIN_MENU;
    selected_menu_option = MAIN_MENU_1_PLAY;

    // draw the logo image
    VDP_setTileMapEx(BG_A, img_title.tilemap, title_basetile, (40 - img_title.tilemap->w) / 2, 5, 0, 0, img_title.tilemap->w, img_title.tilemap->h, CPU);

    // draw the menu items
    VDP_drawText("1 PLAYER", 16, 16);
    VDP_drawText("2 PLAYERS", 16, 18);
    VDP_drawText("CREDITS", 16, 20);

    // draw the current selections
    updateMainMenu();
}

void updateMainMenu()
{
    // clear previous selections
    VDP_clearText(14, 16, 1);
    VDP_clearText(14, 18, 1);
    VDP_clearText(14, 20, 1);
    // show marker for current selection
    if (selected_menu_option == MAIN_MENU_1_PLAY) VDP_drawText(">", 14, 16);
    if (selected_menu_option == MAIN_MENU_2_PLAY) VDP_drawText(">", 14, 18);
    if (selected_menu_option == MAIN_MENU_CREDITS) VDP_drawText(">", 14, 20);
}

void clearMainMenu()
{
    VDP_clearPlane(BG_A, TRUE);
}

void handleMenuInput(u16 joy, u16 changed, u16 state)
{
    // player has selected an option
    if (state & changed & BUTTON_A)
    {
        // clear the menu
        clearMainMenu();
        
        // player 1 select opponent
        if (selected_menu_option == MAIN_MENU_1_PLAY)
        {
            startSelectOpponentMenu();
            return;
        }
        
        // start 2 player game
        if (selected_menu_option == MAIN_MENU_2_PLAY)
        {
            StartSelectPlayer2Menu(PLAYER_2);
            return;
        }
        
        // credits menu
        if (selected_menu_option == MAIN_MENU_CREDITS)
        {
            startCredits();
            return;
        }
    }

    if ((state & changed & BUTTON_UP) && (selected_menu_option > MAIN_MENU_1_PLAY)) selected_menu_option--;
    if ((state & changed & BUTTON_DOWN) && (selected_menu_option < MAIN_MENU_CREDITS)) selected_menu_option++;

    updateMainMenu();
    return;
}


/////////////////////////////////////////////////////////////////////////////////////
// credits
/////////////////////////////////////////////////////////////////////////////////////

void startCredits()
{
    current_game_state = CREDITS;

    // draw the logo image
    VDP_setTileMapEx(BG_A, img_title.tilemap, title_basetile, 0, 0, 0, 0, img_title.tilemap->w, img_title.tilemap->h, CPU);

    // draw the credits
    VDP_drawText("PROGRAMMING", 1, 8);
    VDP_drawText("PAUL JACKSON", 3, 9);
    VDP_drawText("MADE WITH SGDK BY", 1, 14);
    VDP_drawText("STEPHANE DALLONGEVILLE", 3, 15);

    VDP_drawText("GET THE SOURCE CODE AT", 1, 20);
    VDP_drawText("GITHUB.COM/MANDRILLBOY/TICTACTOEMD", 3, 21);

    VDP_drawText("RELEASED AS FREEWARE", 1, 25);
    VDP_drawText("FEEL FREE TO SHARE/MAKE COPIES", 1, 26);
}

void clearCredits()
{
    VDP_clearPlane(BG_A, TRUE);
}

void handleCreditsInput(u16 joy, u16 changed, u16 state)
{
    if ((state & changed & BUTTON_A) || (state & changed & BUTTON_B) || (state & changed & BUTTON_C) || (state & changed & BUTTON_START))
    {
        clearCredits();
        startMainMenu();
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// select opponent
/////////////////////////////////////////////////////////////////////////////////////

void startSelectOpponentMenu()
{
    current_game_state = SELECT_OPPONENT;
    game_opponent = CPU_EASY;

    // draw title image
    VDP_setTileMapEx(BG_A, img_title.tilemap, title_basetile, (40 - img_title.tilemap->w) / 2, 5, 0, 0, img_title.tilemap->w, img_title.tilemap->h, CPU);

    // draw the menu items
    VDP_drawText("SELECT DIFFICULTY", 10, 14);
    VDP_drawText("EASY", 16, 17);
    VDP_drawText("MEDIUM", 16, 19);
    VDP_drawText("HARD", 16, 21);

    // draw the current selections
    updateSelectOpponentMenu();
}

void updateSelectOpponentMenu()
{
    // clear previous selections
    VDP_clearText(14, 17, 1);
    VDP_clearText(14, 19, 1);
    VDP_clearText(14, 21, 1);

    // show marker for current selection
    if (game_opponent == CPU_EASY) VDP_drawText(">", 14, 17);
    if (game_opponent == CPU_MEDIUM) VDP_drawText(">", 14, 19);
    if (game_opponent == CPU_HARD) VDP_drawText(">", 14, 21);
}

void clearSelectOpponentMenu()
{
    VDP_clearPlane(BG_A, TRUE);
}

void handleSelectOpponentInput(u16 joy, u16 changed, u16 state)
{
    // player has selected an option
    if (state & changed & BUTTON_A)
    {
        // player 1 select opponent
        if (selected_menu_option == MAIN_MENU_1_PLAY)
        {
            clearSelectOpponentMenu();
            startGame(game_opponent);
            return;
        }
    }

    if (state & changed & BUTTON_C)
    {
        clearSelectOpponentMenu();
        startMainMenu();
        return;
    }

    if ((state & changed & BUTTON_UP) && (game_opponent > CPU_EASY)) game_opponent--;
    if ((state & changed & BUTTON_DOWN) && (game_opponent < CPU_HARD)) game_opponent++;

    updateSelectOpponentMenu();
}


/////////////////////////////////////////////////////////////////////////////////////
// select player 2 menu
/////////////////////////////////////////////////////////////////////////////////////

void StartSelectPlayer2Menu()
{
    current_game_state = SELECT_PLAYER2_INPUT;
    player2_pad = JOY_1;

    // draw title image
    VDP_setTileMapEx(BG_A, img_title.tilemap, title_basetile, (40 - img_title.tilemap->w) / 2, 5, 0, 0, img_title.tilemap->w, img_title.tilemap->h, CPU);

    // draw the menu items
    VDP_drawText("ASSIGN PLAYER 2", 13, 14);
    VDP_drawText("SHARE PAD 1", 16, 17);
    VDP_drawText("PAD 2", 16, 19);

    // draw the current selections
    UpdateSelectPlayer2Menu();
}

void UpdateSelectPlayer2Menu()
{
    // clear previous selections
    VDP_clearText(14, 17, 1);
    VDP_clearText(14, 19, 1);

    // show marker for current selection
    if (player2_pad == JOY_1) VDP_drawText(">", 14, 17);
    if (player2_pad == JOY_2) VDP_drawText(">", 14, 19);
}

void clearSelectPlayer2Menu()
{
    VDP_clearPlane(BG_A, TRUE);
}

void handleSelectPlayer2Input(u16 joy, u16 changed, u16 state)
{
    // player has selected an option
    if (state & changed & BUTTON_A)
    {
        clearSelectPlayer2Menu();
        startGame(PLAYER_2);
        return;
    }

    // back to main menu
    if (state & changed & BUTTON_C)
    {
        clearSelectPlayer2Menu();
        startMainMenu();
        return;
    }

    if (state & changed & BUTTON_UP) player2_pad = JOY_1;
    if (state & changed & BUTTON_DOWN) player2_pad = JOY_2;

    UpdateSelectPlayer2Menu();
    return;
}


/////////////////////////////////////////////////////////////////////////////////////
// pause menu
/////////////////////////////////////////////////////////////////////////////////////

void startPauseMenu()
{
    // pause menu mode
    selected_menu_option = 0;
    current_game_state = PAUSE_MENU;
    // Show pause menu sprites
    setPauseMenuVisibility(VISIBLE);
    SPR_setAnim(pause_sprite[0], 0);
    // Hide user icon sprite
    setUserCursorVisibility(HIDDEN);
    // Pause music
    XGM_pausePlay();
}

void hidePauseMenu()
{
    // Hide pause menu sprites
    setPauseMenuVisibility(HIDDEN);
    // Show user icon sprite
    setUserCursorVisibility(VISIBLE);
    // Resume Music
    XGM_resumePlay();
    // back to game mode
    current_game_state = GAME_PLAYING;
}

void setPauseMenuVisibility(SpriteVisibility value)
{
    for(int i=0; i<2; i++) SPR_setVisibility(pause_sprite[i], value);
}

void handlePauseMenuInput(u16 joy, u16 changed, u16 state)
{
    if ((state & changed & BUTTON_UP) || (state & changed & BUTTON_DOWN))
    {
        selected_menu_option = (selected_menu_option + 1) % 2;
        SPR_setAnim(pause_sprite[0], selected_menu_option);
        return;
    }

    if ((state & changed & BUTTON_START) || ((state & changed & BUTTON_A) && (selected_menu_option == 0)))
    {
        hidePauseMenu();
        return;
    }

    if (state & changed & BUTTON_A)
    {
        closeGame();
        startMainMenu();
        return;
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// save state
/////////////////////////////////////////////////////////////////////////////////////

void loadScores(enum OPPONENT_TYPE opponent)
{
    SRAM_enableRO();
    games_played = SRAM_readLong(sizeof(u32) * (opponent * 3 + 0));
    games_won = SRAM_readLong(sizeof(u32) * (opponent * 3 + 1));
    games_lost = SRAM_readLong(sizeof(u32) * (opponent * 3 + 2));
    SRAM_disable();
}

void saveScores(enum OPPONENT_TYPE opponent)
{
    SRAM_enable();
    SRAM_writeLong(sizeof(u32) * (opponent * 3 + 0), games_played);
    SRAM_writeLong(sizeof(u32) * (opponent * 3 + 1), games_won);
    SRAM_writeLong(sizeof(u32) * (opponent * 3 + 2), games_lost);
    SRAM_disable();
}


/////////////////////////////////////////////////////////////////////////////////////
// game handling
/////////////////////////////////////////////////////////////////////////////////////

void startGame(enum OPPONENT_TYPE opponent)
{
    // Set the opponent
    game_opponent = opponent;
    // Load scores from SRAM
    loadScores(opponent);
    // Set the game state
    current_game_state = GAME_PLAYING;
    // set the scrolling tiled background
    setBackground();
    // set the scores in the foreground
    setForeground();
    // start playing the music
    XGM_startPlay(music_bg);
    // reset the game (draws the game board & scores)
    resetGame();
}

void closeGame()
{
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    // Hide pause menu sprites
    setPauseMenuVisibility(HIDDEN);
    // Stop music
    XGM_stopPlay();
}

void setBackground()
{
    // draw background image
    int w = 64 / img_bg.tilemap->w;
    int h = 32 / img_bg.tilemap->h;

    for(int i=0; i<w; i++) {
        for(int j=0; j<h; j++) {
            VDP_setTileMapEx(BG_B, img_bg.tilemap, bg_basetile, i * img_bg.tilemap->w, j * img_bg.tilemap->h, 0, 0, img_bg.tilemap->w, img_bg.tilemap->h, CPU);
        }
    }
}

void setForeground()
{
    // Draw the corners of the game square
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, border_basetile), 0, 0);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, TRUE, border_basetile), 27, 0);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, TRUE, FALSE, border_basetile), 0, 27);
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, TRUE, TRUE, border_basetile), 27, 27);

    // draw the border around the game square
    for(u16 i=1; i<27; i++)
    {
        VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, border_basetile+1), i, 0);
        VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, TRUE, FALSE, border_basetile+1), i, 27);
        VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, border_basetile+2), 0, i);
        VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, TRUE, border_basetile+2), 27, i);
    }

    // fill the score panel on the right of the screen
    for(u16 x=28; x<40; x++)
    {
        for(u16 y=0; y<28; y++)
        {
            VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL1, 0, FALSE, FALSE, border_basetile+3), x, y);
        }
    }

    // draw the player images
    VDP_setTileMapEx(BG_A, img_crossbg.tilemap, crossbg_basetile, 30, 1, 0, 0, img_crossbg.tilemap->w, img_crossbg.tilemap->h, DMA);
    VDP_setTileMapEx(BG_A, img_noughtbg.tilemap, noughtbg_basetile, 30, 18, 0, 0, img_noughtbg.tilemap->w, img_noughtbg.tilemap->h, DMA);
}

void setUserCursorVisibility(SpriteVisibility value)
{
    for(int i=0; i<4; i++) SPR_setVisibility(user_cursor[i], value);
}

void drawGameScores()
{
    // stats are from point of view of opponent
    char s[8];
    intToStr(games_lost, s, 8);
    VDP_drawText(s, 30, 9);

    intToStr(games_played - games_won - games_lost, s, 8);
    VDP_drawText("Draws:", 30, 13);
    VDP_drawText(s, 30, 15);

    intToStr(games_won, s, 8);
    VDP_drawText(s, 30, 26);
}

void drawGameBoardSquare(int square, enum SQUARE_STATE state)
{
    u16 x = (square % 3 * 9) + 1;
    u16 y = (square / 3 * 9) + 1;
    
    if (state == EMPTY) VDP_setTileMapEx(BG_A, img_empty.tilemap, square_basetile, x, y, 0, 0, img_empty.tilemap->w, img_empty.tilemap->h, CPU);
    if (state == CROSS) VDP_setTileMapEx(BG_A, img_cross.tilemap, cross_basetile, x, y, 0, 0, img_cross.tilemap->w, img_cross.tilemap->h, CPU);
    if (state == NOUGHT) VDP_setTileMapEx(BG_A, img_nought.tilemap, nought_basetile, x, y, 0, 0, img_nought.tilemap->w, img_nought.tilemap->h, CPU);
}

void takeSquare(int squareindex)
{
    // check square is available
    if (game_board[squareindex] != EMPTY) return;

    // take the square
    game_board[squareindex] = current_player;
    moves_history[9-moves_remaining] = squareindex;
    moves_remaining--;

    // draw the square
    drawGameBoardSquare(squareindex, current_player);

    // check for win
    if (testWin() == TRUE)
    {
        if (current_player == CROSS) games_lost++;
        if (current_player == NOUGHT) games_won++;
        moves_remaining = 0;
    }

    // next player
    current_player = current_player == CROSS ? NOUGHT : CROSS;

    // end of game
    if (moves_remaining == 0)
    {
        games_played++;
        showGameResult();
        return;
    }
    
    // Take CPU turn
    if ((game_opponent != PLAYER_2) && (current_player == NOUGHT)) takeSquare(cpuTurn());
}

void resetGame()
{
    // show the cursor sprites
    setUserCursorVisibility(VISIBLE);

    // empty all squares on the game board
    for(int i=0; i<9; i++) {
        drawGameBoardSquare(i, EMPTY);
        game_board[i] = EMPTY;
    }

    // draw the game scores    
    drawGameScores();

    // reset game flags
    moves_remaining = 9;
    setSelectedSquare(4);

    // If it is the CPU's turn - take it
    if ((game_opponent != PLAYER_2) && (current_player == NOUGHT)) takeSquare(cpuTurn());
}

void setSelectedSquare(int idx)
{
    selected_square = idx;
    // move the user cursor sprites
    SPR_setPosition(user_cursor[0], (selected_square % 3 * 72) + 8, (selected_square / 3 * 72) + 8);
    SPR_setPosition(user_cursor[1], (selected_square % 3 * 72) + 40, (selected_square / 3 * 72) + 8);
    SPR_setPosition(user_cursor[2], (selected_square % 3 * 72) + 8, (selected_square / 3 * 72) + 40);
    SPR_setPosition(user_cursor[3], (selected_square % 3 * 72) + 40, (selected_square / 3 * 72) + 40);
}

void handleGameInput(u16 joy, u16 changed, u16 state)
{
    // pause (any pad)
    if (state & changed & BUTTON_START)
    {
        startPauseMenu();
        return;
    }

    // is this pad is the player for the current turn is using
	if (joy != (current_player == NOUGHT) ? player2_pad : JOY_1) return;

	if (state & changed & BUTTON_RIGHT) setSelectedSquare((selected_square / 3 * 3) + ((selected_square + 1) % 3));
    if (state & changed & BUTTON_LEFT) setSelectedSquare((selected_square / 3 * 3) + ((selected_square + 2) % 3));
    if (state & changed & BUTTON_UP) setSelectedSquare((selected_square + 6) % 9);
    if (state & changed & BUTTON_DOWN) setSelectedSquare((selected_square + 3) % 9);
    if (state & changed & BUTTON_A) takeSquare(selected_square);
}


/////////////////////////////////////////////////////////////////////////////////////
// game result screen
/////////////////////////////////////////////////////////////////////////////////////

void showGameResult()
{
    current_game_state = GAME_RESULT;
    // hide the cursor sprites
    setUserCursorVisibility(HIDDEN); 
    // Save scores
    saveScores(game_opponent);
    // Update the scores
    drawGameScores();
}

void handleGameResultInput(u16 joy, u16 changed, u16 state)
{
    if (state & changed & BUTTON_A) 
    {
        current_game_state = GAME_PLAYING;
        resetGame();
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// game logic
/////////////////////////////////////////////////////////////////////////////////////

bool isCorner(int idx)
{
    return ((idx == TOP_LEFT) || (idx == TOP_RIGHT) || (idx == BOTTOM_LEFT) || (idx == BOTTOM_RIGHT));
}

bool isSide(int idx)
{
    return ((idx == TOP_MIDDLE) || (idx == CENTER_LEFT) || (idx == CENTER_RIGHT) || (idx == BOTTOM_MIDDLE));
}

bool isMiddle(int idx)
{
    return (idx == CENTER_MIDDLE);
}

bool isAdjacentCorner(int idx, int cornerIdx)
{
    return ((idx == TOP_LEFT) && (cornerIdx == TOP_RIGHT || cornerIdx == BOTTOM_LEFT)) ||
            ((idx == TOP_RIGHT) && (cornerIdx == TOP_LEFT || cornerIdx == BOTTOM_RIGHT)) ||
            ((idx == BOTTOM_RIGHT) && (cornerIdx == TOP_RIGHT || cornerIdx == BOTTOM_LEFT)) ||
            ((idx == BOTTOM_LEFT) && (cornerIdx == TOP_LEFT || cornerIdx == BOTTOM_RIGHT));
}

bool isOppositeCorner(int idx, int cornerIdx)
{
    if (idx == TOP_LEFT) return BOTTOM_RIGHT;
    if (idx == TOP_RIGHT) return BOTTOM_LEFT;
    if (idx == BOTTOM_LEFT) return TOP_RIGHT;
    if (idx == BOTTOM_RIGHT) return TOP_LEFT;
    // should not get here
    return idx;
}

int countTilesOnWinLine(int line_index, enum SQUARE_STATE state)
{
    int res = 0;
    for (int i=0; i<3; i++)
    {
        if (game_board[winning_lines[line_index][i]] == state) res++;
    }
    return res;
}

int randomMove()
{
    int r = randomRange(0, moves_remaining-1);
    for (int i=0; i<9; i++)
    {
        if (game_board[i] == EMPTY)
        {
            if (r == 0) return i;
            r--;
        }
    }

    // we should never get here
    return 0;
}

bool takeWin(int* squareIndex, enum SQUARE_STATE player)
{
    for (int line_index=0; line_index<8; line_index++)
    {
        if ((countTilesOnWinLine(line_index, player) == 2) && (countTilesOnWinLine(line_index, EMPTY) == 1))
        {
            for (int i=0; i<3; i++)
            {
                if (game_board[winning_lines[line_index][i]] == EMPTY)
                {
                    *squareIndex = winning_lines[line_index][i];
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

bool stopLoss(int* squareIndex, enum SQUARE_STATE player)
{
    // same logic as takeWin, but from the point of view of the other player
    return takeWin(squareIndex, player == NOUGHT ? CROSS : NOUGHT);
}

bool addToWinnableLine(int* squareIndex, enum SQUARE_STATE player)
{
    int values[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    // calculate the number of winnable lines for each square
    for (int line_index=0; line_index<8; line_index++)
    {
        if ((countTilesOnWinLine(line_index, player) == 1) && (countTilesOnWinLine(line_index, EMPTY) == 2))
        {
            for (int i=0; i<3; i++)
            {
                if (game_board[winning_lines[line_index][i]] == EMPTY) values[winning_lines[line_index][i]]++;
            }
        }
    }

    // return the square with the largest value
    *squareIndex = 0;
    for(int i=1; i<9; i++)
    {
        if (values[i] > values[*squareIndex])
        {
            *squareIndex = i;
        }
    }

    // return TRUE if the maximum square had at least 1 winnable line
    return (values[*squareIndex] > 0);
}

bool blockLoseableLine(int* squareIndex, enum SQUARE_STATE player)
{
    // same logic as addToWinnableLine, but from the point of view of the other player
    return addToWinnableLine(squareIndex, player == NOUGHT ? CROSS : NOUGHT);
}

int cpuTurn()
{
    int squareIdx;

    if (game_opponent == CPU_EASY)
    {
        // take a win 9% of the time
        if (randTest(90) && (takeWin(&squareIdx, current_player))) return squareIdx;
        // Stop the other player from winning 75% of the time
        if (randTest(75) && (stopLoss(&squareIdx, current_player))) return squareIdx;
        // Add to an available winnable line 75% of the time
        if (randTest(75) && (addToWinnableLine(&squareIdx, current_player))) return squareIdx;
    }

    if (game_opponent == CPU_MEDIUM)
    {
        // 50% of the time - take center square if available (gives the player a good chance)
        if ((moves_remaining > 7) && (game_board[CENTER_MIDDLE] == EMPTY) && randTest(50)) return CENTER_MIDDLE;
        // just aggressively try to build winning lines
        if (takeWin(&squareIdx, current_player)) return squareIdx;
        if (stopLoss(&squareIdx, current_player)) return squareIdx;
        if (addToWinnableLine(&squareIdx, current_player)) return squareIdx;
        if (blockLoseableLine(&squareIdx, current_player)) return squareIdx;
    }

    if (game_opponent == CPU_HARD)
    {
        // take win if available
        if (takeWin(&squareIdx, current_player)) return squareIdx;
        
        // Block the player from winning if they can
        if (stopLoss(&squareIdx, current_player)) return squareIdx;

        // first move - take a corner
        if (moves_remaining == 9) return BOTTOM_RIGHT;

        // if opponents first move was to a side sqaure
        if ((moves_remaining == 7) && isSide(moves_history[1])) return CENTER_MIDDLE;
        if ((moves_remaining == 5) && isSide(moves_history[1])) return (game_board[CENTER_RIGHT] == EMPTY) ? TOP_RIGHT : BOTTOM_LEFT;

        // if opponents first move was to an adjacent corner
        if ((moves_remaining == 7) && isAdjacentCorner(moves_history[1], moves_history[0])) return (game_board[TOP_RIGHT] == EMPTY) ? TOP_RIGHT : BOTTOM_LEFT;
        if ((moves_remaining == 5) && isAdjacentCorner(moves_history[1], moves_history[0])) return TOP_LEFT;

        // if opponents first move was to the opposite corner
        if ((moves_remaining == 7) && (moves_history[1] == TOP_LEFT)) return TOP_RIGHT;
        if ((moves_remaining == 5) && (moves_history[1] == TOP_LEFT)) return BOTTOM_LEFT;

        // if opponents first move was to the center square
        if ((moves_remaining == 7) && (moves_history[1] == CENTER_MIDDLE)) return TOP_LEFT;
        if ((moves_remaining == 5) && (moves_history[1] == CENTER_MIDDLE) && isCorner(moves_history[3])) return (game_board[TOP_RIGHT] == EMPTY) ? TOP_RIGHT : TOP_LEFT;

        // if cpu goes second
        if (moves_remaining % 2 == 0)
        {
            // Player started on a corner
            if (isCorner(moves_history[0]))
            {
                if (moves_remaining == 8) return CENTER_MIDDLE;
                if ((moves_remaining == 6) && isOppositeCorner(moves_history[2], moves_history[0])) return BOTTOM_MIDDLE;
            }

            // Players started on a side
            if (isSide(moves_history[0]))
            {
                if (moves_remaining == 8) return CENTER_MIDDLE;
            }
        }
        
        // Tty to take the move that best restricts the opponent
        if (blockLoseableLine(&squareIdx, current_player)) return squareIdx;
        if (addToWinnableLine(&squareIdx, current_player)) return squareIdx;
    }

    // any old square will do
    return randomMove();
}

bool testWin()
{
    // not enough moves for a winning line
    if (moves_remaining > 4) return FALSE;

    // check each winning line
    for(int i=0; i<8; i++)
    {
        if (countTilesOnWinLine(i, current_player) == 3) return TRUE;
    }

    // no win found
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////////////

int main()
{
    // load graphics
    VDP_setPlanSize(64, 32);
    loadTiles();
    loadPallete();
    loadSprites();

    // setup background scrolling
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

    // Setup joystick input handler
    JOY_init();
    JOY_setEventHandler(handleJoypadInput);

    // setup main menu
    startMainMenu();
    
    // enter game loop
    while(TRUE)
    {
        // update the scrolling background
        scrollBackground();

        // Update all sprite animations
        SPR_update();

        // Wait for frame to be fully drawn
        SYS_doVBlankProcess();
    }
    return (0);
}