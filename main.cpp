#include <iostream>
#include "raylib.h"
#include <deque>
#include <raymath.h>

using namespace std;

Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize = 30;
int cellCount = 20;
int offset = 75;

double lastUpdateTime = 0;

bool eventTriggered(double interval){
    double currentTime = GetTime();
    if(currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

bool elementinDeque(Vector2 element, deque<Vector2> deq){
    for(unsigned int i=0; i<deq.size(); i++){
        if(Vector2Equals(deq[i], element)){
            return true;
        }
    }
    return false;
}

class Snake{
    public: 
        deque<Vector2> body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
        Vector2 direction = Vector2{1, 0};
        bool addSegment = false;

        void Draw(){
            for(int i=0; i< body.size(); i++){
                int x = body[i].x;
                int y = body[i].y;
                Rectangle segment = Rectangle{ offset + (float)x*cellSize, offset + (float)y*cellSize, (float)cellSize, (float)cellSize };
                DrawRectangleRounded(segment, 0.5, 6, darkGreen);
            }
        }

        void update(){
            body.push_front(Vector2Add(body[0] , direction));
            if(addSegment){
                addSegment = false;
            }else{
                body.pop_back();
            }
        
        }

        void Reset(){
            body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
            direction = Vector2{1, 0};
        }
};

class Food {
    public:
        Vector2 position;
        Texture2D texture;

        Food(deque<Vector2> snakeBody){
            position = generateRandomPos(snakeBody);
            Image image = LoadImage("Graphics/food.png");
            texture = LoadTextureFromImage(image);
            UnloadImage(image);
        }

        ~Food(){
            UnloadTexture(texture);
        }

        void Draw(){
            // DrawRectangle(position.x * cellSize, position.y * cellSize, cellSize, cellSize, darkGreen);
            DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
        }

        Vector2 generateRandomCell(){
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{x, y};
        }

        Vector2 generateRandomPos(deque<Vector2> snakeBody){
            Vector2 position = generateRandomCell();
            while(elementinDeque(position, snakeBody)){
                position = generateRandomCell();
            }
            return position;
        }
};

class Game {
    public:
        
        Snake snake = Snake();
        Food food = Food(snake.body);
        bool running = true;
        int score  = 0;
        Sound eatSound;
        Sound wallSound;

        Game(){
            InitAudioDevice();
            eatSound = LoadSound("../Sounds/eat.mp3");
            wallSound = LoadSound("../Sounds/wall.mp3");
        }

        ~Game(){
            UnloadSound(eatSound);
            UnloadSound(wallSound);
            CloseAudioDevice();
        }

        void Draw(){

            food.Draw();
            snake.Draw();

            if(IsKeyPressed(KEY_UP) && snake.direction.y != 1){
                snake.direction = {0, -1};
                running = true;
            }else if(IsKeyPressed(KEY_DOWN) && snake.direction.y != -1){
                snake.direction = {0, 1};
                running = true;
            }else if(IsKeyPressed(KEY_LEFT) && snake.direction.x != 1){
                snake.direction = {-1, 0};
                running = true;
            }else if(IsKeyPressed(KEY_RIGHT) && snake.direction.x != -1){
                snake.direction = {1, 0};
                running = true;
            }
        }

        void Update(){
            if(!running){
                return;
            }
            
            if(eventTriggered(0.2)){
                snake.update();
            }

            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }

        void CheckCollisionWithFood(){
            if(Vector2Equals(snake.body[0], food.position)){
                food.position = food.generateRandomPos(snake.body);
                snake.addSegment = true;
                score++;
                PlaySound(eatSound);
            }
        }

        void CheckCollisionWithEdges(){
            if(snake.body[0].x >= cellCount || snake.body[0].x < 0){
                GameOver();
            }

            if(snake.body[0].y >= cellCount || snake.body[0].y < 0){
                GameOver();
            }
        }

        void GameOver(){
            snake.Reset();
            food.position = food.generateRandomPos(snake.body);
            running = false;
            score = 0;
            PlaySound(wallSound);
        }

        void CheckCollisionWithTail(){
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if(elementinDeque(snake.body[0], headlessBody)){
                GameOver();
            }
        }
};

int main(){
    InitWindow(2*offset + cellSize * cellCount, 2*offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();

    while(WindowShouldClose() == false){
        game.Update();

        BeginDrawing();

        ClearBackground(green);
        
        DrawRectangleLinesEx(Rectangle{(float)offset - 5,(float)offset - 5, (float)cellSize* cellCount + 10, (float)cellSize* cellCount + 10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("%i",game.score), offset - 5, offset + cellSize*cellCount+10, 40, darkGreen);

        game.Draw();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}