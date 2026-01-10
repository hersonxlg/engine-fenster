// ============================================================================
// ENGINE.C - FASE 3: VERSIÓN ESTABLE (SIN HOOKS COMPLEJOS)
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h> // Agregado para el rand()


#ifdef _WIN32
    #include <windows.h>
#endif

#include "fenster.h" 
#include "engine.h" 

#define MINIAUDIO_IMPLEMENTATION
#include "libs/miniaudio.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "libs/stb_truetype.h"

// ============================================================================
// ESTRUCTURAS
// ============================================================================

struct GE_Context {
    struct fenster f; 
    uint32_t* render_buffer; // Tu lienzo de baja resolución (320x240)
    
    int render_width;
    int render_height;
    
    // Control de Tiempo
    double last_time;
    float delta_time;
    int target_fps;
};

// ============================================================================
// CORE (Init, Close, Poll, Clear)
// ============================================================================

// Función auxiliar para recalcular el tamaño del buffer de ventana
static void GE_UpdateWindowBuffer(GE_Context* ctx) {
    if (ctx->f.buf) free(ctx->f.buf);
    // Asignamos el buffer al tamaño REAL de la ventana
    ctx->f.buf = (uint32_t*)calloc(ctx->f.width * ctx->f.height, sizeof(uint32_t));
}

GE_Context* GE_Init(const char* title, int game_width, int game_height) {
    GE_Context* ctx = (GE_Context*)calloc(1, sizeof(GE_Context));
    if (!ctx) return NULL;

    // 1. Configurar Lienzo del Juego (Render Buffer)
    ctx->render_width = game_width;
    ctx->render_height = game_height;
    ctx->render_buffer = (uint32_t*)calloc(game_width * game_height, sizeof(uint32_t));

    // 2. Configurar Ventana (Fenster)
    // Inicialmente hacemos que la ventana coincida con el juego
    struct fenster temp_f = {
        .title = title,
        .width = game_width,
        .height = game_height,
        .buf = NULL // Lo asignaremos abajo
    };
    memcpy(&ctx->f, &temp_f, sizeof(struct fenster));
    GE_UpdateWindowBuffer(ctx);

    // 3. Abrir Ventana
    fenster_open(&ctx->f);

    // 4. Ajuste Automático de Ventana (Solo Windows)
    #ifdef _WIN32
        // Calculamos bordes para que el área útil sea EXACTA
        RECT rc = { 0, 0, game_width, game_height };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        int total_w = (rc.right - rc.left);
        int total_h = (rc.bottom - rc.top);
        
        // Centrar en pantalla
        int screen_w = GetSystemMetrics(SM_CXSCREEN);
        int screen_h = GetSystemMetrics(SM_CYSCREEN);
        SetWindowPos((HWND)ctx->f.hwnd, NULL, (screen_w - total_w)/2, (screen_h - total_h)/2, total_w, total_h, 0);
    #endif

    ctx->last_time = fenster_time();
    ctx->target_fps = 60;

    return ctx;
}

void GE_Close(GE_Context* ctx) {
    if (ctx) {
        fenster_close(&ctx->f);
        if (ctx->f.buf) free(ctx->f.buf);
        if (ctx->render_buffer) free(ctx->render_buffer);
        free(ctx);
    }
}

// LA FUNCIÓN CRÍTICA: Aquí escalamos y mostramos la imagen
bool GE_PollEvents(GE_Context* ctx) {
    if (!ctx) return false;

    // 1. Control de Tiempo
    int64_t now = fenster_time();
    ctx->delta_time = (float)(now - ctx->last_time) / 1000.0f;
    ctx->last_time = now;

    // 2. ESCALADO (De Render Buffer -> Ventana)
    // Calculamos escala para mantener proporción (Letterboxing)
    float scale_x = (float)ctx->f.width / ctx->render_width;
    float scale_y = (float)ctx->f.height / ctx->render_height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;
    
    // Centrado
    int view_w = (int)(ctx->render_width * scale);
    int view_h = (int)(ctx->render_height * scale);
    int offset_x = (ctx->f.width - view_w) / 2;
    int offset_y = (ctx->f.height - view_h) / 2;

    // Rellenar fondo negro (para las barras laterales si sobran)
    // Solo necesario si la ventana es más ancha que el juego
    memset(ctx->f.buf, 0, ctx->f.width * ctx->f.height * sizeof(uint32_t));

    // Bucle de copiado de píxeles (Render -> Window)
    for (int y = 0; y < view_h; y++) {
        for (int x = 0; x < view_w; x++) {
            // Mapeo inverso: ¿Qué pixel original corresponde a este pixel de pantalla?
            int src_x = (int)(x / scale);
            int src_y = (int)(y / scale);
            
            // Protección de límites
            if (src_x >= ctx->render_width) src_x = ctx->render_width - 1;
            if (src_y >= ctx->render_height) src_y = ctx->render_height - 1;

            // Copiar color
            uint32_t color = ctx->render_buffer[src_y * ctx->render_width + src_x];
            
            // Poner en pantalla
            ctx->f.buf[(offset_y + y) * ctx->f.width + (offset_x + x)] = color;
        }
    }

    // 3. FORZAR ACTUALIZACIÓN VISUAL
    #ifdef _WIN32
        // Esto le dice a Windows: "Pinta la ventana YA"
        InvalidateRect((HWND)ctx->f.hwnd, NULL, FALSE);
    #endif

    // 4. Procesar mensajes de Windows (Inputs, Cierre, etc.)
    if (fenster_loop(&ctx->f) < 0) return false;
    if (ctx->f.keys[27]) return false; // ESC para salir

    // 5. Limitar FPS
    int64_t frame_time = fenster_time() - now;
    int64_t target_ms = 1000 / ctx->target_fps;
    if (frame_time < target_ms) fenster_sleep(target_ms - frame_time);

    return true;
}

