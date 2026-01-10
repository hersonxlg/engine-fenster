/*
    ============================================================================
    GE (GAME ENGINE) - HEADER DEFINITIVO
    Versión: Fusionada (Base Estable + Features Completas)
    ============================================================================
*/

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// 1. TIPOS Y ESTRUCTURAS DE DATOS
// ============================================================================

// Color en formato 0xAARRGGBB
typedef uint32_t GE_Color;

// Puntos y Rectángulos (Floats para sub-píxeles y suavizado)
typedef struct { float x, y; } GE_Point;
typedef struct { float x, y, w, h; } GE_Rect;

// Estructuras Opacas (El usuario usa punteros, la data está oculta)
typedef struct GE_Context GE_Context;
typedef struct GE_Sprite GE_Sprite;
typedef struct GE_Font GE_Font;
typedef struct GE_Sound GE_Sound;

// Cámara 2D
typedef struct {
    GE_Point offset; // Desplazamiento del centro (normalmente width/2, height/2)
    GE_Point target; // Punto del mundo que queremos enfocar
    float rotation;  // En grados
    float zoom;      // 1.0f es normal, 2.0f es zoom x2
} GE_Camera;

// Sistema de Animación
typedef struct {
    GE_Sprite* sprite;
    int frame_w, frame_h;   // Tamaño de cada cuadro
    int start_frame;        // Cuadro inicial
    int end_frame;          // Cuadro final
    float frame_duration;   // Tiempo por cuadro (ej: 0.1f)
    float timer;            // Contador interno
    int current_frame;      // Cuadro actual
    bool active;            // Si false, no avanza
    bool loop;              // Si true, vuelve al inicio
} GE_Animation;

// Alineación de texto
typedef enum {
    GE_ALIGN_LEFT = 0, 
    GE_ALIGN_CENTER, 
    GE_ALIGN_RIGHT
} GE_TextAlign;

// Teclas (Mapeo estandarizado)
// ============================================================================
// REEMPLAZAR EN ENGINE.H (CORRECCIÓN ASCII)
// ============================================================================

typedef enum {
    GE_KEY_NULL = 0,
    
    // MOUSE (Usamos números arbitrarios altos para no chocar con ASCII)
    GE_MOUSE_LEFT = 1, 
    GE_MOUSE_RIGHT = 2, 
    GE_MOUSE_MIDDLE = 3,
    GE_MOUSE_SCROLL_UP = 4, 
    GE_MOUSE_SCROLL_DOWN = 5,

    // TECLAS DE CONTROL (Códigos específicos de Fenster)
    GE_KEY_BACKSPACE = 8,
    GE_KEY_TAB = 9,
    GE_KEY_ENTER = 13,
    GE_KEY_ESCAPE = 27,
    GE_KEY_SPACE = 32,

    // NÚMEROS (ASCII)
    GE_KEY_0 = '0', GE_KEY_1 = '1', GE_KEY_2 = '2', GE_KEY_3 = '3', GE_KEY_4 = '4',
    GE_KEY_5 = '5', GE_KEY_6 = '6', GE_KEY_7 = '7', GE_KEY_8 = '8', GE_KEY_9 = '9',

    // LETRAS (ASCII)
    GE_KEY_A = 'A', GE_KEY_B = 'B', GE_KEY_C = 'C', GE_KEY_D = 'D', GE_KEY_E = 'E',
    GE_KEY_F = 'F', GE_KEY_G = 'G', GE_KEY_H = 'H', GE_KEY_I = 'I', GE_KEY_J = 'J',
    GE_KEY_K = 'K', GE_KEY_L = 'L', GE_KEY_M = 'M', GE_KEY_N = 'N', GE_KEY_O = 'O',
    GE_KEY_P = 'P', GE_KEY_Q = 'Q', GE_KEY_R = 'R', GE_KEY_S = 'S', GE_KEY_T = 'T',
    GE_KEY_U = 'U', GE_KEY_V = 'V', GE_KEY_W = 'W', GE_KEY_X = 'X', GE_KEY_Y = 'Y',
    GE_KEY_Z = 'Z',

    // FLECHAS Y OTROS (Fenster suele mapearlos en la zona alta)
    GE_KEY_UP = 17, GE_KEY_DOWN = 18, GE_KEY_RIGHT = 19, GE_KEY_LEFT = 20,
    GE_KEY_DELETE = 127

} GE_InputKey;

