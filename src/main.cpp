#include <array>
#include <chrono>
#include <cmath>
#include <sstream>
#include <thread>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ktx.h>

#include "Obj3.hpp"
#include "Ship.hpp"
#include "logs.hpp"
#include "utils.hpp"
#include "version.hpp"

/* TODO consider getting rid of debug-only compilation code, enable debug mode
   via cmd arg instead */

// TODO temporary solution to test out some things
enum Player_id {
    PID_pl1 = 0,
    PID_pl2
};

GLFWwindow* init_window(int w, int h, const std::string& name);

int main(int argc, char** argv)
{
    const std::string program_name {"Rocks and Bullets"};
    int win_w {1280};
    int win_h {720};

    logs::info("PROGRAM START");
    logs::info("name: ", program_name, " ", version_str());

#ifdef DEBUG
    DBG(0, "DEBUG: ", DEBUG);

    {
        std::stringstream ss;
        for(int i {0}; i < argc; ++i) {
            ss << argv[i];
            if (i + 1 < argc) { ss << " "; }
        }

        logs::info("args (", argc, "): ", ss.str());
    }

    DBG(0, "GL_MAX_UNIFORM_LOCATIONS: ", GL_MAX_UNIFORM_LOCATIONS);
#endif //DEBUG

    GLFWwindow* window = init_window(win_w, win_h, program_name);
    if (window == nullptr) {
        logs::err("failed to initialize window");
        return -1;
    }

#ifdef DEBUG
    {
    const char* gl_s = (const char*) glGetString(GL_VERSION);
    DBG(0, "GL_VERSION: ", gl_s ? gl_s : "(null)");

    GLint gl_i {0};
    glGetIntegerv(GL_NUM_EXTENSIONS, &gl_i);
    DBG(0, "GL_NUM_EXTENSIONS: ", gl_i);
    if (DEBUG >= 9) {
        DBG(9, "GL_EXTENSIONS: vvv");
        for (GLint i {0}; i < gl_i; ++i) {
            gl_s =
                reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
            DBG(9, "GL_EXT ", i, ": ", gl_s ? gl_s : "(null)");
        }
    }
    }
#endif

    // Create and set VAO
    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    GLuint vertex_buffer_id;
    glGenBuffers(1, &vertex_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);

    GLuint shader_id =
        load_shaders("shaders/simple.vert", "shaders/simple.frag");
    if (shader_id == 0) {
        logs::err("failed to load shaders");
        glfwTerminate();
        return -1;
    }

    GLuint shader_id_tex =
        load_shaders("shaders/simple_tex.vert", "shaders/simple_tex.frag");
    if (shader_id == 0) {
        logs::err("failed to load shaders");
        glfwTerminate();
        return -1;
    }

    // TEXTURE EXPERIMENT ----------------------------------------
    GLuint texture;
	glGenTextures(1, &texture);
    /* all upcoming GL_TEXTURE_2D operations now have effect on this texture
       object */
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
    // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    // KTX2 -- texture loading experiment --------------------------------------
    ktxTexture2* kTexture;
    KTX_error_code result;

    result = ktxTexture_CreateFromNamedFile(
        "gfx/fonts/terminus_8x16.ktx2",
        // KTX_TEXTURE_CREATE_NO_FLAGS,
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        (ktxTexture**)&kTexture);
    if (result != KTX_SUCCESS) {
        logs::err("KTX create from file failed: ", ktxErrorString(result));
        return -1;
    }

    /* TODO - adapt transcode target format based on GPU extensions available,
       at least differentiate between BC3-7, as desktops are priority now */
    /* force transcoding to BC7 as it may want to default to ASTC but that's
       usually only available on mobile cards. */
    if (ktxTexture2_NeedsTranscoding(kTexture)) {
        result = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);
        if (result != KTX_SUCCESS) {
            logs::err(
                "KTX ktxTexture2_TranscodeBasis failed:",
                ktxErrorString(result));
            return -1;
        }
    }
    result = ktxLoadOpenGL((PFNGLGETPROCADDRESS)glfwGetProcAddress);
    if (result != KTX_SUCCESS) {
        logs::err("KTX LoadOpenGL failed: ", ktxErrorString(result));
        return -1;
    }

    GLenum target, glerror;
    result = ktxTexture_GLUpload((ktxTexture*)kTexture, &texture, &target, &glerror);
    if (result != KTX_SUCCESS) {
        logs::err("KTX GLUpload failed: ", ktxErrorString(result));
        return -1;
    }
    ktxTexture_Destroy(reinterpret_cast<ktxTexture*>(kTexture));
    // END KTX2 -- texture loading experiment ----------------------------------


    Model3 square_model;
    square_model.verts = std::vector{
        // 3x vert coord, 2x texture coord
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom left
        64.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // bottom right
        0.0f, 208.0f, 0.0f, 0.0f, 0.0f, // top left
        64.0f, 208.0f, 0.0f, 1.0f, 0.0f, // top right
    };


    // projection matrix
    constexpr float fov{glm::radians(60.0f)}; // field of view
    const float aspect_r{static_cast<float>(win_w) / win_h}; // aspect ratio
    constexpr float near_clip{0.1f}; // near clipping plane
    constexpr float far_clip{100.0f}; // far clipping plane
    glm::mat4 proj_mx(glm::perspective(fov, aspect_r, near_clip, far_clip));
    DBG(0, "WxH (AR): ", win_w, "x", win_h, " (", aspect_r, ")");

    // view matrix
    float cam_distance {-40.0f};
    glm::mat4 view_mx(1.0f);
    view_mx = glm::translate(view_mx, glm::vec3(0.0f, 0.0f, cam_distance));

    constexpr glm::vec3 color_debug{1.0f, 0.5f, 0.0f};
    constexpr glm::vec3 color_bullet{1.0f, 1.0f, 1.0f};

    Model3 spaceship_model;
    spaceship_model.verts = std::vector{
        -0.65f, -0.65f, 0.0f,
        0.65f, -0.65f, 0.0f,
        0.0f,  1.0f, 0.0f};

    std::array<Ship, 2> ships{
        Ship(
            &spaceship_model,
            glm::vec3{-10.0f, 0.0f, 0.0f},
            glm::vec3{0.0f},
            glm::vec3{0.0f, 1.0f, 1.0f}),
        Ship(
            &spaceship_model,
            glm::vec3{10.0f, 0.0f, 0.0f},
            glm::vec3{0.0f},
            glm::vec3{0.0f, 1.0f, 0.5f})
    };

    std::vector<Bullet> bullets;
    float bullet_muz_vel {0.05f};

    // determine world-space size of screen at distance
    float view_half_h {std::abs(cam_distance) * std::tan(fov / 2)};
    float view_half_w {view_half_h * aspect_r};
    float pixel_size {2 * view_half_w / win_w};
    DBG(0, "vW/2: ", view_half_w, " vH/2: ", view_half_h, " pix: ", pixel_size);

    Boxf arena_bounds {-view_half_w, -view_half_h,
                       view_half_w * 2, view_half_h * 2};

    GLfloat arena_bounds_verts[] {
        arena_bounds.x, arena_bounds.y, 0.0f,
        arena_bounds.x + arena_bounds.w, arena_bounds.y, 0.0f,
        arena_bounds.x + arena_bounds.w, arena_bounds.y + arena_bounds.h, 0.0f,
        arena_bounds.x, arena_bounds.y + arena_bounds.h, 0.0f,
    };

    constexpr unsigned fps_tgt {60}; // FPS target
    float dt {1.0f / fps_tgt}; // TODO hardcoded, adapt to actual delta time
    constexpr std::chrono::milliseconds frame_dur_tgt{1000/fps_tgt};
    unsigned int trans_loc = glGetUniformLocation(shader_id, "transform");
    unsigned int view_loc = glGetUniformLocation(shader_id, "view");
    unsigned int proj_loc = glGetUniformLocation(shader_id, "projection");
    unsigned int color_loc = glGetUniformLocation(shader_id, "color");

    unsigned int tex_trans_loc =
        glGetUniformLocation(shader_id_tex, "transform");
    unsigned int tex_view_loc =
        glGetUniformLocation(shader_id_tex, "view");
    unsigned int tex_proj_loc =
        glGetUniformLocation(shader_id_tex, "projection");
    unsigned int tex_color_loc =
        glGetUniformLocation(shader_id_tex, "color");

    bool should_close {false};
    while (!should_close) {
        glEnableVertexAttribArray(0);

        glClearColor(0.0f, 0.01f, 0.03f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_W)) {
            ships[PID_pl1].vel +=
                ships[PID_pl1].accel * ships[PID_pl1].front * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            ships[PID_pl1].rot.z += ships[PID_pl1].rot_rate * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            ships[PID_pl1].rot.z -= ships[PID_pl1].rot_rate * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            if (ships[PID_pl1].shot_cooldown_rem <= 0.0f) {
                // spawn with ship-relative pos so ship-relative rot works well
                bullets.push_back(Bullet{
                        ships[PID_pl1].gun_disp,
                        ships[PID_pl1].vel});

                glm::mat4 trans_mx {1.0f};
                /* TODO prob. better use Obj3.rot as axis argument containing
                   fraction of 360deg instead of passing rot and hardcoded
                   axis, feels more natural data-wise and more efficient if we
                   later need rotation for more than one axis simultaneously. */
                trans_mx = glm::rotate(
                    trans_mx,
                    glm::radians(ships[PID_pl1].rot.z),
                    glm::vec3(0.0f, 0.0f, 1.0f));
                // just because we need vec4 for glm rotation calc, I'm lazy atm
                glm::vec4 pos_buf {bullets.back().pos, 0.0f};
                pos_buf = trans_mx * pos_buf;
                bullets.back().pos.x = pos_buf.x;
                bullets.back().pos.y = pos_buf.y;

                // move to world-relative pos,no longer care about ship-relative
                bullets.back().pos += ships[PID_pl1].pos;

                // bullet be propelled towards where the ship (gun) is facing
                bullets.back().vel.x += bullet_muz_vel * ships[PID_pl1].front.x;
                bullets.back().vel.y += bullet_muz_vel * ships[PID_pl1].front.y;

                ships[PID_pl1].shot_cooldown_rem +=
                    ships[PID_pl1].shot_cooldown;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_I)) {
            ships[PID_pl2].vel +=
                ships[PID_pl2].accel * ships[PID_pl2].front * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_J)) {
            ships[PID_pl2].rot.z += ships[PID_pl2].rot_rate * dt;
        }
        if (glfwGetKey(window, GLFW_KEY_L)) {
            ships[PID_pl2].rot.z -= ships[PID_pl2].rot_rate * dt;
        }

        // update phase

        for (auto& bullet : bullets) {
            bullet.pos += bullet.vel;
        }

        for (auto & ship : ships) {
            ship.pos += ship.vel;

            if (ship.pos.x < arena_bounds.x) {
                ship.pos.x += arena_bounds.w;
            } else if (ship.pos.x > arena_bounds.x + arena_bounds.w) {
                ship.pos.x -= arena_bounds.w;
            }
            if (ship.pos.y < arena_bounds.y) {
                ship.pos.y += arena_bounds.h;
            } else if (ship.pos.y > arena_bounds.y + arena_bounds.h) {
                ship.pos.y -= arena_bounds.h;
            }

            ship.front.x = -sin(glm::radians(ship.rot.z));
            ship.front.y = cos(glm::radians(ship.rot.z));

            if (ship.shot_cooldown_rem > 0.0f) {ship.shot_cooldown_rem -= dt;}
        }


        // drawing phase

        // drawing ships

        glUseProgram(shader_id);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_mx));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj_mx));

        for (auto& ship : ships) {
            // TODO would it make sense to store the model matrix in class?
            glm::mat4 trans_mx {glm::mat4(1.0f)}; // transformation matrix
            trans_mx = glm::translate(trans_mx, ship.pos);
            trans_mx = glm::rotate(
                trans_mx,
                glm::radians(ship.rot.z),
                glm::vec3(0.0f, 0.0f, 1.0f));


            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            // TODO encapsulate, move, etc (same for other objects)
            // TODO should send the ship model only once, rest are instances
            // drawing the ship
            glUniform3fv(color_loc, 1, glm::value_ptr(ship.color));
            glUniformMatrix4fv(
                trans_loc, 1, GL_FALSE, glm::value_ptr(trans_mx));
            glBufferData(
                GL_ARRAY_BUFFER,
                ship.model->verts.size() * sizeof(ship.model->verts[0]),
                ship.model->verts.data(),
                GL_STATIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, ship.model->verts.size() / 3);
        }

        // drawing bullets
        {
            // just drawing a point at local origin
            GLfloat verts[] {0.0f, 0.0f, 0.0f};
            glBufferData(
                GL_ARRAY_BUFFER,
                3 * sizeof(verts[0]),
                verts,
                GL_STATIC_DRAW);

        }
        for (auto& bullet : bullets) {
            glm::mat4 trans_mx {glm::mat4(1.0f)}; // transformation matrix
            trans_mx = glm::translate(trans_mx, bullet.pos);

            glUseProgram(shader_id);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            glUniform3fv(color_loc, 1, glm::value_ptr(color_bullet));
            glUniformMatrix4fv(
                trans_loc, 1, GL_FALSE, glm::value_ptr(trans_mx));

            glDrawArrays(GL_POINTS, 0, 1);
        }

        // drawing the arena bounds
        glUniform3fv(color_loc, 1, glm::value_ptr(color_debug));
        glUniformMatrix4fv(
            trans_loc, 1, GL_FALSE, glm::value_ptr(glm::mat4{1.0f}));
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(arena_bounds_verts), arena_bounds_verts, GL_STATIC_DRAW);
        glDrawArrays(
            GL_LINE_LOOP, 0,
            sizeof(arena_bounds_verts) / sizeof(arena_bounds_verts[0]) / 3);

        glDisableVertexAttribArray(0);

        // drawing a test texture
        glUseProgram(shader_id_tex);
        glUniformMatrix4fv(tex_view_loc, 1, GL_FALSE, glm::value_ptr(view_mx));
        glUniformMatrix4fv(tex_proj_loc, 1, GL_FALSE, glm::value_ptr(proj_mx));
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        {
            glm::vec3 texture_color {.85f, .85f, .85f};
            glm::mat4 trans_mx {glm::mat4(1.0f)};
            trans_mx = glm::scale(trans_mx, glm::vec3{0.1f});
            trans_mx = glm::translate(trans_mx, glm::vec3{0.0f});

            glUniform3fv(tex_color_loc, 1, glm::value_ptr(texture_color));
            glUniformMatrix4fv(
                tex_trans_loc, 1, GL_FALSE, glm::value_ptr(trans_mx));
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
            glBufferData(
                GL_ARRAY_BUFFER,
                square_model.verts.size() * sizeof(square_model.verts[0]),
                square_model.verts.data(),
                GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                                  5 * sizeof(square_model.verts[0]), nullptr);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(
                1, 2, GL_FLOAT, GL_FALSE,
                5 * sizeof(square_model.verts[0]),
                (void*)(3* sizeof(square_model.verts[0])));
            glEnableVertexAttribArray(1);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glfwSwapBuffers(window);

        glfwPollEvents();

        if (glfwWindowShouldClose(window) != 0 ||
            glfwGetKey(window, GLFW_KEY_ESCAPE))
        {
            should_close = true;
        }

        std::this_thread::sleep_for(frame_dur_tgt); // TODO sleep remainder only
    }

    glfwTerminate();
    logs::info("PROGRAM END");

    return 0;
}

GLFWwindow* init_window(int w, int h, const std::string& name)
{
    GLFWwindow* window {nullptr};

    if (!glfwInit()) {
        logs::err("failed to init GLFW");
        return window;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // We don't want the old OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(w, h, name.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        logs::err("failed to create GLFW window");
        glfwTerminate();
        return window;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        logs::err("failed to init GLEW");
        glfwTerminate();
        return window;
    }

    // Ensure we can capture the escape key being pressed
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    return window;
}
