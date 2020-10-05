#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

//Curent bugs and problems
//Velocity vector not working       --SOLVED
//Find way to scale trail    
//Fix Zoom
struct Body {
    olc::vd2d pos;
    olc::vd2d velocity;
    double mass;
    double radius;
    olc::Pixel colour;
    std::string name;
    std::vector<olc::vd2d> trail;
    olc::vd2d apparentPos;
    uint32_t trailLength;
    uint32_t MAX_TRAIL_LENGTH;
    Body(std::string n, olc::vd2d p, olc::vd2d v, double m, double r, olc::Pixel c = olc::WHITE) {
        pos = p;
        velocity = v;
        mass = m;
        radius = r;
        colour = c;
        name = n;
        trailLength = 10;
        MAX_TRAIL_LENGTH = 300;
    }
    void UpdateTrail(olc::vd2d &startingPos, float scale) {
        uint32_t newTrailLength = (uint32_t)velocity.mag();
        int32_t difference = trailLength - newTrailLength;
        trailLength = std::min(newTrailLength, MAX_TRAIL_LENGTH);
        if (trail.size() == 0) {
            if (trailLength > 0) {
                trail.push_back(pos);
            }
        } else {
            if (trail.size() < (std::size_t)trailLength) {
                if ((int32_t)trail.at(trail.size() - 1).x != (int32_t)pos.x || (int32_t)trail.at(trail.size() - 1).y != (int32_t)pos.y) {
                    trail.push_back(pos);
                }
            } else {
                if ((int32_t)trail.at(trail.size() - 1).x != (int32_t)pos.x || (int32_t)trail.at(trail.size() - 1).y != (int32_t)pos.y) {
                    trail.push_back(pos);
                    trail.erase(trail.begin());
                } else {
                    if (difference > 0) {
                        trail.erase(trail.begin(), trail.begin() + difference);
                    }
                }
            }
        }
    }

    void ApplyForce(olc::vd2d force) {
        velocity += force / mass;
    }
};
class PlanetaryPhysics : public olc::PixelGameEngine {
private:
    std::vector<Body> bodies;
    olc::vd2d barycentre;

    olc::vd2d camera;
    olc::vd2d centre;
    double GRAVITATIONAL_CONSTANT = 0.0001;
    double scale = 1;

    bool locked;
    int lockedNumber;

    bool forceLocked;
    int forceLockedNumber;

    bool barycentreLocked;
    bool paused;
    bool moving;
    bool trails;
    bool showVelocity;