// ============================================================================
// 2. SISTEMA PRINCIPAL (CORE)
// ============================================================================

// Inicializa la ventana y el audio. 'width'/'height' es la resolución interna del juego.
GE_Context* GE_Init(const char* title, int width, int height);

// Ajusta la resolución lógica y redimensiona la ventana automáticamente.
void GE_SetVirtualResolution(GE_Context* ctx, int game_width, int game_height);

// Procesa eventos de ventana e inputs. Retorna false si se debe cerrar.
bool GE_PollEvents(GE_Context* ctx);

// Limpia todo el contexto y cierra la ventana.
void GE_Close(GE_Context* ctx);

// Limpia la pantalla con un color base.
void GE_Clear(GE_Context* ctx, GE_Color color);

// Control de Tiempo
void GE_SetTargetFPS(GE_Context* ctx, int fps);
int GE_GetFPS(GE_Context* ctx);
float GE_GetDeltaTime(GE_Context* ctx);
double GE_GetTime(); // Tiempo desde el inicio en segundos

// ============================================================================
// 3. INPUT (ENTRADA DE DATOS)
// ============================================================================

bool GE_IsKeyDown(GE_Context* ctx, GE_InputKey key);      // Mantenida
bool GE_IsKeyPressed(GE_Context* ctx, GE_InputKey key);   // Pulsada en este frame
bool GE_IsKeyReleased(GE_Context* ctx, GE_InputKey key);  // Soltada en este frame
GE_Point GE_GetMousePosition(GE_Context* ctx);
int GE_GetMouseWheel(GE_Context* ctx);

// ============================================================================
// 4. PRIMITIVAS Y FORMAS BÁSICAS
// ============================================================================

void GE_DrawPixel(GE_Context* ctx, float x, float y, GE_Color color);

// Líneas
void GE_DrawLine(GE_Context* ctx, float x1, float y1, float x2, float y2, GE_Color color);
void GE_DrawLineThick(GE_Context* ctx, float x1, float y1, float x2, float y2, float thickness, GE_Color color);

// Rectángulos
void GE_DrawRect(GE_Context* ctx, float x, float y, float w, float h, GE_Color color);      // Borde
void GE_FillRect(GE_Context* ctx, float x, float y, float w, float h, GE_Color color);      // Relleno

// Círculos
void GE_DrawCircle(GE_Context* ctx, float cx, float cy, float radius, GE_Color color);      // Borde
void GE_FillCircle(GE_Context* ctx, float cx, float cy, float radius, GE_Color color);      // Relleno

// Elipses
void GE_DrawEllipse(GE_Context* ctx, float cx, float cy, float rx, float ry, GE_Color color);
void GE_FillEllipse(GE_Context* ctx, float cx, float cy, float rx, float ry, GE_Color color);

// Polígonos
void GE_DrawTriangle(GE_Context* ctx, float x1, float y1, float x2, float y2, float x3, float y3, GE_Color color);
void GE_FillTriangle(GE_Context* ctx, float x1, float y1, float x2, float y2, float x3, float y3, GE_Color color);
void GE_DrawPolygon(GE_Context* ctx, GE_Point* points, int count, GE_Color color);
void GE_FillPolygon(GE_Context* ctx, GE_Point* points, int count, GE_Color color);

// ============================================================================
// 5. CURVAS Y FORMAS AVANZADAS
// ============================================================================

// Arcos (start/end en grados)
void GE_DrawArc(GE_Context* ctx, float cx, float cy, float radius, float start_deg, float end_deg, GE_Color color);
void GE_FillSector(GE_Context* ctx, float cx, float cy, float radius, float start_deg, float end_deg, GE_Color color);

