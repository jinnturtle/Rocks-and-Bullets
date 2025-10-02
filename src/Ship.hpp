#ifndef SRC_SHIP_HPP_
#define SRC_SHIP_HPP_

// TODO probably separate out a more general Particle3 and inherit from
struct Bullet final {
    Bullet(glm::vec3 pos, glm::vec3 vel);

    glm::vec3 pos;
    glm::vec3 vel;

    // TODO ttl is not used at the moment (they live forever a.k.a. mem leak)
    float ttl; // remaining time to live
};

Bullet::Bullet(glm::vec3 pos, glm::vec3 vel)
: pos {pos}
, vel {vel}
, ttl {10.0f}
{}

struct Ship final: public Obj3 {
    Ship(
        Model3* model,
        glm::vec3 pos,
        glm::vec3 rot,
        glm::vec3 color);

    glm::vec3 color;
    glm::vec3 gun_disp; // where bullets come out from, displacement rel. front;

    GLfloat accel; // acceleration
    GLfloat rot_rate;

    GLfloat shot_cooldown; // time between shots
    GLfloat shot_cooldown_rem; // remaining cooldown time till next shot
};

Ship::Ship(
    Model3* model,
    glm::vec3 pos,
    glm::vec3 rot,
    glm::vec3 color)
: Obj3(model, pos, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f}, rot)
, color {color}
, gun_disp {front.x, front.y + 0.01f, front.z}
, accel {0.1f}
, rot_rate {90.0f}
, shot_cooldown {0.2f}
, shot_cooldown_rem {shot_cooldown}
{}

#endif // SRC_SHIP_HPP_