void GE_Clear(GE_Context* ctx, GE_Color color) {
    if (!ctx || !ctx->render_buffer) return;
    int count = ctx->render_width * ctx->render_height;
    for (int i = 0; i < count; i++) ctx->render_buffer[i] = color;
}

void GE_SetTargetFPS(GE_Context* ctx, int fps) { if (ctx) ctx->target_fps = fps; }
int GE_GetFPS(GE_Context* ctx) { return (ctx && ctx->delta_time > 0) ? (int)(1.0f / ctx->delta_time) : 0; }
float GE_GetDeltaTime(GE_Context* ctx) { return ctx ? ctx->delta_time : 0.0f; }
double GE_GetTime() { return (double)fenster_time(); }

// ============================================================================
// 5. SISTEMA DE INPUT (CORREGIDO)
// ============================================================================


// CORRECCIÓN 1: Usamos 'GE_InputKey' en lugar de 'int' para coincidir con el .h
bool GE_IsKeyDown(GE_Context* ctx, GE_InputKey key) {
    if (!ctx) return false;

    // --- CORRECCIÓN DE MOUSE PARA WINDOWS (Direct Input) ---
    #ifdef _WIN32
        // 0x8000 verifica si el botón está presionado FÍSICAMENTE
        if (key == GE_MOUSE_LEFT)   return (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        if (key == GE_MOUSE_MIDDLE) return (GetAsyncKeyState(VK_MBUTTON) & 0x8000);
        if (key == GE_MOUSE_RIGHT)  return (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
    #else
        // Fallback para Linux/Mac
        if (key == GE_MOUSE_LEFT)   return (ctx->f.mouse & 1);
        if (key == GE_MOUSE_MIDDLE) return (ctx->f.mouse & 2); 
        if (key == GE_MOUSE_RIGHT)  return (ctx->f.mouse & 4); 
    #endif

    // --- TECLADO ---
    // Casteamos GE_InputKey a int para usarlo como índice
    int k = (int)key;
    if (k < 256) {
        return ctx->f.keys[k];
    }
    
    return false;
}

// CORRECCIÓN 1: También aquí usamos GE_InputKey
bool GE_IsKeyPressed(GE_Context* ctx, GE_InputKey key) { return GE_IsKeyDown(ctx, key); }
bool GE_IsKeyReleased(GE_Context* ctx, GE_InputKey key) { return !GE_IsKeyDown(ctx, key); }

GE_Point GE_GetMousePosition(GE_Context* ctx) {
    if (!ctx) return (GE_Point){0,0};
    
    // 1. Calcular Escala (Igual que en PollEvents)
    float scale_x = (float)ctx->f.width / ctx->render_width;
    float scale_y = (float)ctx->f.height / ctx->render_height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;
    
    // 2. Calcular Márgenes (Offset)
    int view_w = (int)(ctx->render_width * scale);
    int view_h = (int)(ctx->render_height * scale);
    int offset_x = (ctx->f.width - view_w) / 2;
    int offset_y = (ctx->f.height - view_h) / 2;

    // 3. OBTENER COORDENADAS (CORREGIDO: Usamos .x e .y)
    // En fenster, .x e .y SIEMPRE son la posición del mouse relativa a la ventana
    float raw_mouse_x = (float)ctx->f.x;
    float raw_mouse_y = (float)ctx->f.y;

    // 4. Transformar al mundo del juego
    float mx = (raw_mouse_x - offset_x) / scale;
    float my = (raw_mouse_y - offset_y) / scale;

    return (GE_Point){ mx, my };
}

int GE_GetMouseWheel(GE_Context* ctx) { return 0; }

void GE_ShowCursor(bool visible) {
    #ifdef _WIN32
        if (visible) {
            while (ShowCursor(TRUE) < 0); 
        } else {
            while (ShowCursor(FALSE) >= 0); 
        }
    #endif
}

// ============================================================================
// PRIMITIVAS GRÁFICAS (DrawPixel, Lines, Rects, Circles...)
// ============================================================================

// Helper seguro
static void GE_PutPixelSafe(GE_Context* ctx, int x, int y, GE_Color color) {
    if (x < 0 || x >= ctx->render_width || y < 0 || y >= ctx->render_height) return;
    if ((color >> 24) == 0) return; // Transparente
    ctx->render_buffer[y * ctx->render_width + x] = color;
}

void GE_DrawPixel(GE_Context* ctx, float x, float y, GE_Color color) {
    if (!ctx || !ctx->render_buffer) return;
    GE_PutPixelSafe(ctx, (int)x, (int)y, color);
}

void GE_DrawLine(GE_Context* ctx, float x1, float y1, float x2, float y2, GE_Color color) {
    int x0 = (int)x1, y0 = (int)y1;
    int xEnd = (int)x2, yEnd = (int)y2;
    int dx = abs(xEnd - x0), sx = x0 < xEnd ? 1 : -1;
    int dy = -abs(yEnd - y0), sy = y0 < yEnd ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        GE_PutPixelSafe(ctx, x0, y0, color);
        if (x0 == xEnd && y0 == yEnd) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void GE_DrawLineThick(GE_Context* ctx, float x1, float y1, float x2, float y2, float thickness, GE_Color color) {
    float angle = atan2(y2 - y1, x2 - x1);
    float dx = sin(angle);
    float dy = cos(angle);
    for(float i = -thickness/2; i < thickness/2; i += 0.5f) {
        GE_DrawLine(ctx, x1 + i*dx, y1 - i*dy, x2 + i*dx, y2 - i*dy, color);
    }
}

void GE_DrawRect(GE_Context* ctx, float x, float y, float w, float h, GE_Color color) {
    GE_DrawLine(ctx, x, y, x+w, y, color);
    GE_DrawLine(ctx, x, y+h, x+w, y+h, color);
    GE_DrawLine(ctx, x, y, x, y+h, color);
    GE_DrawLine(ctx, x+w, y, x+w, y+h, color);
}

void GE_FillRect(GE_Context* ctx, float x, float y, float w, float h, GE_Color color) {
    int ix = (int)x, iy = (int)y, iw = (int)w, ih = (int)h;
    for (int j = 0; j < ih; j++) {
        for (int i = 0; i < iw; i++) {
            GE_PutPixelSafe(ctx, ix + i, iy + j, color);
        }
    }
}

// Círculos (Midpoint)
void GE_DrawCircle(GE_Context* ctx, float cx, float cy, float radius, GE_Color color) {
    if (radius <= 0) { GE_DrawPixel(ctx, cx, cy, color); return; }
    int x0 = (int)cx, y0 = (int)cy, r = (int)radius;
    int x = r, y = 0, err = 0;
    while (x >= y) {
        GE_PutPixelSafe(ctx, x0 + x, y0 + y, color); GE_PutPixelSafe(ctx, x0 + y, y0 + x, color);
        GE_PutPixelSafe(ctx, x0 - y, y0 + x, color); GE_PutPixelSafe(ctx, x0 - x, y0 + y, color);
        GE_PutPixelSafe(ctx, x0 - x, y0 - y, color); GE_PutPixelSafe(ctx, x0 - y, y0 - x, color);
        GE_PutPixelSafe(ctx, x0 + y, y0 - x, color); GE_PutPixelSafe(ctx, x0 + x, y0 - y, color);
        if (err <= 0) { y += 1; err += 2 * y + 1; }
        if (err > 0) { x -= 1; err -= 2 * x + 1; }
    }
}

void GE_FillCircle(GE_Context* ctx, float cx, float cy, float radius, GE_Color color) {
    if (radius <= 0) { GE_DrawPixel(ctx, cx, cy, color); return; }
    int x0 = (int)cx, y0 = (int)cy, r = (int)radius;
    int x = r, y = 0, err = 0;
    while (x >= y) {
        GE_DrawLine(ctx, x0 - x, y0 - y, x0 + x, y0 - y, color);
        GE_DrawLine(ctx, x0 - x, y0 + y, x0 + x, y0 + y, color);
        GE_DrawLine(ctx, x0 - y, y0 - x, x0 + y, y0 - x, color);
        GE_DrawLine(ctx, x0 - y, y0 + x, x0 + y, y0 + x, color);
        if (err <= 0) { y += 1; err += 2 * y + 1; }
        if (err > 0) { x -= 1; err -= 2 * x + 1; }
    }
}

void GE_DrawEllipse(GE_Context* ctx, float cx, float cy, float rx, float ry, GE_Color color) {
    for (int i = 0; i < 360; i++) {
        float rad = i * 3.14159f / 180.0f;
        float rad_next = (i + 1) * 3.14159f / 180.0f;
        GE_DrawLine(ctx, cx + cos(rad)*rx, cy + sin(rad)*ry, cx + cos(rad_next)*rx, cy + sin(rad_next)*ry, color);
    }
}

void GE_FillEllipse(GE_Context* ctx, float cx, float cy, float rx, float ry, GE_Color color) {
    int irx = (int)rx, iry = (int)ry;
    for(int y = -iry; y <= iry; y++) {
        for(int x = -irx; x <= irx; x++) {
            if(((float)(x*x)/(rx*rx)) + ((float)(y*y)/(ry*ry)) <= 1.0f)
                GE_PutPixelSafe(ctx, (int)cx + x, (int)cy + y, color);
        }
    }
}

void GE_DrawTriangle(GE_Context* ctx, float x1, float y1, float x2, float y2, float x3, float y3, GE_Color color) {
    GE_DrawLine(ctx, x1, y1, x2, y2, color);
    GE_DrawLine(ctx, x2, y2, x3, y3, color);
    GE_DrawLine(ctx, x3, y3, x1, y1, color);
}

void GE_DrawPolygon(GE_Context* ctx, GE_Point* points, int count, GE_Color color) {
    for (int i = 0; i < count - 1; i++) {
        GE_DrawLine(ctx, points[i].x, points[i].y, points[i+1].x, points[i+1].y, color);
    }
    GE_DrawLine(ctx, points[count-1].x, points[count-1].y, points[0].x, points[0].y, color);
}

// --- UTILIDADES MATEMÁTICAS ---

float GE_Math_Distance(float x1, float y1, float x2, float y2) {
    return sqrtf(powf(x2 - x1, 2) + powf(y2 - y1, 2));
}

float GE_Math_Lerp(float start, float end, float t) {
    return start + t * (end - start);
}

GE_Color GE_Color_FromRGB(uint8_t r, uint8_t g, uint8_t b) {
    // Formato 0xAARRGGBB (Alpha 255 por defecto)
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

GE_Color GE_Color_FromHex(uint32_t hex) {
    return hex; // Asumimos que ya viene en formato correcto o RGB
}

// --- TRIÁNGULOS Y POLÍGONOS (RELLENO) ---

// Función auxiliar para calcular el área con signo (para coordenadas baricéntricas)
static float EdgeFunction(GE_Point a, GE_Point b, GE_Point p) {
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

void GE_FillTriangle(GE_Context* ctx, float x1, float y1, float x2, float y2, float x3, float y3, GE_Color color) {
    // Usamos el algoritmo de Caja Delimitadora (Bounding Box) + Baricéntricas
    // Es robusto y evita huecos entre triángulos adyacentes.
    
    int minX = (int)fminf(x1, fminf(x2, x3));
    int minY = (int)fminf(y1, fminf(y2, y3));
    int maxX = (int)fmaxf(x1, fmaxf(x2, x3));
    int maxY = (int)fmaxf(y1, fmaxf(y2, y3));

    // Clipping con la pantalla
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= ctx->render_width) maxX = ctx->render_width - 1;
    if (maxY >= ctx->render_height) maxY = ctx->render_height - 1;

    GE_Point p1 = {x1, y1};
    GE_Point p2 = {x2, y2};
    GE_Point p3 = {x3, y3};

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            GE_Point p = { (float)x + 0.5f, (float)y + 0.5f }; // Centro del pixel
            
            // Coordenadas baricéntricas
            float w0 = EdgeFunction(p2, p3, p);
            float w1 = EdgeFunction(p3, p1, p);
            float w2 = EdgeFunction(p1, p2, p);
            
            // Si el punto está "a la derecha" de todos los bordes, está dentro
            // (Manejamos winding order chequeando si todos son positivos O todos negativos)
            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                GE_PutPixelSafe(ctx, x, y, color);
            }
        }
    }
}

void GE_FillPolygon(GE_Context* ctx, GE_Point* points, int count, GE_Color color) {
    // Método simple: Abanico de triángulos (Triangle Fan) desde el primer punto.
    // NOTA: Esto solo funciona bien para polígonos CONVEXOS.
    // Para cóncavos se requiere algoritmos más complejos (Scanline o Tesselation).
    if (count < 3) return;
    
    for (int i = 1; i < count - 1; i++) {
        GE_FillTriangle(ctx, points[0].x, points[0].y, 
                             points[i].x, points[i].y, 
                             points[i+1].x, points[i+1].y, color);
    }
}

// --- ARCOS Y SECTORES ---

void GE_DrawArc(GE_Context* ctx, float cx, float cy, float radius, float start_deg, float end_deg, GE_Color color) {
    float step = 10.0f / radius; // Paso dinámico según el tamaño para suavidad
    if (step > 0.1f) step = 0.1f; // Mínimo de calidad
    
    float start_rad = start_deg * 3.14159f / 180.0f;
    float end_rad = end_deg * 3.14159f / 180.0f;
    
    for (float angle = start_rad; angle < end_rad; angle += step) {
        float x = cx + cosf(angle) * radius;
        float y = cy + sinf(angle) * radius;
        GE_DrawPixel(ctx, x, y, color);
    }
}

void GE_FillSector(GE_Context* ctx, float cx, float cy, float radius, float start_deg, float end_deg, GE_Color color) {
    // Dibujamos líneas desde el centro hacia el borde del arco (como rebanadas de pizza muy finas)
    // No es el método más rápido (overdraw), pero es muy sencillo de implementar.
    float step = 1.0f / radius; // Paso muy fino para que no queden huecos
    if (step > 0.01f) step = 0.01f;

    float start_rad = start_deg * 3.14159f / 180.0f;
    float end_rad = end_deg * 3.14159f / 180.0f;

    for (float angle = start_rad; angle <= end_rad; angle += step) {
        float x = cx + cosf(angle) * radius;
        float y = cy + sinf(angle) * radius;
        GE_DrawLine(ctx, cx, cy, x, y, color);
    }
}

// --- CURVAS DE BÉZIER ---

void GE_DrawBezierQuad(GE_Context* ctx, GE_Point p0, GE_Point p1, GE_Point p2, int segments, GE_Color color) {
    GE_Point prev = p0;
    for (int i = 1; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;
        
        // Fórmula Cuadrática: (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
        float x = (u*u * p0.x) + (2*u*t * p1.x) + (t*t * p2.x);
        float y = (u*u * p0.y) + (2*u*t * p1.y) + (t*t * p2.y);
        
        GE_DrawLine(ctx, prev.x, prev.y, x, y, color);
        prev = (GE_Point){x, y};
    }
}

void GE_DrawBezierCubic(GE_Context* ctx, GE_Point p0, GE_Point p1, GE_Point p2, GE_Point p3, int segments, GE_Color color) {
    GE_Point prev = p0;
    for (int i = 1; i <= segments; i++) {
        float t = (float)i / (float)segments;
        float u = 1.0f - t;
        float u2 = u*u; float u3 = u*u*u;
        float t2 = t*t; float t3 = t*t*t;

        // Fórmula Cúbica
        float x = (u3 * p0.x) + (3*u2*t * p1.x) + (3*u*t2 * p2.x) + (t3 * p3.x);
        float y = (u3 * p0.y) + (3*u2*t * p1.y) + (3*u*t2 * p2.y) + (t3 * p3.y);

        GE_DrawLine(ctx, prev.x, prev.y, x, y, color);
        prev = (GE_Point){x, y};
    }
}

// --- CÁMARA 2D ---

void GE_BeginMode2D(GE_Context* ctx, GE_Camera camera) {
    // En un motor de software 2D simple sin matriz de transformación global,
    // el manejo de cámara suele hacerse modificando las coordenadas antes de dibujar.
    // Como estamos dibujando pixel a pixel directamente, la forma más fácil de implementar
    // esto en el futuro es aplicar la transformación en cada DrawPixel/DrawLine.
    
    // POR AHORA: Guardamos la cámara en el contexto (necesitaríamos extender struct Context)
    // O implementamos funciones de conversión manual WorldToScreen.
    // Esta función es un placeholder para cuando hagamos un renderer más avanzado.
}

void GE_EndMode2D(GE_Context* ctx) {
    // Resetear transformaciones
}

GE_Point GE_WorldToScreen(GE_Camera camera, GE_Point world_pos) {
    // 1. Trasladar al origen relativo al objetivo
    float dx = world_pos.x - camera.target.x;
    float dy = world_pos.y - camera.target.y;

    // 2. Rotar
    float rad = camera.rotation * 3.14159f / 180.0f;
    float cos_res = cosf(rad);
    float sin_res = sinf(rad);
    
    float rot_x = dx * cos_res - dy * sin_res;
    float rot_y = dx * sin_res + dy * cos_res;

    // 3. Escalar (Zoom) y Trasladar al centro de pantalla (Offset)
    return (GE_Point){
        (rot_x * camera.zoom) + camera.offset.x,
        (rot_y * camera.zoom) + camera.offset.y
    };
}

GE_Point GE_ScreenToWorld(GE_Camera camera, GE_Point screen_pos) {
    // Inversa de WorldToScreen
    float dx = screen_pos.x - camera.offset.x;
    float dy = screen_pos.y - camera.offset.y;

    // Deshacer Zoom
    dx /= camera.zoom;
    dy /= camera.zoom;

    // Deshacer Rotación
    float rad = -camera.rotation * 3.14159f / 180.0f;
    float cos_res = cosf(rad);
    float sin_res = sinf(rad);

    float rot_x = dx * cos_res - dy * sin_res;
    float rot_y = dx * sin_res + dy * cos_res;

    return (GE_Point){
        rot_x + camera.target.x,
        rot_y + camera.target.y
    };
}

// --- STUBS TEMPORALES PARA SPRITES / TEXTO / AUDIO ---
// (Estas funciones requieren librerías externas stb_image / miniaudio)
// Las dejamos vacías para que compile, pero avisando.

//GE_Sprite* GE_LoadSprite(const char* f) { printf("[GE] Error: Sprites no implementados aun.\n"); return NULL; }

int GE_GetWidth(GE_Context* ctx) { return ctx ? ctx->render_width : 0; }
int GE_GetHeight(GE_Context* ctx) { return ctx ? ctx->render_height : 0; }


// ============================================================================
// 6. IMPLEMENTACIÓN DE SPRITES Y ANIMACIONES
// ============================================================================

struct GE_Sprite {
    int width;
    int height;
    int channels;       // Generalmente 4 (RGBA)
    unsigned char* data; // Píxeles crudos (byte a byte: R, G, B, A, R, G, B, A...)
};

// --- Helper para mezclar colores (Alpha Blending) ---
// Mezcla un color nuevo (fg) sobre el color existente (bg) respetando la transparencia
static GE_Color GE_BlendColors(GE_Color bg, GE_Color fg, uint8_t alpha_sprite) {
    // Si el sprite es totalmente transparente, no hacemos nada
    if (alpha_sprite == 0) return bg;
    
    // Extraer componentes del Fondo (Background)
    uint8_t bg_a = (bg >> 24) & 0xFF;
    uint8_t bg_r = (bg >> 16) & 0xFF;
    uint8_t bg_g = (bg >> 8)  & 0xFF;
    uint8_t bg_b = bg & 0xFF;

    // Extraer componentes del Frente (Foreground/Sprite)
    uint8_t fg_a = (fg >> 24) & 0xFF;
    uint8_t fg_r = (fg >> 16) & 0xFF;
    uint8_t fg_g = (fg >> 8)  & 0xFF;
    uint8_t fg_b = fg & 0xFF;

    // Combinar el alpha del píxel con el alpha global (si quisiéramos tintes semitransparentes)
    // Por ahora usamos el alpha del píxel directo.
    
    // Fórmula estándar de Alpha Blending:
    // Out = (Alpha * Fg + (255 - Alpha) * Bg) / 255
    uint16_t alpha = alpha_sprite;
    uint16_t inv_alpha = 255 - alpha;

    uint8_t out_r = (uint8_t)((alpha * fg_r + inv_alpha * bg_r) / 255);
    uint8_t out_g = (uint8_t)((alpha * fg_g + inv_alpha * bg_g) / 255);
    uint8_t out_b = (uint8_t)((alpha * fg_b + inv_alpha * bg_b) / 255);
    uint8_t out_a = 255; // El resultado en pantalla siempre es opaco

    return (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
}

GE_Sprite* GE_LoadSprite(const char* filepath) {
    GE_Sprite* spr = (GE_Sprite*)malloc(sizeof(GE_Sprite));
    if (!spr) return NULL;

    // Forzamos 4 canales (RGBA) para facilitar el dibujo
    spr->data = stbi_load(filepath, &spr->width, &spr->height, &spr->channels, 4);
    
    if (!spr->data) {
        printf("[GE] Error cargando sprite: %s\n", filepath);
        free(spr);
        return NULL;
    }
    
    return spr;
}

void GE_UnloadSprite(GE_Sprite* sprite) {
    if (sprite) {
        if (sprite->data) stbi_image_free(sprite->data);
        free(sprite);
    }
}

int GE_GetSpriteWidth(GE_Sprite* sprite) {
    return sprite ? sprite->width : 0;
}

int GE_GetSpriteHeight(GE_Sprite* sprite) {
    return sprite ? sprite->height : 0;
}

void GE_DrawSprite(GE_Context* ctx, GE_Sprite* sprite, float x, float y, GE_Color tint) {
    if (!sprite || !sprite->data) return;
    
    // Usamos DrawSpriteEx para no repetir código, asumiendo rect completo
    GE_Rect src = { 0, 0, (float)sprite->width, (float)sprite->height };
    GE_Rect dst = { x, y, (float)sprite->width, (float)sprite->height };
    GE_DrawSpriteEx(ctx, sprite, src, dst, tint);
}

void GE_DrawSpriteEx(GE_Context* ctx, GE_Sprite* sprite, GE_Rect src, GE_Rect dest, GE_Color tint) {
    if (!ctx || !sprite || !sprite->data) return;

    // Coordenadas enteras para iterar
    int dest_x = (int)dest.x;
    int dest_y = (int)dest.y;
    int dest_w = (int)dest.w;
    int dest_h = (int)dest.h;

    int src_x = (int)src.x;
    int src_y = (int)src.y;
    int src_w = (int)src.w;
    
    // Clipping básico: Si está totalmente fuera de pantalla, no dibujamos
    if (dest_x >= ctx->render_width || dest_y >= ctx->render_height || 
        dest_x + dest_w <= 0 || dest_y + dest_h <= 0) return;

    // Tinte (Tint) - Extraer componentes para multiplicar
    uint8_t tint_r = (tint >> 16) & 0xFF;
    uint8_t tint_g = (tint >> 8)  & 0xFF;
    uint8_t tint_b = tint & 0xFF;

    // Recorremos los píxeles de DESTINO (Pantalla)
    for (int dy = 0; dy < dest_h; dy++) {
        int screen_y = dest_y + dy;
        if (screen_y < 0 || screen_y >= ctx->render_height) continue;

        // Calculamos qué fila del sprite corresponde (Nearest Neighbor scaling)
        int sy = (dy * (int)src.h) / dest_h;
        int sprite_row_offset = (src_y + sy) * sprite->width * 4;

        for (int dx = 0; dx < dest_w; dx++) {
            int screen_x = dest_x + dx;
            if (screen_x < 0 || screen_x >= ctx->render_width) continue;

            // Calculamos columna del sprite
            int sx = (dx * (int)src.w) / dest_w;
            int sprite_idx = sprite_row_offset + ((src_x + sx) * 4);

            // Leer color del sprite
            uint8_t r = sprite->data[sprite_idx + 0];
            uint8_t g = sprite->data[sprite_idx + 1];
            uint8_t b = sprite->data[sprite_idx + 2];
            uint8_t a = sprite->data[sprite_idx + 3];

            if (a == 0) continue; // Totalmente transparente, saltar

            // Aplicar Tinte (Multiplicativo)
            if (tint != 0xFFFFFFFF) {
                r = (r * tint_r) / 255;
                g = (g * tint_g) / 255;
                b = (b * tint_b) / 255;
            }

            GE_Color fg_color = (a << 24) | (r << 16) | (g << 8) | b;
            
            // Obtener color actual de fondo para mezclar
            GE_Color bg_color = ctx->render_buffer[screen_y * ctx->render_width + screen_x];
            
            // Mezclar y pintar
            ctx->render_buffer[screen_y * ctx->render_width + screen_x] = GE_BlendColors(bg_color, fg_color, a);
        }
    }
}

// Nota: DrawSpritePro (Rotación) requiere matemáticas de textura complejas 
// (inverse mapping) que son lentas en CPU pura. Por ahora usaremos el Ex (Escalado/Recorte).
void GE_DrawSpritePro(GE_Context* ctx, GE_Sprite* sprite, GE_Rect src, GE_Rect dest, GE_Point origin, float rotation, GE_Color tint) {
    // TODO: Implementar rotación por software (Bilineal o Nearest Neighbor rotado)
    // Por ahora, redirigimos a Ex ignorando rotación para que no crashee.
    GE_Rect final_dest = { dest.x - origin.x, dest.y - origin.y, dest.w, dest.h };
    GE_DrawSpriteEx(ctx, sprite, src, final_dest, tint);
}

void GE_DrawSpriteQuad(GE_Context* c, GE_Sprite* s, GE_Point p1, GE_Point p2, GE_Point p3, GE_Point p4, GE_Color t) {
    // Placeholder para deformación de sprites (Quad warping)
}

// --- SISTEMA DE ANIMACIÓN ---

GE_Animation GE_CreateAnimation(GE_Sprite* sprite, int frame_w, int frame_h, float duration) {
    GE_Animation anim = {0};
    anim.sprite = sprite;
    anim.frame_w = frame_w;
    anim.frame_h = frame_h;
    anim.frame_duration = duration;
    
    if (sprite) {
        // Calcular cuántos frames caben horizontalmente
        int cols = sprite->width / frame_w;
        // int rows = sprite->height / frame_h; 
        // Asumimos spritesheet horizontal simple por defecto, pero se puede configurar
        anim.start_frame = 0;
        anim.end_frame = cols - 1;
    }
    
    anim.active = true;
    anim.loop = true;
    return anim;
}

void GE_UpdateAnimation(GE_Animation* anim, float dt) {
    if (!anim || !anim->active) return;

    anim->timer += dt;
    if (anim->timer >= anim->frame_duration) {
        anim->timer = 0;
        anim->current_frame++;
        
        if (anim->current_frame > anim->end_frame) {
            if (anim->loop) {
                anim->current_frame = anim->start_frame;
            } else {
                anim->current_frame = anim->end_frame;
                anim->active = false; // Detener si no es loop
            }
        }
    }
}

void GE_DrawAnimation(GE_Context* ctx, GE_Animation* anim, float x, float y, bool flip_x, GE_Color tint) {
    if (!anim || !anim->sprite) return;

    int cols = anim->sprite->width / anim->frame_w;
    if (cols == 0) cols = 1;

    // Calcular posición del frame en la textura
    int col = anim->current_frame % cols;
    int row = anim->current_frame / cols;

    GE_Rect src = {
        (float)(col * anim->frame_w),
        (float)(row * anim->frame_h),
        (float)anim->frame_w,
        (float)anim->frame_h
    };

    GE_Rect dest = { x, y, (float)anim->frame_w, (float)anim->frame_h };
    
    // Si queremos invertir horizontalmente (Flip X)
    // El truco en DrawSpriteEx sería pasar width negativo, pero nuestra implementación
    // de DrawSpriteEx asume positivos. Para un flip real necesitaríamos ajustar los bucles.
    // Por simplicidad, dibujamos normal por ahora.
    
    GE_DrawSpriteEx(ctx, anim->sprite, src, dest, tint);
}

//int GE_GetSpriteWidth(GE_Sprite* sprite) { return sprite ? sprite->width : 0; }
//int GE_GetSpriteHeight(GE_Sprite* sprite) { return sprite ? sprite->height : 0; }

// ============================================================================
// 7. IMPLEMENTACIÓN DE TEXTO (STB_TRUETYPE)
// ============================================================================

/*
struct GE_Font {
    stbtt_bakedchar cdata[96]; // Información de caracteres ASCII 32..126
    GE_Sprite* texture;        // El atlas de la fuente convertido a Sprite
    float size;
};

GE_Font* GE_LoadFont(const char* filepath, float size) {
    // 1. Leer el archivo .ttf completo a memoria
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        printf("[GE] Error: No se pudo abrir la fuente '%s'\n", filepath);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char* ttf_buffer = (unsigned char*)malloc(fsize);
    fread(ttf_buffer, 1, fsize, f);
    fclose(f);

    // 2. Crear un bitmap temporal para el "Atlas" de la fuente
    int width = 512;
    int height = 512;
    unsigned char* temp_bitmap = (unsigned char*)malloc(width * height);

    GE_Font* font = (GE_Font*)calloc(1, sizeof(GE_Font));
    font->size = size;

    // 3. "Hornear" (Bake) los caracteres al bitmap
    // Esto dibuja todas las letras en una sola imagen en blanco y negro
    stbtt_BakeFontBitmap(ttf_buffer, 0, size, temp_bitmap, width, height, 32, 96, font->cdata);
    
    free(ttf_buffer); // Ya no necesitamos el archivo original

    // 4. Convertir el bitmap de 1 canal (Gris) a nuestro formato de Sprite (RGBA)
    font->texture = (GE_Sprite*)malloc(sizeof(GE_Sprite));
    font->texture->width = width;
    font->texture->height = height;
    font->texture->channels = 4;
    font->texture->data = (unsigned char*)malloc(width * height * 4);

    for (int i = 0; i < width * height; i++) {
        uint8_t value = temp_bitmap[i];
        // Lo convertimos a blanco puro, usando el valor como Alpha
        font->texture->data[i*4 + 0] = 255; // R
        font->texture->data[i*4 + 1] = 255; // G
        font->texture->data[i*4 + 2] = 255; // B
        font->texture->data[i*4 + 3] = value; // Alpha
    }

    free(temp_bitmap);
    return font;
}

void GE_UnloadFont(GE_Font* font) {
    if (font) {
        if (font->texture) GE_UnloadSprite(font->texture);
        free(font);
    }
}

void GE_DrawText(GE_Context* ctx, GE_Font* font, const char* text, float x, float y, GE_Color color) {
    if (!font || !text) return;

    // Extraer componentes del color para tintar
    // (Aprovechamos que GE_DrawSpriteEx ya soporta tintes)
    
    float startX = x;

    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            // Obtener coordenadas del caracter dentro del atlas
            stbtt_GetBakedQuad(font->cdata, 512, 512, *text - 32, &x, &y, &q, 1);
            
            // Convertir coordenadas de Quad a Rect
            GE_Rect src = { q.s0 * 512, q.t0 * 512, (q.s1 - q.s0) * 512, (q.t1 - q.t0) * 512 };
            GE_Rect dst = { q.x0, q.y0, q.x1 - q.x0, q.y1 - q.y0 };

            // Dibujar la letra usando nuestro sistema de Sprites existente
            GE_DrawSpriteEx(ctx, font->texture, src, dst, color);
        }
        text++;
    }
}


GE_Point GE_MeasureText(GE_Font* font, const char* text) {
    if (!font || !text) return (GE_Point){0,0};
    
    float x = 0;
    float y = 0;
    
    // Recorremos el texto char a char
    const char* ptr = text;
    while (*ptr) {
        if (*ptr >= 32 && *ptr < 128) {
            stbtt_aligned_quad q;
            // GetBakedQuad avanza la variable 'x' automáticamente
            stbtt_GetBakedQuad(font->cdata, 512, 512, *ptr - 32, &x, &y, &q, 1);
        }
        ptr++;
    }
    
    // x ahora contiene el ancho total del texto
    // Asumimos una altura estándar basada en el tamaño de fuente (aprox)
    return (GE_Point){ x, 32.0f }; 
}

// Stub para alineación (puedes implementarlo calculando MeasureText primero)
void GE_DrawTextAligned(GE_Context* c, GE_Font* f, const char* t, float x, float y, GE_TextAlign align, GE_Color col) {
    if (!c || !f || !t) return;

    // 1. Si es alineación izquierda, dibujamos normal y salimos (ahorra cálculos)
    if (align == GE_ALIGN_LEFT) {
        GE_DrawText(c, f, t, x, y, col);
        return;
    }

    // 2. Medimos el ancho del texto
    GE_Point size = GE_MeasureText(f, t);
    
    // 3. Ajustamos la posición X según la alineación
    float finalX = x;
    
    if (align == GE_ALIGN_CENTER) {
        finalX = x - (size.x / 2.0f); // Restamos la mitad del ancho
    } 
    else if (align == GE_ALIGN_RIGHT) {
        finalX = x - size.x;          // Restamos todo el ancho
    }
    
    // 4. Dibujamos en la nueva posición calculada
    GE_DrawText(c, f, t, finalX, y, col);
}

*/


// ============================================================================
// 8. IMPLEMENTACIÓN DE AUDIO (MINIAUDIO)
// ============================================================================

// Estructura interna para manejar el motor de audio global
// Usamos una variable global estática para el motor porque miniaudio lo requiere
static ma_engine g_audioEngine;
static bool g_audioInitialized = false;

// Función interna para iniciar el motor si no está iniciado
static void GE_InitAudioSystem() {
    if (!g_audioInitialized) {
        if (ma_engine_init(NULL, &g_audioEngine) == MA_SUCCESS) {
            g_audioInitialized = true;
            printf("[GE] Sistema de Audio Inicializado.\n");
        } else {
            printf("[GE] Error: Fallo al iniciar Audio.\n");
        }
    }
}

struct GE_Sound {
    ma_sound sound; // Objeto de sonido de miniaudio
    bool loaded;
};

GE_Sound* GE_LoadSound(const char* filepath) {
    GE_InitAudioSystem(); // Asegurar que el sistema existe
    if (!g_audioInitialized) return NULL;

    GE_Sound* s = (GE_Sound*)malloc(sizeof(GE_Sound));
    
    // Cargar sonido decodificándolo directamente (Stream es mejor para música, Decode para SFX)
    // Aquí usamos MA_SOUND_FLAG_DECODE para cargarlo en RAM (mejor para efectos cortos)
    ma_result result = ma_sound_init_from_file(&g_audioEngine, filepath, MA_SOUND_FLAG_DECODE, NULL, NULL, &s->sound);
    
    if (result != MA_SUCCESS) {
        printf("[GE] Error cargando sonido: %s\n", filepath);
        free(s);
        return NULL;
    }
    
    s->loaded = true;
    return s;
}

void GE_UnloadSound(GE_Sound* sound) {
    if (sound && sound->loaded) {
        ma_sound_uninit(&sound->sound);
        free(sound);
    }
}

void GE_PlaySound(GE_Sound* sound) {
    if (sound && sound->loaded) {
        // Si ya está sonando, lo reiniciamos (para disparos rápidos)
        if (ma_sound_is_playing(&sound->sound)) {
            ma_sound_seek_to_pcm_frame(&sound->sound, 0);
        }
        ma_sound_start(&sound->sound);
    }
}

void GE_StopSound(GE_Sound* sound) {
    if (sound && sound->loaded) {
        ma_sound_stop(&sound->sound);
        ma_sound_seek_to_pcm_frame(&sound->sound, 0); // Rebobinar al inicio
    }
}

void GE_SetVolume(GE_Sound* sound, float volume) {
    if (sound && sound->loaded) {
        ma_sound_set_volume(&sound->sound, volume);
    }
}

void GE_SetLooping(GE_Sound* sound, bool loop) {
    if (sound && sound->loaded) {
        ma_sound_set_looping(&sound->sound, loop);
    }
}

// Nota: Deberíamos tener una función GE_CloseAudio() para llamar a ma_engine_uninit
// Puedes agregarla dentro de GE_Close() si quieres ser estricto con la memoria.

// ============================================================================
// 7. SISTEMA DE TEXTO (CORREGIDO Y DEFINITIVO)
// ============================================================================

// 1. Definición de la Estructura (Debe ir PRIMERO)
struct GE_Font {
    stbtt_bakedchar cdata[96]; // Datos de los caracteres (ASCII 32 a 126)
    GE_Sprite* texture;        // El atlas de la fuente (imagen)
    float size;                // Tamaño base
};

// 2. Función de Carga
GE_Font* GE_LoadFont(const char* filepath, float size) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        printf("[GE] ERROR CRITICO: No se encontro la fuente '%s'.\n", filepath);
        return NULL; 
    }
    
    // Leer archivo completo
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* ttf_buffer = (unsigned char*)malloc(fsize);
    fread(ttf_buffer, 1, fsize, f);
    fclose(f);

    // Crear bitmap temporal
    int width = 512;
    int height = 512;
    unsigned char* temp_bitmap = (unsigned char*)malloc(width * height);

    GE_Font* font = (GE_Font*)calloc(1, sizeof(GE_Font));
    font->size = size;

    // "Hornear" caracteres en el bitmap
    stbtt_BakeFontBitmap(ttf_buffer, 0, size, temp_bitmap, width, height, 32, 96, font->cdata);
    free(ttf_buffer); 

    // Convertir bitmap gris a Sprite RGBA (Blanco con Alpha)
    font->texture = (GE_Sprite*)malloc(sizeof(GE_Sprite));
    font->texture->width = width;
    font->texture->height = height;
    font->texture->channels = 4;
    font->texture->data = (unsigned char*)malloc(width * height * 4);

    for (int i = 0; i < width * height; i++) {
        font->texture->data[i*4 + 0] = 255; // R
        font->texture->data[i*4 + 1] = 255; // G
        font->texture->data[i*4 + 2] = 255; // B
        font->texture->data[i*4 + 3] = temp_bitmap[i]; // Alpha (transparencia)
    }

    free(temp_bitmap);
    return font;
}

