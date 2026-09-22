#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "include/max_heap.h"
#include "include/post.h"

// ============================================================================
// CONFIGURATION
// ============================================================================
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define MAX_INPUT_LENGTH 140
#define HEAP_CAPACITY 50
#define FPS 60

// Colors
#define COLOR_BG (SDL_Color){15, 23, 42, 255}           // Dark slate
#define COLOR_PANEL (SDL_Color){30, 41, 59, 255}        // Slate 800
#define COLOR_TEXT (SDL_Color){241, 245, 249, 255}      // Slate 100
#define COLOR_INPUT_BG (SDL_Color){51, 65, 85, 255}     // Slate 700
#define COLOR_BUTTON (SDL_Color){59, 130, 246, 255}     // Blue 500
#define COLOR_BUTTON_HOVER (SDL_Color){37, 99, 235, 255} // Blue 600
#define COLOR_HIGH_RISK (SDL_Color){239, 68, 68, 255}   // Red 500
#define COLOR_MED_RISK (SDL_Color){251, 191, 36, 255}   // Amber 400
#define COLOR_LOW_RISK (SDL_Color){34, 197, 94, 255}    // Green 500
#define COLOR_DELETED (SDL_Color){220, 38, 38, 255}     // Red 600

// ============================================================================
// MODERATION RULES
// ============================================================================
typedef struct {
    const char *word;
    float risk_increase;
} RiskKeyword;

static const RiskKeyword RISK_KEYWORDS[] = {
    {"shit", 0.3f}, {"fuck", 0.4f}, {"damn", 0.2f}, {"hate", 0.35f},
    {"kill", 0.5f}, {"death", 0.4f}, {"violence", 0.45f},
    {"attack", 0.4f}, {"bomb", 0.6f}, {"weapon", 0.5f},
    {"drugs", 0.4f}, {"scam", 0.5f}, {"fraud", 0.5f},
    {"idiot", 0.25f}, {"stupid", 0.2f}, {"dumb", 0.2f}
};
#define NUM_KEYWORDS (sizeof(RISK_KEYWORDS) / sizeof(RISK_KEYWORDS[0]))

// ============================================================================
// STRUCTURES
// ============================================================================
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font_large;
    TTF_Font *font_medium;
    TTF_Font *font_small;
    int running;
    max_heap *heap;
    int next_post_id;
    
    // Input state
    char input_text[MAX_INPUT_LENGTH + 1];
    int input_cursor;
    int input_active;
    
    // Button state
    SDL_Rect post_button;
    SDL_Rect review_button;
    SDL_Rect clear_button;
    int post_button_hover;
    int review_button_hover;
    int clear_button_hover;
    
    // Animation
    char last_message[256];
    int message_timer;
    SDL_Color message_color;
    
    // Stats
    int total_posted;
    int total_reviewed;
    int total_deleted;
    int total_evicted;
} AppState;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

float calculate_risk(const char *text) {
    float risk = 0.05f; // Base risk
    
    // Convert to lowercase for comparison
    char lower_text[MAX_CONTENT_LENGTH];
    strncpy(lower_text, text, MAX_CONTENT_LENGTH);
    for (int i = 0; lower_text[i]; i++) {
        lower_text[i] = tolower(lower_text[i]);
    }
    
    // Check for risk keywords
    for (size_t i = 0; i < NUM_KEYWORDS; i++) {
        if (strstr(lower_text, RISK_KEYWORDS[i].word)) {
            risk += RISK_KEYWORDS[i].risk_increase;
        }
    }
    
    // Cap and spike risks
    int caps = 0, total = 0;
    for (const char *p = text; *p; p++) {
        if (isupper(*p)) caps++;
        if (isalpha(*p)) total++;
    }
    if (total > 0 && (float)caps / total > 0.7f) {
        risk += 0.15f; // YELLING increases risk
    }
    
    // Multiple exclamation marks
    int exclamations = 0;
    for (const char *p = text; *p; p++) {
        if (*p == '!') exclamations++;
    }
    if (exclamations > 2) risk += 0.1f;
    
    // Clamp risk between 0 and 1
    if (risk > 1.0f) risk = 1.0f;
    if (risk < 0.0f) risk = 0.0f;
    
    return risk;
}

