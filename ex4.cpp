/*********************************
 * Ergasia 1 - Askhsh 4 - 15/01/22
 * Chatzimichail Antonios-Kyrillos - 2657 
 * Karaiskos Charalampos - 2765
 * Peteinarelis Charalampos - 2804
 *********************************/

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MARK_POINTS 1
#define FILL_POLYGON 2
#define PICK_RED_COLOR 3
#define PICK_GREEN_COLOR 4
#define PICK_BLUE_COLOR 5
#define CLEAR_SCREEN 6

#define NOT_ACTIVE 0
#define ACTIVE 1
#define IGNORED -1

#define MAX_POINTS 10

//#define DEBUG_MODE 1

GLint screenWidth= 640;
GLint screenHeight =480;

int menu; // for menu creation
int mode; // for menu choices
static int numCorners = 0;

int numActive = 0;

 typedef struct 
 { 
	 GLint x,y;
 }
 GLintPoint;
 
 typedef struct 
 { 
	 int y_min;
	 int y_max;
	 int x_min;
	 double x_step;
	 double x_curr;
	 int is_active;
 }
 edge_T;

GLintPoint corners[MAX_POINTS];
edge_T edges[MAX_POINTS] = {0};

int compare_Ymin(const void *s1, const void *s2){
	edge_T *e1 = (edge_T *)s1;
	edge_T *e2 = (edge_T *)s2;
	
	int ydiff = (e1->y_min) - (e2->y_min);
	
	if(ydiff == 0)
		return (e1->x_min) - (e2->x_min);
	
	return ydiff;
}

int compare_Xcurr(const void *s1, const void *s2){
	edge_T *e1 = (edge_T *)s1;
	edge_T *e2 = (edge_T *)s2;
	
	return round(e1->x_curr) - round(e2->x_curr);
}

void createEdgesFromPoints(GLintPoint* points){
	for(int i=1; i<=numCorners; i++){
		GLintPoint curr = points[i%numCorners];
		GLintPoint prev = points[(i-1)%numCorners];
		int y_max, y_min, x_y_max, x_y_min;
		if(curr.y > prev.y){
			y_max = curr.y;
			x_y_max = curr.x;
			y_min = prev.y;
			x_y_min = prev.x;
		}
		else{
			y_max = prev.y;
			x_y_max = prev.x;
			y_min = curr.y;
			x_y_min = curr.x;
		}
		edges[i-1].y_max = y_max;
		edges[i-1].y_min = y_min;
		edges[i-1].x_min = (curr.x < prev.x) ? curr.x : prev.x;
		edges[i-1].x_curr = x_y_min;
		if(y_max - y_min != 0){
			edges[i-1].x_step = (x_y_max - x_y_min) / (double)(y_max - y_min);
			edges[i-1].is_active = NOT_ACTIVE;
		}
		else{
			edges[i-1].x_step = 0;
			edges[i-1].is_active = IGNORED;
		}
	}
	
	qsort(edges, numCorners, sizeof(edge_T), compare_Ymin);
}

void printEdges(){
	printf("***Edges***\n");
	for(int i=0; i<numCorners; i++){
		printf("%d: y_min=%d, y_max=%d, x_min=%d, x_step=%lf, x_curr=%lf, is_active=%d\n", i, edges[i].y_min, edges[i].y_max, edges[i].x_min, edges[i].x_step, edges[i].x_curr, edges[i].is_active);
	}
}

void drawPoint( int ax, int ay)
{
	glBegin(GL_POINTS);
		glVertex2i(ax,ay);
	glEnd();
	glFlush();
}

void drawPointNoFlush( int ax, int ay)
{
	glBegin(GL_POINTS);
		glVertex2i(ax,ay);
	glEnd();
}

void drawPoints(GLintPoint* points, int numPoints){
	for(int i=0; i<numPoints; i++)
		drawPoint(points[i].x, points[i].y);
}

/*void drawLine(int ax, int ay, int bx, int by)
{
	glBegin(GL_LINES);
		glVertex2i(ax,ay);
		glVertex2i(bx, by);
	glEnd();
	glFlush();
}*/

int find_max_y_max(){
	int max;
	if(numCorners == 0)
		return -1;
	
	max = edges[0].y_max;
	for(int i=1; i<numCorners; i++){
		if(max < edges[i].y_max)
			max = edges[i].y_max;
	}
	
	return max;
}

void drawScanLine(int y_i){
	
	/*printf("y_i=%d\n", y_i);
	printEdges();	*/
	
	numActive = 0;
	//'edges' is sorted with respect to y_min
	for(int i=0; i<numCorners; i++){
		if(edges[i].y_min > y_i)
			break;
		if(edges[i].y_min == y_i){
			if(edges[i].is_active == NOT_ACTIVE)
				edges[i].is_active = ACTIVE;
		}
	}
	
	//sort 'edges' with respect to x_curr
	qsort(edges, numCorners, sizeof(edge_T), compare_Xcurr);
	//paint the active pairs
	for(int i=0; i<numCorners; i++){
		if(edges[i].is_active != ACTIVE)
			continue;
		
		numActive++;
		for(int j=i+1; j<numCorners; j++){
			if(edges[j].is_active != ACTIVE)
				continue;
			
			numActive++;
			for(int x_i=round(edges[i].x_curr); x_i<round(edges[j].x_curr); x_i++){
				drawPointNoFlush(x_i, y_i);
			}
			i = j;
			break;
		}
	}
	
	#ifdef DEBUG_MODE
	if(numActive%2!=0){
		printf("POSSIBLE BUG: y_i=%d, numActive=%d\n", y_i, numActive); 
		//printEdges();
	}
	#endif
	
	for(int i=0; i<numCorners; i++){
		if(edges[i].is_active == ACTIVE){
			//set the inactive edges
			if(y_i >= edges[i].y_max - 1){
				edges[i].is_active = NOT_ACTIVE;
				continue;
			}
			//calculate x values
			edges[i].x_curr += edges[i].x_step;
		}
				
	}
	
	
	//sort 'edges' with respect to y_min again
	qsort(edges, numCorners, sizeof(edge_T), compare_Ymin);
}

