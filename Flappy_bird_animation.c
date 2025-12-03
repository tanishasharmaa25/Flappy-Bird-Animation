#include <cairo/cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 400
#define HEIGHT 600

typedef struct {
    double x, y;
    double speed;
} Bird;

typedef struct {
    double x;                   //Horizontal position of obstacle
    double gap_y;               //vertical start position
    double gap_height;          //gap between top and bottom pipe
} Obstacle;

void draw_frame(int frame, Bird bird, Obstacle obs, int game_over) {
    char filename[50];
    sprintf(filename, "frame_%03d.png", frame);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
    cairo_t *cr = cairo_create(surface);

    //background
    cairo_set_source_rgb(cr, 0.4, 0.7, 1.0);
    cairo_paint(cr);

    //draw obstacle
    cairo_set_source_rgb(cr, 0.2, 0.8, 0.2);

    //top pipe
    cairo_rectangle(cr, obs.x, 0, 80, obs.gap_y);
    cairo_fill(cr);

    //bottom pipe
    cairo_rectangle(cr, obs.x, obs.gap_y + obs.gap_height, 80, HEIGHT);
    cairo_fill(cr);

    //draw bird
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_arc(cr, bird.x, bird.y, 20, 0, 2 * 3.14159);
    cairo_fill(cr);

    //GAME OVER text
    if (game_over) {
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_select_font_face(cr, "Arial",
                               CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 50);

        cairo_text_extents_t ext;
        cairo_text_extents(cr, "GAME OVER", &ext);

        double x = (WIDTH - ext.width) / 2 - ext.x_bearing;
        double y = (HEIGHT - ext.height) / 2 - ext.y_bearing;

        cairo_move_to(cr, x, y);
        cairo_show_text(cr, "GAME OVER");
    }

    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, filename);
    cairo_surface_destroy(surface);
}

int check_collision(Bird bird, Obstacle obs) {
    //screen boundaries
    if (bird.y < 0 || bird.y > HEIGHT)
        return 1;

    //pipe collision
    if (bird.x + 20 > obs.x && bird.x - 20 < obs.x + 80) {
        if (bird.y - 20 < obs.gap_y || bird.y + 20 > obs.gap_y + obs.gap_height) {
            return 1;
        }
    }
    return 0;
}

int main() {
    Bird bird = {200, HEIGHT/2, 2.0};
    Obstacle obs = {WIDTH, 200, 150};

    char moves[200];
    printf("Enter moves (U/D sequence, no spaces like UUDDUDU): ");
    scanf("%s", moves);

    int frame = 1;
    int game_over = 0;

    for (int i = 0; i < strlen(moves); i++) {
        char m = moves[i];

        //speed adjustment rules
        if (i > 0) {
            if (moves[i] == 'U' && moves[i-1] == 'U')
                bird.speed *= 1.2;
            if (moves[i] == 'D' && moves[i-1] == 'D')
                bird.speed *= 0.8;
        }

        //bird movement
        if (m == 'U') bird.y -= bird.speed;
        if (m == 'D') bird.y += bird.speed;

        //move obstacle left
        obs.x -= 20;

        //respawn obstacle
        if (obs.x < -100) {
            obs.x = WIDTH;
            obs.gap_y = (rand() % 300) + 100;
        }

        // collision check
        if (check_collision(bird, obs)) {
            game_over = 1;
            draw_frame(frame++, bird, obs, 1);  //Draw final GAME OVER frame
            break;  //STOP animation immediately
        }

        //normal frame
        draw_frame(frame++, bird, obs, 0);
    }

    

    printf("Animation frames generated successfully.\n");
    system("cmd /c \"ffmpeg -y -framerate 5 -i frame_%03d.png -pix_fmt yuv420p flappy_Bird_animation.mp4\"");
    return 0;
}
