//
//modified by: 
//* Angel Rojas
//* Christy Guerrero
//* Nygel Aton
//* Abddullah Aljahdali
//date: Jan 25, 18
//
//3350 Spring 2018 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git
//
//elements we will add to program...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

const int MAX_PARTICLES = 1000;
const float GRAVITY = 0.1;

//some structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

class Global {
public:
	int xres, yres;
	Shape box[MAX_PARTICLES];
	Particle particle[MAX_PARTICLES];
	int nparticle;
	int nbox;
	Shape circle;
	Global() {
		xres = 800;
		yres = 600;
		//define a box shape
		
		// Begin drawing CIRCLE here
		Shape *b = &circle;
		b->width = 100; // circle raduii
		b->center.x = xres - 110;
		b->center.y = 0;
		for (int i=0; i<5; i++) {
			b = &box[nbox];
			b->width = 100;
			b->height = 10;
			b->center.x = -250 + 5*65 +(i*100);
			b->center.y = yres - 5*60 -(i*50);
			++nbox;
		}
		nparticle = 0;
	}
} g;

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	X11_wrapper() {
		GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		int w = g.xres, h = g.yres;
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			//cout << "\n\tcannot connect to X server\n" << endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			//cout << "\n\tno appropriate visual found\n" << endl;
			exit(EXIT_FAILURE);
		} 
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		XSetWindowAttributes swa;
		swa.colormap = cmap;
		swa.event_mask =
			ExposureMask | KeyPressMask | KeyReleaseMask |
			ButtonPress | ButtonReleaseMask |
			PointerMotionMask |
			StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, w, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "3350 Lab1");
	}
	bool getXPending() {
		//See if there are pending events.
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		//Get a pending event.
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	srand(time(NULL));
	init_opengl();
	//Main animation loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			check_mouse(&e);
			done = check_keys(&e);
		}
		movement();
		render();
		x11.swapBuffers();
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	 glEnable(GL_TEXTURE_2D);
    initialize_fonts();
    glClear(GL_COLOR_BUFFER_BIT);


}

void makeParticle(int x, int y)
{
	if (g.nparticle >= MAX_PARTICLES)
		return;
	//cout << "makeParticle() " << x << " " << y << endl;
	//position of particle
	Particle *p = &g.particle[g.nparticle];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = (float)rand()/(float)RAND_MAX * 1.0 -0.5;
	p->velocity.x = (float)rand()/(float)RAND_MAX * 1.0 -0.5;
	++g.nparticle;
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	// if (e->type == ButtonRelease) {
	// 	return;
	// }
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;

			int y = g.yres - e->xbutton.y;
			for (int i=0; i<25; i++)
				makeParticle(e->xbutton.x, y);


		}
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_a:
				//Key A was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}
	const static int numberOfPoints = 100;

void movement()
{
	if (g.nparticle <= 0)
		return;
	Particle *p;

	for (int i=0; i<g.nparticle; i++) {
		p = &g.particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;

		Shape *s;
		//check with the cirlce
		s = &g.circle;
		float x=0, y=0;
		x = p->s.center.x - s->center.x;
		y = p->s.center.y - s->center.y;
		float dist = sqrt(x*x + y*y);
		if (dist < s->width) {
		// p->s.center.x -= dist;
		// p->s.center.y -= dist;
				p->velocity.y = -(p->velocity.y);
				//p->velocity.x = -(p->velocity.x);
				p->velocity.x -= 0.01;
				p->velocity.y *= 0.1;
		}


		//check for collision with shapes...
		
		for (int i=0; i<g.nbox; i++) {
			s= &g.box[i];
			float x = s->center.x;
			float y = s->center.y;
			float w = s->width;
			float h = s->height;
			// //cout << p->s.center.y << endl;
			if (p->s.center.y > y-h && p->s.center.y < y+h && p->s.center.x < x+w && p->s.center.x > x-w) {
				p->velocity.y = -(p->velocity.y);
				p->velocity.y *= 0.6;
			}
			if (p->s.center.y < 0.0) {
				// //cout << "off screen" << endl;
				g.particle[i] = g.particle[g.nparticle-1];
				--g.nparticle;
			}
		}
	}

}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	//
	//draw a box
	Shape *s = &g.circle;
	glColor3ub(90,140,90);
	glBegin(GL_POLYGON);
	// glVertex2i(s->center.x, s->center.y);
	for (int i=0; i<numberOfPoints; i++) {
		glVertex2i(
			s->center.x + (s->width*sin(i*(2*3.14549/numberOfPoints))),
			s->center.y + (s->width*cos(i*(2*3.14549/numberOfPoints)))
			);
	}
	glEnd();
	// glColor3ub(90,140,90);
	
	for (int i=0; i<g.nbox; i++) {
		s = &g.box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		float w, h;
		w = s->width;
		h = s->height;
		glColor3ub(220,150,160);
		glBegin(GL_QUADS);
			glVertex2i(-w, -h);
			glVertex2i(-w,  h);
			glVertex2i( w,  h);
			glVertex2i( w, -h);
		glEnd();
		glPopMatrix();
		//
		//Draw the particle here
		glPushMatrix();
		glColor3ub(150,160,220);
		Vec *c;
		for (int i=0; i<g.nparticle; i++) {
			c = &g.particle[i].s.center;
			w =
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
	for (int i=0; i<10; i++)
        makeParticle(g.box[0].center.x ,g.box[0].center.y + 100 );


    r.bot =  g.box[0].center.y - 2;
    r.left = g.box[0].center.x + 20;
    int difs = g.box[0].center.y - g.box[1].center.y;
    r.center = 30;


    ggprint8b(&r, difs, 0xffffffff, "Requirements");
    r.bot =  g.box[1].center.y - 2;
    r.left = g.box[1].center.x + 0;



    ggprint8b(&r, difs, 0xffffffff, "Design");
    r.bot =  g.box[2].center.y - 2;
    r.left = g.box[2].center.x + 6;



    ggprint8b(&r, difs, 0xffffffff, "Coding");
    r.bot =  g.box[3].center.y - 2;
    r.left = g.box[3].center.x + 2;

    ggprint8b(&r, difs, 0xffffffff, "Testing");
    r.bot =  g.box[4].center.y - 2;
    r.left = g.box[4].center.x + 6;

    ggprint8b(&r, difs,  0xffffffff, "Maintenance");

	
}






