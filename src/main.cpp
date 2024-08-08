#include <iostream>
#include <vector>
#include <random>

#include <raylib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define WINDOW_TITLE "CollagenPrototype"

typedef struct particle_t {
    float radius;
    Vector2 position;
} particle_t;

typedef struct collagen_network_t {
    float r;
    float L;
} collagen_network_t;

typedef struct simulation_t {
    int N;
    float D0;
    float dt;
    collagen_network_t collagen_network;
    std::vector<particle_t> particles;
} simulation_t;


std::vector<particle_t> generate_particles(int N) {
    std::vector<particle_t> particles;

    for (int i = 0; i < N; i++) {
        particle_t p = {};
        p.position.x = WINDOW_WIDTH / 2;
        p.position.y = WINDOW_HEIGHT / 2;
        p.radius = 5;

        particles.push_back(p);
    }

    return particles;
}

float generate_random_unit_angle() {
    float full_angle = 2.0 * PI;
    float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / full_angle);

    return theta;
}

Vector2 get_position_from_unit_angle(float theta) {
    float x = cosf(theta);
    float y = sinf(theta);

    return {x, y};
}

void draw_collagen_network(collagen_network_t cn, float w, float h) {
    int xoffs = static_cast<int>(w / cn.L);
    int yoffs = static_cast<int>(h / cn.L);

    for (int i = 0; i < xoffs; i++) {
        for (int j = 0; j < yoffs; j++) {
            DrawCircle(cn.L / 2 + i * cn.L, cn.L / 2 + j * cn.L, cn.r, BLACK);
        }
    }
}

bool is_point_in_collagen_network(Vector2 point, collagen_network_t cn, float r, float w, float h) {
    float Rx = point.x / cn.L - 0.5f;
    float Ry = point.y / cn.L - 0.5f;

    float Cx = floor(Rx);
    float dx = Rx - Cx;
    if (dx > 0.5) {
        Cx = ceil(Rx);
    }

    float Cy = floor(Ry);
    float dy = Ry - Cy;
    if (dy > 0.5) {
        Cy = ceil(Ry);
    }

    float dist = sqrtf(powf(Rx - Cx, 2) + powf(Ry - Cy, 2));
    float tr = (cn.r + r) / cn.L;

    return dist < tr;
}

void random_walk_particle(particle_t *p, float d0, float dt, collagen_network_t cn) {
    if (!p)
        return;

    float dr = sqrtf(2 * d0 * dt);

    float angle = generate_random_unit_angle();
    Vector2 ds = get_position_from_unit_angle(angle);

    float nx = p->position.x + dr * ds.x;
    float ny = p->position.y + dr * ds.y;

    Vector2 point = {nx, ny};

    bool is_next_step_inside = is_point_in_collagen_network(point, cn, p->radius, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (is_next_step_inside)
        return;

    p->position.x = nx;
    p->position.y = ny;
}

void init_simulation(simulation_t *simulation, int N, float D0, float dt, float L, float r) {
    if (!simulation)
        return;

    simulation->N = N;
    simulation->D0 = D0;
    simulation->dt = dt;
    simulation->collagen_network = {};
    simulation->collagen_network.L = L;
    simulation->collagen_network.r = r;

    simulation->particles = generate_particles(simulation->N);
}

void step_simulation(simulation_t *simulation) {
    for (auto & p : simulation->particles) {
        random_walk_particle(&p, simulation->D0, simulation->dt, simulation->collagen_network);
    }
}

void draw_simulation(simulation_t *simulation) {
    draw_collagen_network(simulation->collagen_network, WINDOW_WIDTH, WINDOW_HEIGHT);
    for (auto &p : simulation->particles) {
        DrawCircle(p.position.x, p.position.y, 5, RED);
    }
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    int N = 250;
    float D0 = 0.5f;
    float dt = 1.0f;

    simulation_t simulation = {};
    init_simulation(&simulation, N, D0, dt, 80, 15);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        draw_simulation(&simulation);

        EndDrawing();

        step_simulation(&simulation);
    }

    CloseWindow();

    return 0;
}