    double angle2;
public:
    PlanetaryPhysics() {
        sAppName = "Planetary Physics";
        camera = olc::vd2d(0,0);
        barycentre = olc::vd2d(0, 0);
        centre = olc::vd2d(0,0);
        locked = false;
        lockedNumber = 0;
        forceLockedNumber = 0;
        paused = false;
        trails = true;
        showVelocity = false;
        barycentreLocked = false;
    }
public:
    bool OnUserCreate() override {
        Body sun("Sun", olc::vd2d(0, 0), olc::vd2d(0, 0), 2000, 20, olc::YELLOW);
        Body sun2("Sun2", olc::vd2d(ScreenWidth()*0.25, 0), olc::vd2d(0, 300), 2000, 20, olc::YELLOW);
        Body planet("Planet 1", olc::vd2d(ScreenWidth()*0.25, 0), olc::vd2d(0, sqrt(4*GRAVITATIONAL_CONSTANT*(sun.mass+1)/ (double)(ScreenWidth()*0.25))), 1, 5);
        Body planet2("Planet 2",olc::vd2d(ScreenWidth(), 0), olc::vd2d(0, 100.0f), 1, 5);
        Body planet3("Planet 3", olc::vd2d(ScreenWidth()*0.25f, 0), olc::vd2d(0, 300.0f), 10, 10);
        bodies.push_back(sun);
        //bodies.push_back(sun2);
        //bodies.push_back(planet);
        bodies.push_back(planet2);
        //bodies.push_back(planet3);
        return true;
    }
    bool OnUserUpdate(float fElapsedTime) override {
        // called once per frame, draws random coloured pixels
        Clear(olc::Pixel(0,3,51));
        double scaleModifier = 0.01f;
        if (IsFocused()) {
            if(!locked){
                float movementMultiplier = 8000.0f;
                if (GetKey(olc::Key::W).bHeld) {
                    camera.y += movementMultiplier * scaleModifier * fElapsedTime;
                }
                if (GetKey(olc::Key::S).bHeld) {
                    camera.y -= movementMultiplier * scaleModifier *  fElapsedTime;
                }
                if (GetKey(olc::Key::A).bHeld) {
                    camera.x += movementMultiplier * scaleModifier *  fElapsedTime;
                }
                if (GetKey(olc::Key::D).bHeld) {
                    camera.x -= movementMultiplier * scaleModifier *  fElapsedTime;
                }
            }
            if (GetKey(olc::Key::SHIFT).bHeld) {
                scale += scaleModifier * 0.1 * (scale);
            }
            if (GetKey(olc::Key::CTRL).bHeld) {
                scale -= scaleModifier * 0.1 * (scale);
            }
            if (GetKey(olc::Key::E).bPressed) {
                locked = !locked;
                forceLocked = false;
                barycentreLocked = false;
                ClearTrails();
            }
            if (GetKey(olc::Key::R).bPressed) {
                forceLocked = !forceLocked;
                locked = false;
                barycentreLocked = false;
                ClearTrails();
            }
            if (GetKey(olc::Key::B).bPressed) {
                barycentreLocked = !barycentreLocked;
                locked = false;
                forceLocked = false;
                ClearTrails();
            }
            if (GetKey(olc::Key::T).bPressed) {
                trails = !trails;
            }
            if (GetKey(olc::Key::V).bPressed) {
                showVelocity = !showVelocity;
            }
            if (GetKey(olc::Key::SPACE).bPressed) {
                paused = !paused;
            }

            if (locked) {
                if (GetKey(olc::Key::LEFT).bPressed) {
                    lockedNumber--;
                    ClearTrails();
                }
                if (GetKey(olc::Key::RIGHT).bPressed) {
                    lockedNumber++;
                    ClearTrails();
                }
            } else {
                if (GetKey(olc::Key::LEFT).bPressed) {
                    forceLockedNumber--;
                    ClearTrails();
                }
                if (GetKey(olc::Key::RIGHT).bPressed) {
                    forceLockedNumber++;
                    ClearTrails();
                }
            }
            if (forceLocked) {
                if (GetMouse(0).bHeld) {
                    DrawLine(calibrate(bodies.at(forceLockedNumber%bodies.size()).pos), olc::vd2d(GetMouseX(), GetMouseY()), olc::RED);
                    olc::vd2d mouseToWorld = ScreenToWorld((olc::vd2d(GetMouseX(), GetMouseY()) - camera) / scale);
                    olc::vd2d force = (olc::vd2d(GetMouseX(), GetMouseY()))-calibrate(bodies.at(forceLockedNumber%bodies.size()).pos);
                    bodies.at(forceLockedNumber%bodies.size()).ApplyForce(force.norm()*bodies.at(forceLockedNumber%bodies.size()).mass*0.05f);
                }
            }
        }
        if (!paused) {
            UpdatePhysics();
            UpdatePositions(fElapsedTime);
        }
        if (locked) {
            centre = bodies.at(lockedNumber%bodies.size()).pos;
            camera = olc::vd2d(0, 0);
        }
        if (barycentreLocked) {
            centre = barycentre;
            camera = olc::vd2d(0, 0);
        }

        //Drawing
        for (unsigned int i = 0; i < bodies.size(); i++) {
            olc::vd2d position = calibrate(bodies.at(i).pos);
            bodies.at(i).UpdateTrail(position, scale);

            //Locked outline on planet
            if (locked && lockedNumber%bodies.size() == i) {
                FillCircle(position, (int32_t)(bodies.at(i).radius*(scale))+1, olc::GREEN);
            }
            if (forceLocked && forceLockedNumber%bodies.size() == i) {
                FillCircle(position, (int32_t)(bodies.at(i).radius*(scale)) + 1, olc::RED);
            }
            //Drawing body
            FillCircle(position, (int32_t)bodies.at(i).radius*(scale), bodies.at(i).colour);
            //Drawing trails
            if (trails) {
                for (unsigned int j = 0; j < bodies.at(i).trail.size(); j++) {
                    Draw(calibrate(bodies.at(i).trail.at(j)), bodies.at(i).colour);
                    //Draw(bodies.at(i).trail.at(j), bodies.at(i).colour);
                }
            }
            //Drawing velocity vector
            if (showVelocity) {
                DrawArrow(bodies.at(i).pos, bodies.at(i).velocity);
            }
        }
        //Drawing Barycentre
        Draw(calibrate(barycentre), olc::RED);

        CalculateBarycentre();
        //HUD
        std::ostringstream scalePrint;
        scalePrint.precision(3);
        scalePrint << std::fixed << scale;
        //Text
        DrawString(olc::vi2d(9, 0), locked ? "Locked:  " + bodies.at(lockedNumber%bodies.size()).name: "Unlocked", locked? olc::GREEN:olc::YELLOW);
        DrawString(olc::vi2d(10, 10), forceLocked ? "Force Locked:  " + bodies.at(forceLockedNumber%bodies.size()).name : "Force Unlocked", forceLocked ? olc::GREEN : olc::YELLOW);
        DrawString(olc::vi2d(9, 20), trails ? "Trails Enabled" : "Trails Disabled", trails ? olc::GREEN : olc::RED);
        DrawString(olc::vi2d(9, 30), "Scale: " + scalePrint.str(), olc::WHITE);
        DrawString(olc::vi2d((int32_t)(ScreenWidth()*0.8f), 0), paused?"PAUSED":"", paused?olc::RED:olc::WHITE);
        DrawString(olc::vi2d(10, (int32_t)ScreenHeight()*0.89), locked ? "Mass: " + std::to_string((int32_t)bodies.at(lockedNumber%bodies.size()).mass) : "", locked ? olc::GREEN : olc::WHITE);
        DrawString(olc::vi2d(10, (int32_t)ScreenHeight()*0.95), locked ? "Speed:" + std::to_string((int32_t)bodies.at(lockedNumber%bodies.size()).velocity.mag()) : "", locked ? olc::GREEN : olc::WHITE);
        return true;
    }