SDL_Color get_risk_color(float risk) {
    if (risk >= 0.7f) return COLOR_HIGH_RISK;
    if (risk >= 0.4f) return COLOR_MED_RISK;
    return COLOR_LOW_RISK;
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, 
               int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void draw_text_wrapped(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                       int x, int y, int max_width, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, text, color, max_width);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void draw_button(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                 SDL_Rect *rect, int is_hover, SDL_Color bg_color, SDL_Color hover_color) {
    SDL_Color color = is_hover ? hover_color : bg_color;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, rect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    SDL_RenderDrawRect(renderer, rect);
    
    // Text
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, COLOR_TEXT);
    if (surface) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            int text_w = surface->w;
            int text_h = surface->h;
            SDL_Rect text_rect = {
                rect->x + (rect->w - text_w) / 2,
                rect->y + (rect->h - text_h) / 2,
                text_w, text_h
            };
            SDL_RenderCopy(renderer, texture, NULL, &text_rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

int point_in_rect(int x, int y, SDL_Rect *rect) {
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

// ============================================================================
// APP FUNCTIONS
// ============================================================================

void show_message(AppState *app, const char *msg, SDL_Color color) {
    strncpy(app->last_message, msg, sizeof(app->last_message) - 1);
    app->last_message[sizeof(app->last_message) - 1] = '\0';
    app->message_color = color;
    app->message_timer = FPS * 3; // Show for 3 seconds
}

void post_message(AppState *app) {
    if (strlen(app->input_text) == 0) {
        show_message(app, "Cannot post empty message!", COLOR_HIGH_RISK);
        return;
    }
    
    float risk = calculate_risk(app->input_text);
    
    // Posts with risk = 1.0 are automatically deleted
    if (risk >= 1.0f) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Post DELETED (100%% risk): \"%s\"", app->input_text);
        show_message(app, msg, COLOR_DELETED);
        app->total_deleted++;
        app->input_text[0] = '\0';
        app->input_cursor = 0;
        return;
    }
    
    post *p = malloc(sizeof(post));
    p->id = app->next_post_id++;
    p->risk = risk;
    strncpy(p->content, app->input_text, MAX_CONTENT_LENGTH - 1);
    p->content[MAX_CONTENT_LENGTH - 1] = '\0';
    
    bool inserted = max_heap_insert(app->heap, p);
    
    if (inserted) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Posted! Risk: %.0f%% | Queue: %d/%d", 
                 risk * 100, max_heap_size(app->heap), HEAP_CAPACITY);
        show_message(app, msg, get_risk_color(risk));
        app->total_posted++;
    } else {
        show_message(app, "Post rejected (queue full, risk too low)", COLOR_MED_RISK);
        app->total_evicted++;
        free(p);
    }
    
    app->input_text[0] = '\0';
    app->input_cursor = 0;
}

void review_next_post(AppState *app) {
    if (max_heap_is_empty(app->heap)) {
        show_message(app, "No posts to review!", COLOR_MED_RISK);
        return;
    }
    
    post reviewed;
    if (max_heap_pop_max(app->heap, &reviewed)) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Reviewed #%d (%.0f%% risk): \"%s\"",
                 reviewed.id, reviewed.risk * 100, reviewed.content);
        show_message(app, msg, get_risk_color(reviewed.risk));
        app->total_reviewed++;
    }
}

void clear_queue(AppState *app) {
    while (!max_heap_is_empty(app->heap)) {
        post temp;
        max_heap_pop_max(app->heap, &temp);
    }
    show_message(app, "Queue cleared!", COLOR_MED_RISK);
}

// ============================================================================
// RENDERING
// ============================================================================

void render_input_panel(AppState *app) {
    SDL_Renderer *r = app->renderer;
    
    // Panel background
    SDL_Rect panel = {20, 20, WINDOW_WIDTH - 40, 200};
    SDL_SetRenderDrawColor(r, COLOR_PANEL.r, COLOR_PANEL.g, COLOR_PANEL.b, 255);
    SDL_RenderFillRect(r, &panel);
    
    // Title
    draw_text(r, app->font_large, "Content Moderation System", 40, 35, COLOR_TEXT);
    
    // Input label
    char label[100];
    snprintf(label, sizeof(label), "New Post (%d/%d chars):", 
             (int)strlen(app->input_text), MAX_INPUT_LENGTH);
    draw_text(r, app->font_medium, label, 40, 75, COLOR_TEXT);
    
    // Input box
    SDL_Rect input_box = {40, 105, WINDOW_WIDTH - 200, 45};
    SDL_SetRenderDrawColor(r, COLOR_INPUT_BG.r, COLOR_INPUT_BG.g, COLOR_INPUT_BG.b, 255);
    SDL_RenderFillRect(r, &input_box);
    
    // Input border (active/inactive)
    if (app->input_active) {
        SDL_SetRenderDrawColor(r, COLOR_BUTTON.r, COLOR_BUTTON.g, COLOR_BUTTON.b, 255);
    } else {
        SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
    }
    SDL_RenderDrawRect(r, &input_box);
    
    // Input text
    if (strlen(app->input_text) > 0) {
        draw_text(r, app->font_medium, app->input_text, 50, 115, COLOR_TEXT);
    } else {
        draw_text(r, app->font_medium, "Type your post here...", 50, 115, 
                 (SDL_Color){150, 150, 150, 255});
    }
    
    // Cursor
    if (app->input_active && (SDL_GetTicks() / 500) % 2 == 0) {
        int cursor_x = 50;
        if (strlen(app->input_text) > 0) {
            int w, h;
            TTF_SizeText(app->font_medium, app->input_text, &w, &h);
            cursor_x += w;
        }
        SDL_SetRenderDrawColor(r, COLOR_TEXT.r, COLOR_TEXT.g, COLOR_TEXT.b, 255);
        SDL_RenderDrawLine(r, cursor_x, 115, cursor_x, 140);
    }
    
    // Post button
    app->post_button = (SDL_Rect){WINDOW_WIDTH - 140, 105, 100, 45};
    draw_button(r, app->font_medium, "POST", &app->post_button, 
                app->post_button_hover, COLOR_BUTTON, COLOR_BUTTON_HOVER);
    
    // Stats
    char stats[200];
    snprintf(stats, sizeof(stats), "Posted: %d | Reviewed: %d | Deleted: %d | Evicted: %d",
             app->total_posted, app->total_reviewed, app->total_deleted, app->total_evicted);
    draw_text(r, app->font_small, stats, 40, 170, (SDL_Color){180, 180, 180, 255});
}

