//John Henry Buenaventura
//Hw 1
// Purpose: This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <ctime>

#define RAND ((float)rand() / RAND_MAX)
#define PI 3.14159

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 50000
#define GRAVITY 0.1
#define BOUNCE 0.1
#define MAX_BOXES 5
#define MAX_CIRC 2


//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures
struct Vec {
    float x, y, z;
};

//////////////////////////////////////
struct Shape {
    float width, height;
    float radius;
    Vec center;
};

/////////////////////////////////////
class Particle {
public:
    Shape s;
    Vec velocity;
    
    float speed(){
        return sqrt(pow(velocity.x,2));
    }
    
    Vec color(){
        float spd = speed();
        spd = spd / 0.0;
        if (spd > 1.0)
            spd = 1.0;
        Vec clr;
        clr.x = 1.0 - spd;
        clr.y = 2.0;
        clr.z = spd;
        return clr;
    }
};

////////////////////////////////////
class Spinner{
public:
    Shape s;
    float velocity;    
    float rotation;
    
    void spin() 
    {
        rotation = fmod(rotation + velocity, 2.0*PI);
    }
    
    void draw() 
    {
        float r = s.radius;
        float x = s.center.x;
        float y = s.center.y;
        float t = rotation;
        float w = r / 10.0;
        glColor3ub(255,255,255);
        glBegin(GL_POLYGON);
        glVertex2f(x + (r * cos(t)) - (w * sin(t)), y + (r * sin(t)) + (w * cos(t)));
        glVertex2f(x + (r * cos(t)) + (w * sin(t)), y + (r * sin(t)) - (w * cos(t)));
        glVertex2f(x - (r * cos(t)) + (w * sin(t)), y - (r * sin(t)) - (w * cos(t)));
        glVertex2f(x - (r * cos(t)) - (w * sin(t)), y - (r * sin(t)) + (w * cos(t)));
        
        glEnd();   
    }
    
    void collide(Particle *p) 
    {
        float x = p->s.center.x;
        float y = p->s.center.y;
        float x1 = s.center.x + s.radius * cos(rotation);
        float y1 = s.center.y + s.radius * sin(rotation);
        float x2 = s.center.x - s.radius * cos(rotation);
        float y2 = s.center.y - s.radius * sin(rotation);

        float cx = s.center.x;
        float cy = s.center.y;
        float r = s.radius;
        float dist = pow(cx - x, 2) + pow(cy - y, 2);
        if (dist > r * r)
            return;
        
        float dist2 = fabs((y2-y1)*x-(x2-x1)*y+x2*y1-y2*x1)/sqrt(pow(y2-y1,2)+pow(x2-x1,2));
        if (dist2 < s.radius / 10.0) {
            p->s.center.x -= p->velocity.x;
            p->s.center.y -= p->velocity.y;
            Vec normal;
            float len = sqrt(pow(x1-x2,2)+pow(y1-y2,2));
            normal.x = (y1 - y2) / len;
            normal.y = (x2 - x1) / len;
            float dot = normal.x * p->velocity.x + normal.y * p->velocity.y;
            p->velocity.x = (-2.0 * dot * normal.x + p->velocity.x);
            p->velocity.y = (-2.0 * dot * normal.y + p->velocity.y);
            float dx = p->velocity.x;
            float dy = p->velocity.y;
            float partSpeed = p->speed();
            bool dir = ((cy-y)*(dx-x) > (cx-x)*(dy-y));
            if (dir) {
                velocity += 0.000000001 * dist * partSpeed;
            }else{
                velocity -= 0.000000001 * dist * partSpeed;
            }
            if (velocity > 0.2)
                velocity = 0.2;
            if (velocity < -0.2)
                velocity = -0.2;
        }
    }
};

////////////////////////////////////////////
struct Game {
    Shape box[MAX_BOXES];
    Shape circ[MAX_CIRC];
    Particle particle[MAX_PARTICLES];
    Spinner spin;
    int n;
    int lastX;
    int lastY;
    int mode;
    int boxNum[2];
    float spread[2];
    int genNum[2];
};

///////////////////////////////////////////
//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);

///////////////////////////////////////////
int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;
    game.mode = 0;
    game.boxNum[0] = MAX_BOXES;
    game.spread[0] = 2.0;
    game.genNum[0] = 50;
    game.boxNum[1] = 0;
    game.spread[1] = 0.5;
    game.genNum[1] = 5;
    
    
    //declare a box shape
    for (int i = 0; i < MAX_BOXES; i++) {
        game.box[i].width = 100;
        game.box[i].height = 10;
        game.box[i].center.x = 120 + i*65;
        game.box[i].center.y = 500 - i*60;
    }
    game.circ[0].radius =  150;
    game.circ[0].center.x = WINDOW_WIDTH - 200;
    game.circ[0].center.y = 100;
    game.circ[1].radius =  10;
    game.circ[1].center.x = 0;
    game.circ[1].center.y = WINDOW_HEIGHT;
    game.spin.velocity = -0.01;
    game.spin.s.center.x = WINDOW_WIDTH / 2.0 + 100.0;
    game.spin.s.center.y = WINDOW_HEIGHT / 2.0 - 100.0;
    game.spin.s.radius = 100.0;
    
    //start animation
    while(!done) {
        while(XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);
        }
        movement(&game);
        render(&game);
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "CS335 Hw01");
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    ButtonPress | ButtonReleaseMask |
    PointerMotionMask |
    StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
                        InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

