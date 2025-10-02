/* Structures for defining basic 3d models */

#ifndef SRC_OBJ3_HPP_
#define SRC_OBJ3_HPP_

// TODO warn/error if already defined

// generic 3d object model
struct Model3 final {
    std::vector<GLfloat> verts; // vertices
    // can define more here as needed (normals, UVs, etc.)
};

// generic 3d object
struct Obj3 {
    Obj3(
        Model3* model,
        glm::vec3 pos,
        glm::vec3 front,
        glm::vec3 vel,
        glm::vec3 rot);
    virtual ~Obj3(){};

    Model3* model;
    glm::vec3 pos; // position
    glm::vec3 front; // facing in local space
    glm::vec3 vel; // velocity
    glm::vec3 rot; // rotation
};

Obj3::Obj3(
    Model3* model,
    glm::vec3 pos,
    glm::vec3 front,
    glm::vec3 vel,
    glm::vec3 rot)
: model {model}
, pos {pos}
, front {front}
, vel {vel}
, rot {rot}
{}

#endif // SRC_OBJ3_HPP_