void render_heap_visualization(AppState *app) {
    SDL_Renderer *r = app->renderer;
    
    // Panel
    SDL_Rect panel = {20, 240, WINDOW_WIDTH - 40, WINDOW_HEIGHT - 260};
    SDL_SetRenderDrawColor(r, COLOR_PANEL.r, COLOR_PANEL.g, COLOR_PANEL.b, 255);
    SDL_RenderFillRect(r, &panel);
    
    // Title
    char title[100];
    snprintf(title, sizeof(title), "Review Queue (%d/%d posts)", 
             max_heap_size(app->heap), HEAP_CAPACITY);
    draw_text(r, app->font_large, title, 40, 255, COLOR_TEXT);
    
    // Control buttons
    app->review_button = (SDL_Rect){WINDOW_WIDTH - 250, 250, 100, 35};
    app->clear_button = (SDL_Rect){WINDOW_WIDTH - 135, 250, 95, 35};
    
    draw_button(r, app->font_small, "Review", &app->review_button,
                app->review_button_hover, COLOR_BUTTON, COLOR_BUTTON_HOVER);
    draw_button(r, app->font_small, "Clear", &app->clear_button,
                app->clear_button_hover, (SDL_Color){220, 38, 38, 255}, 
                (SDL_Color){185, 28, 28, 255});
    
    // Message display
    if (app->message_timer > 0) {
        draw_text_wrapped(r, app->font_medium, app->last_message, 40, 300, 
                         WINDOW_WIDTH - 100, app->message_color);
    }
    
    // Display posts in queue
    if (max_heap_is_empty(app->heap)) {
        draw_text(r, app->font_medium, "Queue is empty. Post something to get started!",
                 40, 350, (SDL_Color){150, 150, 150, 255});
        return;
    }
    
    int y_offset = app->message_timer > 0 ? 360 : 320;
    int posts_shown = 0;
    int max_posts = 8; // Show top 8 posts
    
    // Show posts from heap (we'll peek at the structure)
    for (int i = 0; i < app->heap->size && posts_shown < max_posts; i++) {
        heap_node *node = app->heap->nodes[i];
        
        for (int j = 0; j < node->post_count && posts_shown < max_posts; j++) {
            post *p = node->posts[j];
            
            // Post card
            SDL_Rect card = {40, y_offset, WINDOW_WIDTH - 80, 50};
            
            // Background color based on risk
            SDL_Color risk_color = get_risk_color(p->risk);
            SDL_SetRenderDrawColor(r, risk_color.r, risk_color.g, risk_color.b, 40);
            SDL_RenderFillRect(r, &card);
            
            // Border
            SDL_SetRenderDrawColor(r, risk_color.r, risk_color.g, risk_color.b, 255);
            SDL_RenderDrawRect(r, &card);
            
            // Post ID and risk
            char header[50];
            snprintf(header, sizeof(header), "#%d - Risk: %.0f%%", p->id, p->risk * 100);
            draw_text(r, app->font_small, header, 50, y_offset + 5, risk_color);
            
            // Content
            char content[150];
            if (strlen(p->content) > 70) {
                strncpy(content, p->content, 67);
                content[67] = '\0';
                strcat(content, "...");
            } else {
                strcpy(content, p->content);
            }
            draw_text(r, app->font_small, content, 50, y_offset + 25, COLOR_TEXT);
            
            y_offset += 60;
            posts_shown++;
        }
    }
    
    if (max_heap_size(app->heap) > max_posts) {
        char more[50];
        snprintf(more, sizeof(more), "... and %d more posts", 
                 max_heap_size(app->heap) - posts_shown);
        draw_text(r, app->font_small, more, 40, y_offset + 10, 
                 (SDL_Color){150, 150, 150, 255});
    }
}