void makeParticle(Game *game, int x, int y)
{
    Particle *p = &game->particle[game->n];
    if (game->n >= MAX_PARTICLES) {
        return;
    }
    
    p->s.center.x = x;
    p->s.center.y = y;
    float theta = RAND * PI * 2.0;
    float spread = game->spread[game->mode]; 
    p->velocity.y = ((float)rand() / RAND_MAX - 0.5) * game->spread[game->mode];
    p->velocity.x = ((float)rand() / RAND_MAX - 0.5) * game->spread[game->mode];
    p->velocity.x = spread * cos(theta);
    p->velocity.y = spread * sin(theta);
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;
    
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            int y = WINDOW_HEIGHT - e->xbutton.y;
            makeParticle(game, e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            return;
        }
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        game->lastX = e->xbutton.x;
        game->lastY = e->xbutton.y;
        if (game->mode==0){
            game->circ[1].center.x = e->xbutton.x;
            game->circ[1].center.y = WINDOW_HEIGHT - e->xbutton.y;
        } else {
            game->spin.s.center.x =  e->xbutton.x;
            game->spin.s.center.y = WINDOW_HEIGHT - e->xbutton.y;
        }
        if (++n < 10)
            return;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }else if (key == XK_s) {
            game->mode ^= 1;
        }
        //You may check other keys here.
        
    }
    return 0;
}
void circ(Game *game)
{
    static float count = 0.0;
    count += 0.05;
    game->lastX = (WINDOW_WIDTH / 2) + cos(count) * (WINDOW_WIDTH * 0.4);
    game->lastY = (WINDOW_HEIGHT / 2) + sin(count * 2.0) * (WINDOW_HEIGHT * 0.4);
}

void DrawCirc(float cx, float cy, float r, int segs)
{
    glBegin(GL_POLYGON); 
    for(int i = 0; i < segs; i++) 
    { 
        float theta = 2.0 * PI * float(i) / float(segs);//get the current angle 
        
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        
        glVertex2f(x + cx, y + cy);
        
    } 
    glEnd(); 
    
}

void movement(Game *game)
{
    if (game->mode==1)
        game->spin.spin();
        
    circ(game);
    for (int i = 0; i < game->genNum[game->mode]; i++)
        makeParticle(game, 150, WINDOW_HEIGHT - 50);
    
    Particle *p;
    
    if (game->n <= 0)
        return;
    for (int i = 0; i < game->n;i++) {
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= .2 + (RAND - 0.5) * 0.04;
        
        //check for collision with shapes...
        if (game->mode==1)
            game->spin.collide(p);
        Shape *s;
        for (int j = 0; j < game->boxNum[game->mode]; j++) {
            s = &game->box[j];
            if (p->s.center.y >= s->center.y - (s->height) &&
                p->s.center.y <= s->center.y + (s->height) &&
                p->s.center.x >= s->center.x - (s->width) &&
                p->s.center.x <= s->center.x + (s->width)) {
                p->s.center.y = p->s.center.y + ((s->center.y + s->height) - p->s.center.y);
                p->velocity.y *= -BOUNCE;
                }
        }
        for (int j = 0; j < MAX_CIRC; j++) {
            if (j == 1 && game->mode==1)
                continue; 
            s = &game->circ[j];
            float dist = sqrt(pow(p->s.center.x - s->center.x,2)+pow(p->s.center.y - s->center.y,2));
            if (dist < s->radius) {
		
                p->s.center.x -= p->velocity.x;
                p->s.center.y -= p->velocity.y;
                Vec normal;
                normal.x = (p->s.center.x - s->center.x) / dist;
                normal.y = (p->s.center.y - s->center.y) / dist;

        		p->velocity.x += normal.x * 1.0;
	        	p->velocity.y += normal.y * 1.0;

            }
        }
        if (p->s.center.y < 0.0) {
            game->particle[i] = game->particle[game->n-1];
            game->n--;
        }
    }
}


void render(Game *game)
{
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...
    if (game->mode==1)
        game->spin.draw();
    
    //draw box
    Shape *s;
    glColor3ub(50,132,128);
    
    for (int i = 0; i < MAX_CIRC; i++) {
        if (i == 1)
            continue; 
        s = &game->circ[i];
        DrawCirc(s->center.x, s->center.y, s->radius, 50);
    }
    
    glColor3ub(100,140,100);
    for (int i = 0; i < game->boxNum[game->mode]; i++) {
        s = &game->box[i];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd();
        glPopMatrix();
    }
    
    //draw all particles here
    glPushMatrix();
    for(int i = 0;i<game->n;i++){
        Vec *c = &game->particle[i].s.center;
        Vec col = game->particle[i].color();
        glColor3ub(col.x * 255,col.y * 255,col.z * 255);
        w = 2;
        h = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }
}