// Curvas de Bézier
void GE_DrawBezierQuad(GE_Context* ctx, GE_Point p0, GE_Point p1, GE_Point p2, int segments, GE_Color color);
void GE_DrawBezierCubic(GE_Context* ctx, GE_Point p0, GE_Point p1, GE_Point p2, GE_Point p3, int segments, GE_Color color);

// ============================================================================
// 6. SPRITES Y ANIMACIONES
// ============================================================================

GE_Sprite* GE_LoadSprite(const char* filepath);
void GE_UnloadSprite(GE_Sprite* sprite);

int GE_GetSpriteWidth(GE_Sprite* sprite);
int GE_GetSpriteHeight(GE_Sprite* sprite);

// Dibujado
void GE_DrawSprite(GE_Context* ctx, GE_Sprite* sprite, float x, float y, GE_Color tint);
void GE_DrawSpriteEx(GE_Context* ctx, GE_Sprite* sprite, GE_Rect src, GE_Rect dest, GE_Color tint);
void GE_DrawSpritePro(GE_Context* ctx, GE_Sprite* sprite, GE_Rect src, GE_Rect dest, GE_Point origin, float rotation, GE_Color tint);
void GE_DrawSpriteQuad(GE_Context* ctx, GE_Sprite* sprite, GE_Point p1, GE_Point p2, GE_Point p3, GE_Point p4, GE_Color tint);

// Animaciones
GE_Animation GE_CreateAnimation(GE_Sprite* sprite, int fw, int fh, float duration);
void GE_UpdateAnimation(GE_Animation* anim, float dt);
void GE_DrawAnimation(GE_Context* ctx, GE_Animation* anim, float x, float y, bool flip_x, GE_Color tint);

// ============================================================================
// 7. TEXTO (FONTS)
// ============================================================================

GE_Font* GE_LoadFont(const char* filepath, float size);
void GE_UnloadFont(GE_Font* font);

void GE_DrawText(GE_Context* ctx, GE_Font* font, const char* text, float x, float y, GE_Color color);
void GE_DrawTextAligned(GE_Context* ctx, GE_Font* font, const char* text, float x, float y, GE_TextAlign align, GE_Color color);
GE_Point GE_MeasureText(GE_Font* font, const char* text);

// ============================================================================
// 8. AUDIO
// ============================================================================

GE_Sound* GE_LoadSound(const char* filepath);
void GE_UnloadSound(GE_Sound* sound);

void GE_PlaySound(GE_Sound* sound);
void GE_StopSound(GE_Sound* sound);
void GE_SetVolume(GE_Sound* sound, float volume); // 0.0f a 1.0f
void GE_SetLooping(GE_Sound* sound, bool loop);

// ============================================================================
// 9. CÁMARA 2D
// ============================================================================

void GE_BeginMode2D(GE_Context* ctx, GE_Camera camera);
void GE_EndMode2D(GE_Context* ctx);
GE_Point GE_ScreenToWorld(GE_Camera camera, GE_Point screen_pos);
GE_Point GE_WorldToScreen(GE_Camera camera, GE_Point world_pos);

// ============================================================================
// 10. UTILIDADES MATEMÁTICAS
// ============================================================================

float GE_Math_Distance(float x1, float y1, float x2, float y2);
float GE_Math_Lerp(float start, float end, float t);
GE_Color GE_Color_FromRGB(uint8_t r, uint8_t g, uint8_t b);
GE_Color GE_Color_FromHex(uint32_t hex);

void GE_ShowCursor(bool visible);

// Getters para obtener dimensiones de forma segura
int GE_GetWidth(GE_Context* ctx);
int GE_GetHeight(GE_Context* ctx);

// En engine.h
int GE_GetSpriteWidth(GE_Sprite* sprite);
int GE_GetSpriteHeight(GE_Sprite* sprite);

#endif // GAME_ENGINE_H
       //