//draw ScanLines in [y_from, y_to)
void drawScanLines(int y_from, int y_to){
	for(int y_i=y_from; y_i < y_to; y_i++)
		drawScanLine(y_i);
	
	glFlush();
}

void myInit (void)
{
	//glClearColor(0.5, 0.5, 0.5, 0.0);    // select clearing (background) color
	glClear(GL_COLOR_BUFFER_BIT);		// clear the screen
	//glColor3f(1.0, 1.0, 1.0);           // initialize viewing color values
	glColor3f(0.0,0.0,1.0);
	glPointSize(4);						// set point size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0,(GLfloat)screenWidth, 0.0,(GLfloat)screenHeight);// Creates a matrix for projecting two dimensional
																	// coordinates onto the screen and multiplies the 
																	// current projection matrix by it.
																	// It is used for projecting a two dimensional image 
																	// onto a two dimensional screen. 
																	// For two dimensional objects using two dimensional 
																	// vertex commands all z coordinates are zero.
	glFlush();
	
	printf("How to use:\n");
	printf("press RIGHT mouse button\n");
	printf("select 'Mark points' and LEFT click to mark points\n");
	printf("select 'Fill polygon' to fill the polygon formed by the marked points, using the YX algorithm\n");
	printf("select 'Pick color' to change color\n");
	printf("select 'Clear screen' to clear points and fill\n");
	printf("press 'Q' to quit\n");
}
										

void myDisplay(void)
{
	glClearColor(0.5, 0.5, 0.5, 0.0);    // select clearing (background) color
	glClear(GL_COLOR_BUFFER_BIT);		// clear the screen
	drawPoints(corners, numCorners);
	createEdgesFromPoints(corners);
	if(numCorners > 0 && mode == FILL_POLYGON)
		drawScanLines(edges[0].y_min, find_max_y_max());
	glFlush();							// send all output to display
}

 //Specifying points with the mouse
void myMouse (int button, int state, int x1, int y1)
{
	//static GLintPoint corner[MAX_POINTS];
	//static int numCorners = 0;
	
	if (button == GLUT_LEFT_BUTTON && state ==GLUT_DOWN)
	{		
		if(mode == MARK_POINTS)
		{
			if(numCorners >= MAX_POINTS){
				printf("WARNING: Maximum number of points reached (%d)\n", numCorners);
				return;
			}
			corners[numCorners].x =x1;
			corners[numCorners].y = screenHeight-y1;
			drawPoint(corners[numCorners].x,corners[numCorners].y);
			numCorners++;
		}
		//printf("numCorners=%d\n", numCorners);
	}
	
		
	glFlush();
	
}
	// triggered when a Keyboard key is pressed
void keyEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
		//Quit
		case 'Q':
		case 'q':
		exit(-1);
		break;
		default:
		break;
	}
}

// Menu choices
void myMenu(int selection)
{
	switch(selection) {

		case MARK_POINTS: // draw points
			mode = selection;
			glutPostRedisplay(); // put comments to see what happens
			break;

		case FILL_POLYGON: // drawing rectangle
			mode = selection;
			//printEdges();
			glutPostRedisplay();
			break;

		case PICK_RED_COLOR:
			glColor3f(1.0,0.0,0.0);
			glutPostRedisplay();
			break;

		case PICK_GREEN_COLOR:
			glColor3f(0.0,1.0,0.0);
			glutPostRedisplay();
			break;

		case PICK_BLUE_COLOR:
			glColor3f(0.0,0.0,1.0);
			glutPostRedisplay();
			break;

		case CLEAR_SCREEN:
			numCorners = 0;
			glClear(GL_COLOR_BUFFER_BIT);  // clear the window
			glFlush();
			break;

		default:
			printf("Invalid menu selection\n");
			break;
	}

}



int main(int argc, char **argv)
{

	int menu, pickColorSubmenu;
	
	// initialize toolkit
	glutInit(&argc, argv);
	// set display mode
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	// set window size
	glutInitWindowSize (screenWidth, screenHeight);
	// set window initial position
	glutInitWindowPosition(1,1);
	// creates a window
	glutCreateWindow ("HW1, Ex4 - YX Algorithm to fill polygons");
	// registers a redraw function, this function is called whenever 
	// the system determines that the window should be redrawn.
	// This first happens when the window is first opened.
	glutDisplayFunc(myDisplay);
	// registers a keyboard handler
	glutKeyboardFunc(keyEvent);
	glutMouseFunc(myMouse);
	
	// Create a two-level pop-up menu
	pickColorSubmenu = glutCreateMenu(myMenu);
	glutAddMenuEntry("Red", PICK_RED_COLOR);
	glutAddMenuEntry("Green", PICK_GREEN_COLOR);
	glutAddMenuEntry("Blue", PICK_BLUE_COLOR);

	// menu creation
	menu = glutCreateMenu(myMenu);
	glutAddMenuEntry("Mark points", MARK_POINTS);
	glutAddMenuEntry("Fill polygon", FILL_POLYGON);
	glutAddSubMenu("Pick color", pickColorSubmenu);
	glutAddMenuEntry("Clear Screen", CLEAR_SCREEN);

	// register menu to right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//Initializations
	myInit();

	// perpetual ever processing loop
	glutMainLoop();

	glutDestroyMenu(pickColorSubmenu);
	glutDestroyMenu(menu);

	return 0;
}

