#include <iostream>
#include <random>
#include <vector>
#include <bits/stdc++.h>

#include <raylib.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "CollagenPrototype"

typedef struct particle_t {
    float radius;
    Vector3 position;
} particle_t;

typedef struct collagen_network_t {
    float r;
    float L;
} collagen_network_t;

typedef struct simulation_t {
    int NP;
    float D0;
    float dt;
    int current_step;
    float diffusion_tensor[6]; // Dxx, Dxy, Dxz, Dyy, Dyz, Dzz
    collagen_network_t collagen_network;
    std::vector<particle_t> particles;
} simulation_t;


std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(-1, 1);

bool is_point_in_sphere(Vector3 point) {
    return powf(point.x, 2) + powf(point.y, 2) + powf(point.z, 2) <= 1;
}

Vector3 cube_point() {
    Vector3 point = {};
    point.x = distribution(generator);
    point.y = distribution(generator);
    point.z = distribution(generator);

    return point;
}

Vector3 project_to_sphere_surface(Vector3 point) {
    float mag = sqrtf(powf(point.x, 2) + powf(point.y, 2) + powf(point.z, 2));

    Vector3 unit_point = {};
    unit_point.x = point.x / mag;
    unit_point.y = point.y / mag;
    unit_point.z = point.z / mag;

    return unit_point;
}

Vector3 calculate_displacement(float D0, float dt, Vector3 point) {
    float dr = sqrtf(6 * D0 * dt);

    Vector3 s = {};
    s.x = dr * point.x;
    s.y = dr * point.y;
    s.z = dr * point.z;

    return s;
}

Vector3 spherical_point() {
    Vector3 point = cube_point();
    while (!is_point_in_sphere(point)) {
        point = cube_point();
    }

    return point;
}

std::vector<particle_t> generate_particles(int NP) {
    std::vector<particle_t> particles;

    for (int i = 0; i < NP; i++) {
        particle_t p = {};
        p.position.x = 0;
        p.position.y = 0;
        p.position.z = 0;
        p.radius = 5;

        particles.push_back(p);
    }

    return particles;
}

void draw_collagen_network(collagen_network_t cn, float w, float h) {
    int xoffs = static_cast<int>(w / cn.L);
    int yoffs = static_cast<int>(h / cn.L);

    for (int i = 0; i < xoffs; i++) {
        for (int j = 0; j < yoffs; j++) {
            DrawCircle(cn.L / 2 + i * cn.L - (WINDOW_WIDTH / 2), cn.L / 2 + j * cn.L - (WINDOW_HEIGHT/ 2), cn.r, BLACK);
        }
    }
}

bool is_point_in_collagen_network(Vector3 point, collagen_network_t cn, float r, float w, float h) {
    float Rx = (point.x - (WINDOW_WIDTH / 2)) / cn.L - 0.5f;
    float Ry = (point.y - (WINDOW_HEIGHT / 2)) / cn.L - 0.5f;

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

    // NOTE: z position is ignored because this is a model of radial collagen

    float dist = sqrtf(powf(Rx - Cx, 2) + powf(Ry - Cy, 2));
    float tr = (cn.r + r) / cn.L;

    return dist < tr;
}

void random_walk_particle(particle_t *p, float D0, float dt, collagen_network_t cn) {
    if (!p)
        return;

    Vector3 s = spherical_point();
    s = project_to_sphere_surface(s);
    s = calculate_displacement(D0, dt, s);

    float nx = p->position.x + s.x;
    float ny = p->position.y + s.y;
    float nz = p->position.z + s.z;

    Vector3 np = {nx, ny, nz};

    bool is_next_step_inside = is_point_in_collagen_network(np, cn, p->radius, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (is_next_step_inside)
        return;

    p->position.x += s.x;
    p->position.y += s.y;
    p->position.z += s.z;
}

void init_simulation(simulation_t *simulation, int NP, float D0, float dt, float L, float r) {
    if (!simulation)
        return;

    simulation->current_step = 0;
    simulation->NP = NP;
    simulation->D0 = D0;
    simulation->dt = dt;
    //simulation->diffusion_tensor = {0, 0, 0, 0, 0, 0};
    simulation->collagen_network = {};
    simulation->collagen_network.L = L;
    simulation->collagen_network.r = r;

    simulation->particles = generate_particles(simulation->NP);
}

void step_simulation(simulation_t *simulation) {
    simulation->current_step++;

    simulation->diffusion_tensor[0] = 0;
    simulation->diffusion_tensor[1] = 0;
    simulation->diffusion_tensor[2] = 0;
    simulation->diffusion_tensor[3] = 0;
    simulation->diffusion_tensor[4] = 0;
    simulation->diffusion_tensor[5] = 0;
    
    for (auto & p : simulation->particles) {
        float x = p.position.x;
        float y = p.position.y;
        float z = p.position.z;

        simulation->diffusion_tensor[0] += x * x;
        simulation->diffusion_tensor[1] += x * y;
        simulation->diffusion_tensor[2] += x * z;
        simulation->diffusion_tensor[3] += y * y;
        simulation->diffusion_tensor[4] += y * z;
        simulation->diffusion_tensor[5] += z * z;

        float coefactor = (1 / (2 * simulation->current_step * simulation->dt * simulation->NP));

        simulation->diffusion_tensor[0] *= coefactor;
        simulation->diffusion_tensor[1] *= coefactor;
        simulation->diffusion_tensor[2] *= coefactor;
        simulation->diffusion_tensor[3] *= coefactor;
        simulation->diffusion_tensor[4] *= coefactor;
        simulation->diffusion_tensor[5] *= coefactor;

        random_walk_particle(&p, simulation->D0, simulation->dt, simulation->collagen_network);
    }

    std::cout << simulation->diffusion_tensor[0] << ", " << simulation->diffusion_tensor[1]  << ", " << simulation->diffusion_tensor[2] << std::endl;
    std::cout << simulation->diffusion_tensor[1] << ", " << simulation->diffusion_tensor[3]  << ", " << simulation->diffusion_tensor[4] << std::endl;
    std::cout << simulation->diffusion_tensor[2] << ", " << simulation->diffusion_tensor[4]  << ", " << simulation->diffusion_tensor[5] << std::endl;
    std::cout << std::endl;
}

void draw_simulation(simulation_t *simulation) {
    draw_collagen_network(simulation->collagen_network, WINDOW_WIDTH, WINDOW_HEIGHT);
    for (auto &p : simulation->particles) {
        DrawCircle(p.position.x, p.position.y, 5, RED);
    }
}

int main() {

    int N = 10000;
    float D0 = 5.0f;
    float dt = 0.5f;

    simulation_t simulation = {};
    init_simulation(&simulation, N, D0, dt, 80, 15);

    Camera2D camera = {};
    camera.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    camera.target = {};
    camera.zoom = 1.0f;
    camera.rotation = 0.0f;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode2D(camera);
        draw_simulation(&simulation);
        EndMode2D();

        EndDrawing();

        step_simulation(&simulation);
    }

    //CloseWindow();

    return 0;
}