void render(AppState *app) {
    // Clear screen
    SDL_SetRenderDrawColor(app->renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
    SDL_RenderClear(app->renderer);
    
    // Render UI components
    render_input_panel(app);
    render_heap_visualization(app);
    
    // Present
    SDL_RenderPresent(app->renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

void handle_mouse_motion(AppState *app, int x, int y) {
    app->post_button_hover = point_in_rect(x, y, &app->post_button);
    app->review_button_hover = point_in_rect(x, y, &app->review_button);
    app->clear_button_hover = point_in_rect(x, y, &app->clear_button);
}

void handle_mouse_click(AppState *app, int x, int y) {
    if (point_in_rect(x, y, &app->post_button)) {
        post_message(app);
    } else if (point_in_rect(x, y, &app->review_button)) {
        review_next_post(app);
    } else if (point_in_rect(x, y, &app->clear_button)) {
        clear_queue(app);
    } else {
        // Click anywhere else to focus input
        SDL_Rect input_area = {40, 105, WINDOW_WIDTH - 200, 45};
        app->input_active = point_in_rect(x, y, &input_area);
    }
}

void handle_text_input(AppState *app, const char *text) {
    if (!app->input_active) return;
    
    int current_len = strlen(app->input_text);
    int text_len = strlen(text);
    
    if (current_len + text_len < MAX_INPUT_LENGTH) {
        strcat(app->input_text, text);
    }
}

void handle_keydown(AppState *app, SDL_Keycode key) {
    if (!app->input_active) return;
    
    if (key == SDLK_BACKSPACE && strlen(app->input_text) > 0) {
        app->input_text[strlen(app->input_text) - 1] = '\0';
    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        post_message(app);
    } else if (key == SDLK_ESCAPE) {
        app->input_active = 0;
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    
    srand(time(NULL));
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "Content Moderation System - Max Heap Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Load fonts (try multiple common paths)
    const char *font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:\\Windows\\Fonts\\arial.ttf",
        NULL
    };
    
    TTF_Font *font_large = NULL;
    TTF_Font *font_medium = NULL;
    TTF_Font *font_small = NULL;
    
    for (int i = 0; font_paths[i] != NULL; i++) {
        font_large = TTF_OpenFont(font_paths[i], 24);
        if (font_large) {
            font_medium = TTF_OpenFont(font_paths[i], 18);
            font_small = TTF_OpenFont(font_paths[i], 14);
            break;
        }
    }
    
    if (!font_large || !font_medium || !font_small) {
        fprintf(stderr, "Failed to load fonts. Please install DejaVu fonts.\n");
        fprintf(stderr, "Ubuntu/Debian: sudo apt-get install fonts-dejavu\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Initialize app state
    AppState app = {0};
    app.window = window;
    app.renderer = renderer;
    app.font_large = font_large;
    app.font_medium = font_medium;
    app.font_small = font_small;
    app.running = 1;
    app.heap = max_heap_new(HEAP_CAPACITY);
    app.next_post_id = 1;
    app.input_active = 1;
    
    if (!app.heap) {
        fprintf(stderr, "Failed to create heap\n");
        TTF_CloseFont(font_large);
        TTF_CloseFont(font_medium);
        TTF_CloseFont(font_small);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    show_message(&app, "Welcome! Type a post and click POST to add to the moderation queue.", 
                 COLOR_BUTTON);
    
    // Main loop
    SDL_Event event;
    Uint32 frame_start, frame_time;
    const int frame_delay = 1000 / FPS;
    
    while (app.running) {
        frame_start = SDL_GetTicks();
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    app.running = 0;
                    break;
                    
                case SDL_MOUSEMOTION:
                    handle_mouse_motion(&app, event.motion.x, event.motion.y);
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        handle_mouse_click(&app, event.button.x, event.button.y);
                    }
                    break;
                    
                case SDL_TEXTINPUT:
                    handle_text_input(&app, event.text.text);
                    break;
                    
                case SDL_KEYDOWN:
                    handle_keydown(&app, event.key.keysym.sym);
                    break;
            }
        }
        
        // Update
        if (app.message_timer > 0) {
            app.message_timer--;
        }
        
        // Render
        render(&app);
        
        // Frame rate control
        frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > (int)frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
    }
    
    // Cleanup
    max_heap_free(app.heap);
    TTF_CloseFont(font_large);
    TTF_CloseFont(font_medium);
    TTF_CloseFont(font_small);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}