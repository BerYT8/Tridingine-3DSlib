#pragma once

class Player
{
public:

    bool Init();

    void Update();
    void Draw();

    float X() const;
    float Y() const;

    float Radius() const;

private:

    float x;
    float y;

    float radius;
    float speed;
};