void GE_UnloadFont(GE_Font* font) {
    if (font) {
        if (font->texture) GE_UnloadSprite(font->texture);
        free(font);
    }
}

// 3. Función de Medición (CRUCIAL PARA ALINEAR)
GE_Point GE_MeasureText(GE_Font* font, const char* text) {
    if (!font || !text) return (GE_Point){0,0};
    
    float x = 0;
    float y = 0;
    const char* ptr = text;
    
    while (*ptr) {
        if (*ptr >= 32 && *ptr < 128) {
            stbtt_aligned_quad q;
            // GetBakedQuad AVANZA la variable 'x' automáticamente
            stbtt_GetBakedQuad(font->cdata, 512, 512, *ptr - 32, &x, &y, &q, 1);
        }
        ptr++;
    }
    // x contiene el ancho total acumulado
    return (GE_Point){ x, font->size }; 
}

// 4. Función de Dibujado Simple
void GE_DrawText(GE_Context* ctx, GE_Font* font, const char* text, float x, float y, GE_Color color) {
    if (!font || !text) return;

    float startX = x; // Guardamos X original por si quisiéramos saltos de línea (futuro)
    const char* ptr = text;

    while (*ptr) {
        if (*ptr >= 32 && *ptr < 128) {
            stbtt_aligned_quad q;
            // GetBakedQuad calcula las coordenadas de recorte (src) y pantalla (dst)
            stbtt_GetBakedQuad(font->cdata, 512, 512, *ptr - 32, &x, &y, &q, 1);
            
            GE_Rect src = { q.s0 * 512, q.t0 * 512, (q.s1 - q.s0) * 512, (q.t1 - q.t0) * 512 };
            GE_Rect dst = { q.x0, q.y0, q.x1 - q.x0, q.y1 - q.y0 };

            GE_DrawSpriteEx(ctx, font->texture, src, dst, color);
        }
        ptr++;
    }
}

// 5. Función de Alineación (USA LA MEDICIÓN)
//                                                                    AQUI ESTABA EL ERROR
//                                                                            v
void GE_DrawTextAligned(GE_Context* c, GE_Font* f, const char* t, float x, float y, GE_TextAlign align, GE_Color col) {
    if (!c || !f || !t) return;

    // Alineación 0 = Izquierda (GE_ALIGN_LEFT)
    if (align == 0) {
        GE_DrawText(c, f, t, x, y, col);
        return;
    }

    GE_Point size = GE_MeasureText(f, t);
    float finalX = x;

    if (align == 1) { // GE_ALIGN_CENTER
        finalX = x - (size.x / 2.0f);
    } 
    else if (align == 2) { // GE_ALIGN_RIGHT
        finalX = x - size.x;
    }
    
    GE_DrawText(c, f, t, finalX, y, col);
}
