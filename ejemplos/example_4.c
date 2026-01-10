/*
    ============================================================================
    EJEMPLO 5: SIMULACIÓN DE FÍSICA Y GEOMETRÍA
    ============================================================================
    
    DESCRIPCIÓN:
    Este ejemplo crea un motor de física básico desde cero para simular cuerpos
    rígidos circulares (figuras curvas). No se usan imágenes, todo es
    renderizado geométrico matemático.

    CÓMO USARLO:
    1. Ejecuta el programa. Verás pelotas cayendo por gravedad.
    2. CLICK IZQUIERDO: Genera una nueva pelota en la posición del mouse.
    3. CLICK DERECHO: Crea una "Explosión" de fuerza que empuja las pelotas.
    4. ESPACIO: Pausa / Reanuda la simulación.
    5. 'C': Limpiar todas las pelotas.

    CONCEPTOS CLAVE:
    - Integración de Euler (Velocidad y Aceleración).
    - Detección de Colisiones Círculo-Círculo (Teorema de Pitágoras).
    - Resolución de Colisiones (Rebote elástico y separación estática).
    - Uso de colores con canal Alpha (Transparencia) para efectos visuales.
    ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "engine.h"

// ============================================================================
// CONFIGURACIÓN
// ============================================================================
#define SCREEN_W 800
#define SCREEN_H 600
#define MAX_BALLS 1000

// Propiedades Físicas
#define GRAVITY 500.0f        // Gravedad hacia abajo (pixeles/segundo^2)
#define BOUNCE 0.7f           // Factor de rebote (0.0 = se pega, 1.0 = rebote perpetuo)
#define FRICTION 0.995f       // Fricción del aire (resistencia)

// ============================================================================
// ESTRUCTURAS DE DATOS
// ============================================================================

typedef struct {
    GE_Point pos;   // Posición (x, y)
    GE_Point vel;   // Velocidad (vx, vy)
    float radius;   // Radio del círculo
    float mass;     // Masa (usualmente proporcional al radio)
    GE_Color color; // Color de la pelota
    bool active;    // Si está en uso o no
} Ball;

// ============================================================================
// FUNCIONES AUXILIARES DE FÍSICA
// ============================================================================

// Distancia al cuadrado (más rápido que sqrt para comparaciones)
float DistSqr(GE_Point p1, GE_Point p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return dx*dx + dy*dy;
}

// Resolver colisión entre dos pelotas
void ResolveCollision(Ball* b1, Ball* b2) {
    float dx = b2->pos.x - b1->pos.x;
    float dy = b2->pos.y - b1->pos.y;
    float distance = sqrtf(dx*dx + dy*dy);
    float minDist = b1->radius + b2->radius;

    // Si se están tocando (o traslapando)
    if (distance < minDist && distance > 0.0001f) {
        // 1. Separación Estática (Para que no se queden pegadas)
        // Calculamos cuánto se metió una dentro de la otra
        float overlap = minDist - distance;
        float nx = dx / distance; // Normal X
        float ny = dy / distance; // Normal Y
        
        // Movemos cada una la mitad del traslape en dirección opuesta
        b1->pos.x -= nx * overlap * 0.5f;
        b1->pos.y -= ny * overlap * 0.5f;
        b2->pos.x += nx * overlap * 0.5f;
        b2->pos.y += ny * overlap * 0.5f;

        // 2. Respuesta Dinámica (Rebote)
        // Vector tangente
        float tx = -ny;
        float ty = nx;

        // Producto punto tangente
        float dpTan1 = b1->vel.x * tx + b1->vel.y * ty;
        float dpTan2 = b2->vel.x * tx + b2->vel.y * ty;

        // Producto punto normal (Conservación de momento 1D)
        float dpNorm1 = b1->vel.x * nx + b1->vel.y * ny;
        float dpNorm2 = b2->vel.x * nx + b2->vel.y * ny;

        // Intercambio de momento elástico
        float m1 = (dpNorm1 * (b1->mass - b2->mass) + 2.0f * b2->mass * dpNorm2) / (b1->mass + b2->mass);
        float m2 = (dpNorm2 * (b2->mass - b1->mass) + 2.0f * b1->mass * dpNorm1) / (b1->mass + b2->mass);

        // Actualizar velocidades
        b1->vel.x = tx * dpTan1 + nx * m1;
        b1->vel.y = ty * dpTan1 + ny * m1;
        b2->vel.x = tx * dpTan2 + nx * m2;
        b2->vel.y = ty * dpTan2 + ny * m2;
        
        // Aplicar pérdida de energía por el choque
        b1->vel.x *= BOUNCE; b1->vel.y *= BOUNCE;
        b2->vel.x *= BOUNCE; b2->vel.y *= BOUNCE;
    }
}

// Generar color aleatorio brillante
GE_Color RandomColor() {
    return (0xFF << 24) | ((rand()%200 + 55) << 16) | ((rand()%200 + 55) << 8) | (rand()%200 + 55);
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    GE_Context* ctx = GE_Init("Ejemplo 5: Fisicas y Curvas", SCREEN_W, SCREEN_H);
    if (!ctx) return -1;

    GE_Font* font = GE_LoadFont("assets/fonts/Roboto-Medium.ttf", 20.0f);
    srand((unsigned int)time(NULL));

    // Banco de Pelotas
    Ball balls[MAX_BALLS];
    int ballCount = 0;

    // Crear una pelota grande estática en el centro (Obstáculo)
    balls[ballCount++] = (Ball){ 
        {SCREEN_W/2, SCREEN_H/2}, {0,0}, 60.0f, 10000.0f, 0xFFFFFFFF, true 
    };

    bool paused = false;

    // --- GAME LOOP ---
    while (GE_PollEvents(ctx)) {
        GE_Clear(ctx, 0xFF101020); // Azul oscuro muy profundo
        
        float dt = GE_GetDeltaTime(ctx);
        if (paused) dt = 0; // Si está pausado, el tiempo físico es 0

        GE_Point m = GE_GetMousePosition(ctx);

        // --- INPUT ---
        
        // CLICK IZQ: Spawnear pelota
        if (GE_IsKeyPressed(ctx, GE_MOUSE_LEFT) && ballCount < MAX_BALLS) {
            float r = (float)(rand() % 15 + 10); // Radio entre 10 y 25
            balls[ballCount++] = (Ball){
                m, // Posición mouse
                {(float)(rand()%200 - 100), (float)(rand()%200 - 100)}, // Vel inicial aleatoria
                r,      // Radio
                r * 2.0f, // Masa
                RandomColor(),
                true
            };
        }

        // CLICK DER: Explosión (Repulsión)
        if (GE_IsKeyPressed(ctx, GE_MOUSE_RIGHT)) {
            for (int i = 0; i < ballCount; i++) {
                if (i == 0) continue; // No mover la bola central
                float dx = balls[i].pos.x - m.x;
                float dy = balls[i].pos.y - m.y;
                float dist = sqrtf(dx*dx + dy*dy);
                if (dist < 200.0f && dist > 1.0f) {
                    float force = 100000.0f / dist; // Fuerza inversa a la distancia
                    balls[i].vel.x += (dx / dist) * force * dt;
                    balls[i].vel.y += (dy / dist) * force * dt;
                }
            }
        }

        // ESPACIO: Pausa
        if (GE_IsKeyPressed(ctx, GE_KEY_SPACE)) paused = !paused;

        // C: Limpiar (menos la bola central)
        if (GE_IsKeyPressed(ctx, 82) || GE_IsKeyPressed(ctx, 67)) { // R o C
            ballCount = 1; 
        }

        // --- FÍSICA ---
        // Hacemos varios "sub-steps" para mayor estabilidad si hay muchas bolas
        int subSteps = 4;
        float subDt = dt / subSteps;

        for (int step = 0; step < subSteps; step++) {
            for (int i = 0; i < ballCount; i++) {
                if (i == 0) continue; // La bola 0 es estática
                
                // 1. Gravedad
                balls[i].vel.y += GRAVITY * subDt;
                
                // 2. Fricción del aire
                balls[i].vel.x *= FRICTION;
                balls[i].vel.y *= FRICTION;

                // 3. Movimiento
                balls[i].pos.x += balls[i].vel.x * subDt;
                balls[i].pos.y += balls[i].vel.y * subDt;

                // 4. Colisiones con Paredes (Límites de pantalla)
                if (balls[i].pos.x < balls[i].radius) {
                    balls[i].pos.x = balls[i].radius;
                    balls[i].vel.x *= -BOUNCE;
                }
                if (balls[i].pos.x > SCREEN_W - balls[i].radius) {
                    balls[i].pos.x = SCREEN_W - balls[i].radius;
                    balls[i].vel.x *= -BOUNCE;
                }
                if (balls[i].pos.y < balls[i].radius) {
                    balls[i].pos.y = balls[i].radius;
                    balls[i].vel.y *= -BOUNCE;
                }
                if (balls[i].pos.y > SCREEN_H - balls[i].radius) {
                    balls[i].pos.y = SCREEN_H - balls[i].radius;
                    balls[i].vel.y *= -0.5f; // El suelo absorbe más energía
                }

                // 5. Colisiones Bola contra Bola
                for (int j = i + 1; j < ballCount; j++) {
                    ResolveCollision(&balls[i], &balls[j]);
                }
                // Colisión contra la bola estática central (índice 0)
                ResolveCollision(&balls[0], &balls[i]);
            }
        }

        // --- RENDERIZADO ---
        
        for (int i = 0; i < ballCount; i++) {
            // Dibujar cuerpo relleno
            GE_FillCircle(ctx, (int)balls[i].pos.x, (int)balls[i].pos.y, (int)balls[i].radius, balls[i].color);
            
            // Dibujar brillo (detalle estético para que parezcan esferas)
            GE_FillCircle(ctx, (int)balls[i].pos.x - (int)(balls[i].radius/3), 
                               (int)balls[i].pos.y - (int)(balls[i].radius/3), 
                               (int)(balls[i].radius/4), 0x88FFFFFF);
            
            // Dibujar borde antialiasing (simulado)
            GE_DrawCircle(ctx, (int)balls[i].pos.x, (int)balls[i].pos.y, (int)balls[i].radius, 0xAA000000);
        }

        // --- UI ---
        if (font) {
            GE_DrawTextAligned(ctx, font, "SIMULACION DE FISICA", SCREEN_W/2, 20, GE_ALIGN_CENTER, 0xFFFFFFFF);
            
            char stats[64];
            sprintf(stats, "Objetos: %d", ballCount);
            GE_DrawText(ctx, font, stats, 10, 20, 0xFF00FF00);

            GE_DrawText(ctx, font, "[Click Izq] Spawn  [Click Der] Explosion", 10, SCREEN_H - 50, 0xFFAAAAAA);
            GE_DrawText(ctx, font, "[Espacio] Pausa  [C] Limpiar", 10, SCREEN_H - 25, 0xFFAAAAAA);
            
            if (paused) {
                GE_DrawTextAligned(ctx, font, "- PAUSADO -", SCREEN_W/2, SCREEN_H/2, GE_ALIGN_CENTER, 0xFFFF0000);
            }
        }
    }

    GE_UnloadFont(font);
    GE_Close(ctx);
    return 0;
}