    olc::vd2d calibrate(olc::vd2d pos) {
        return WorldToScreen((pos - (centre))*scale + camera);
    }
    olc::vd2d WorldToScreen(olc::vd2d c) {
        return olc::vd2d(c.x + ScreenWidth()/2, c.y + ScreenHeight()/2);
    }
    olc::vd2d ScreenToWorld(olc::vd2d c) {
        return olc::vd2d(c.x - ScreenWidth() / 2, c.y - ScreenHeight() / 2);
    }

    void UpdatePhysics() {
        // called once per frame, draws random coloured pixels
        for (unsigned int i = 0; i < bodies.size(); i++) {
            olc::vd2d forceTotal(0,0);
            for (unsigned int j = 0; j < bodies.size(); j++) {
                if (i != j) {
                    olc::vd2d force = bodies.at(j).pos - bodies.at(i).pos;
                    force = force.norm();
                    double distance = force.mag();
                    double multipliers = (GRAVITATIONAL_CONSTANT * bodies.at(i).mass * bodies.at(j).mass);
                    force *= multipliers / (distance*distance);
                    forceTotal += force;
                }
            }
            bodies.at(i).ApplyForce(forceTotal);
        }
    }

    void UpdatePositions(float fElapsedTime) {
        for (unsigned int i = 0; i < bodies.size(); i++) {
            bodies.at(i).pos += bodies.at(i).velocity*fElapsedTime;
            bodies.at(i).apparentPos = calibrate(bodies.at(i).pos);
        }
    }

    void ClearTrails() {
        for (unsigned int i = 0; i < bodies.size(); i++) {
            bodies.at(i).trail.clear();
        }
    }

    void DrawArrow(olc::vd2d &startPos, olc::vd2d &direction, double length = 100) {
        olc::Pixel colour = olc::DARK_YELLOW;
        olc::vd2d lineEndPos = calibrate(startPos + (direction.norm()*(direction.mag() < length ? direction.mag() : length)));
        DrawLine(calibrate(startPos) , lineEndPos, colour);
        /*
        double angle = atan2(direction.y,direction.x);
        olc::vd2d sideRight(cos(angle + 135), sin(angle + 135));
        olc::vd2d sideLeft (cos(angle - 90), sin(angle - 90));
        DrawLine(lineEndPos, lineEndPos + sideRight*5, colour);
        DrawLine(lineEndPos, lineEndPos + sideLeft * 5, colour);
        */
    }

    void CalculateBarycentre() {
        double radius = 0;
        std::vector<olc::vd2d> barycentres;
        for (uint32_t i = 0; i < bodies.size(); i++) {
            for (uint32_t j = 0; j < bodies.size(); j++) {
                if (i < j) {
                    radius = (bodies.at(j).pos - bodies.at(i).pos).mag()*bodies.at(j).mass / (bodies.at(i).mass + bodies.at(j).mass);
                    barycentres.push_back(bodies.at(i).pos + ((bodies.at(j).pos - bodies.at(i).pos).norm() * radius));
                    //Draw(calibrate(bodies.at(i).pos + ((bodies.at(j).pos - bodies.at(i).pos).norm() * radius)), olc::GREEN);
                    //DrawLine(calibrate(bodies.at(i).pos), calibrate(bodies.at(j).pos));
                }
            }
        }
        olc::vd2d average(0,0);
        for (uint32_t i = 0; i < barycentres.size(); i++) {
            average.x += barycentres.at(i).x;
            average.y += barycentres.at(i).y;
        }
        average /= barycentres.size();
        barycentre = average;
    }
};
int main() {
    PlanetaryPhysics demo;
    if (demo.Construct(256, 240, 4, 4))
        demo.Start();
    return 0;
}