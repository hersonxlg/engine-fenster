#include <stdio.h>
#include <math.h>
#include "engine.h"

#define SCREEN_W 800
#define SCREEN_H 600
#define POINT_RADIUS 10

// Calcula un punto en la curva Bézier Cúbica
GE_Point CalculateBezier(GE_Point p0, GE_Point p1, GE_Point p2, GE_Point p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    GE_Point p;
    p.x = (uuu * p0.x) + (3 * uu * t * p1.x) + (3 * u * tt * p2.x) + (ttt * p3.x);
    p.y = (uuu * p0.y) + (3 * uu * t * p1.y) + (3 * u * tt * p2.y) + (ttt * p3.y);
    return p;
}

int main() {
    // 1. Inicializar Motor
    GE_Context* ctx = GE_Init("Bezier Curve Demo + Texto", SCREEN_W, SCREEN_H);
    if (!ctx) return -1;

    // 2. Cargar Fuente (Asegúrate de que la ruta coincida con tu carpeta)
    // Usaremos Roboto-Medium.ttf con tamaño 24px
    GE_Font* font = GE_LoadFont("assets/fonts/Roboto-Medium.ttf", 24.0f);
    
    // Verificación de seguridad por si la ruta está mal
    if (!font) {
        printf("ADVERTENCIA: No se pudo cargar la fuente. Verifica la carpeta 'assets'.\n");
    }

    // 3. Definir puntos de control
    GE_Point points[4] = {
        {100, 500}, {100, 100}, {700, 100}, {700, 500}
    };

    int draggingPoint = -1;

    // --- GAME LOOP ---
    while (GE_PollEvents(ctx)) {
        GE_Clear(ctx, 0xFF181818); // Fondo gris oscuro

        // --- INPUT ---
        GE_Point mouse = GE_GetMousePosition(ctx);

        if (GE_IsKeyPressed(ctx, GE_MOUSE_LEFT)) {
            for (int i = 0; i < 4; i++) {
                if (GE_Math_Distance(mouse.x, mouse.y, points[i].x, points[i].y) <= POINT_RADIUS + 5) {
                    draggingPoint = i;
                    break;
                }
            }
        }

        if (GE_IsKeyReleased(ctx, GE_MOUSE_LEFT)) draggingPoint = -1;
        if (draggingPoint != -1) points[draggingPoint] = mouse;

        // --- DIBUJAR CURVA Y LINEAS ---
        GE_DrawLine(ctx, (int)points[0].x, (int)points[0].y, (int)points[1].x, (int)points[1].y, 0xFF555555);
        GE_DrawLine(ctx, (int)points[3].x, (int)points[3].y, (int)points[2].x, (int)points[2].y, 0xFF555555);

        GE_Point prevPoint = points[0];
        int segments = 60;
        for (int i = 1; i <= segments; i++) {
            float t = (float)i / (float)segments;
            GE_Point currentPoint = CalculateBezier(points[0], points[1], points[2], points[3], t);
            GE_DrawLine(ctx, (int)prevPoint.x, (int)prevPoint.y, (int)currentPoint.x, (int)currentPoint.y, 0xFF00FFFF);
            prevPoint = currentPoint;
        }

        // --- DIBUJAR PUNTOS ---
        for (int i = 0; i < 4; i++) {
            GE_Color color = (i == 0 || i == 3) ? 0xFFFF0000 : 0xFF00FF00;
            if (i == draggingPoint) color = 0xFFFFFFFF;
            GE_FillCircle(ctx, (int)points[i].x, (int)points[i].y, POINT_RADIUS, color);
            GE_DrawCircle(ctx, (int)points[i].x, (int)points[i].y, POINT_RADIUS, 0xFF000000);
        }

        // --- DIBUJAR TEXTO INFORMATIVO ---
        if (font) {
            // Título centrado arriba
            GE_DrawTextAligned(ctx, font, "Demo Curva Bezier Interactiva", 
                               SCREEN_W / 2, 40, GE_ALIGN_CENTER, 0xFFFFFFFF);
            
            // Instrucciones centradas abajo (Un poco más pequeñas visualmente por el color gris)
            GE_DrawTextAligned(ctx, font, "Click y Arrastra los puntos para editar la curva", 
                               SCREEN_W / 2, SCREEN_H - 40, GE_ALIGN_CENTER, 0xFFAAAAAA);

            // Coordenadas del mouse (esquina superior izquierda)
            char mouseInfo[32];
            sprintf(mouseInfo, "Mouse: %d, %d", (int)mouse.x, (int)mouse.y);
            GE_DrawText(ctx, font, mouseInfo, 10, 30, 0xFFFFFF00); // Amarillo
        }
    }

    // 4. Limpieza
    GE_UnloadFont(font);
    GE_Close(ctx);
    return 0;
}
