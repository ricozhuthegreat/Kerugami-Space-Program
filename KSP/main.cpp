
#include <windows.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

// This struct represents one unit of a point in 3-dimensional space
struct Point3D
{
    double x;
    double y;
    double z;
};

// This struct is used to represent one object (loaded from a .obj file)
struct Object
{

    vector<Point3D> vertices;
    vector<Point3D> normals;
    vector<int> triangles;
    vector<int> polygons;
    vector<int> elements;

    double maxX;
    double minX;

    double maxY;
    double minY;

    double maxZ;
    double minZ;

};

struct Union
{
    vector<vector<Object> > components;
};

// This function returns the magnitude of a given Point3D vector
double getMagnitude (Point3D p)
{
    return sqrt((p.x * p.x) + (p.y * p.y) + (p.z * p.z));
}

// This method returns a new Point3D with subtracted values
Point3D subtractP3D (Point3D p1, Point3D p2)
{
    Point3D p3;
    p3.x = p1.x - p2.x;
    p3.y = p1.y - p2.y;
    p3.z = p1.z - p2.z;
    return p3;
}

// This method does cross multiplication on the 2 input points
Point3D crossMultiplyP3D (Point3D p1, Point3D p2)
{
    Point3D p3;
    p3.x = (p1.y * p2.z) - (p2.y * p1.z);
    p3.x = (p1.z * p2.x) - (p2.z * p1.x);
    p3.x = (p1.x * p2.y) - (p2.x * p1.y);
    return p3;
}

// This method returns a new normalized unit vector Point3D of given Point3D p
Point3D normalize (Point3D p)
{
    double magnitude = getMagnitude(p);
    Point3D np;
    np.x = (p.x/magnitude);
    np.y = (p.y/magnitude);
    np.z = (p.z/magnitude);
    return np;
}

// This method returns a new loaded .obj file into the program as a new vector of objects
vector<Object> loadObject (string fName) {

    // New vector to load the program files to
    vector<Object> objects;

    // Create a new input file stream reader that will parse the .obj object model file
    ifstream fParser(fName.c_str());

    if (!fParser)
    {
        cout << "Invalid File!" << endl;
        return objects;
    }

    // String variable used to store each line
    string line;

    // Create a new blank object for the first case
    Object newO;
    objects.push_back(newO);

    // Integer variable used to keep track of the current index of the object
    int cObj = 0;

    // These double variables are used to record the maximum x, y and z values of the current object. Used later for scaling and normalization
    double maxX, maxY, maxZ = -1000000;
    // These double variables are used to record the minimum x, y and z values of the current object. Used later for scaling and normalization
    double minX, minY, minZ = 1000000;

    // Start the reading call for the entire file
    while (getline(fParser, line))
    {

        // Check to see if the current line decalres a new object
        if (line.substr(0,2) == "o ")
        {

            Object newO;
            objects.push_back(newO);
            cObj++;

            cout << "New object added" << endl;

        }
        else if (line.substr(0,2) == "v ")
        {

            // The current line contains a point for a vertex. Create a point and push it to the back of the current object's vertex
            Point3D tempP;

            istringstream sParser(line.substr(2));
            sParser >> tempP.x;
            //sParser >> tempP.y;
            //sParser >> tempP.z;

            sParser >> tempP.z;
            sParser >> tempP.y;

            // Check to see if each coordinate is the currently assumed greatest value (maximum)
            if (tempP.x > maxX) {
                maxX = tempP.x;
            }

            if (tempP.y > maxY) {
                maxY = tempP.y;
            }

            if (tempP.z > maxZ) {
                maxZ = tempP.z;
            }

            // Check to see if each coordinate is the currently assumed smallest value (minimum)
            if (tempP.x < minX) {
                minX = tempP.x;
            }

            if (tempP.y < minY) {
                minY = tempP.y;
            }

            if (tempP.z < minZ) {
                minZ = tempP.z;
            }

            objects[cObj].vertices.push_back(tempP);

        }
        else if (line.substr(0,2) == "f ")
        {

            // Get the amount of vertices and determine whether it is a triangle or a polygon

            // Assume that the current line contains a new face composed of 4 vertex indices only for now
            string as, bs, cs, ds;

            // Tokenize the string line and read each token
            istringstream sParser(line.substr(2));

            sParser >> as;
            sParser >> bs;
            sParser >> cs;
            sParser >> ds;

            int a = stoi(as.substr(0,as.find("/")));
            int b = stoi(bs.substr(0,bs.find("/")));
            int c = stoi(cs.substr(0,cs.find("/")));

            // Decrement indices (.obj indices frustratingly start at 1)
            a--;
            b--;
            c--;

            // Check to see if a fourth vertice exists
            if (ds != "") {

                // If the last string token isn't blank, a fourth vertice exists and is added

                int d = stoi(ds.substr(0,ds.find("/")));
                d--;

                objects[cObj].polygons.push_back(a);
                objects[cObj].polygons.push_back(b);
                objects[cObj].polygons.push_back(c);
                objects[cObj].polygons.push_back(d);

            } else {

                // If the last string token is blank, fill triangles instead of polygons
                objects[cObj].triangles.push_back(a);
                objects[cObj].triangles.push_back(b);
                objects[cObj].triangles.push_back(c);

            }

        }

    }

    // Update the max and min coordinate parameters in each object
    for (Object &obj : objects) {

        obj.maxX = maxX;
        obj.minX = minX;

        obj.maxY = maxY;
        obj.minY = minY;

        obj.maxZ = maxZ;
        obj.minZ = minZ;

    }

    cout << "Models Loaded!" << endl;
}

// This method takes in an object and scales it down to an input range
Object scaleObject (Object obj, double nMaxX, double nMinX, double nMaxY, double nMinY, double nMaxZ, double nMinZ) {

        double aspectRatio = (obj.maxY - obj.minY) / (obj.maxX - obj.minX);

        nMaxX *= (1/aspectRatio);
        nMinX *= (1/aspectRatio);

        // Object struct for the new point
        Object nObj = obj;

        vector<Point3D> nVertices;

        for (Point3D p : obj.vertices) {

            // Temporary point variable to be pushed back
            Point3D temp;

            // Scale the x, y and z to be between their respective nMax and nMin values (normalization range)

            temp.x = ( ( (p.x - obj.minX) * (nMaxX - nMinX) ) / (obj.maxX - obj.minX) ) + nMinX;
            temp.y = ( ( (p.y - obj.minY) * (nMaxY - nMinY) ) / (obj.maxY - obj.minY) ) + nMinY;
            temp.z = ( ( (p.z - obj.minZ) * (nMaxZ - nMinZ) ) / (obj.maxZ - obj.minZ) ) + nMinZ;

            nVertices.push_back(temp);

        }

        nObj.vertices = nVertices;

        return nObj;

}

// This void method draws vector of objects with a translation of xpos, ypos, zpos
void drawObject (vector<Object> objects, double xpos, double ypos, double zpos) {

    for (int m=0; m<objects.size(); m++) {

        vector<Point3D> vertices = objects[m].vertices;
        vector<int> triangles = objects[m].triangles;
        vector<int> polygons = objects[m].polygons;

        for (int i=0; i<triangles.size(); i+=3) {

            // First element refers to index of vertex, draw triangle based off of points
            Point3D cVertex1 = vertices[triangles[i]];
            Point3D cVertex2 = vertices[triangles[i+1]];
            Point3D cVertex3 = vertices[triangles[i+2]];

            glBegin(GL_LINE_STRIP);

                glVertex3d(cVertex1.x + xpos, cVertex1.y + ypos, cVertex1.z + zpos);
                glVertex3d(cVertex2.x + xpos, cVertex2.y + ypos, cVertex2.z + zpos);
                glVertex3d(cVertex3.x + xpos, cVertex3.y + ypos, cVertex3.z + zpos);

            glEnd();

        }

        for (int i=0; i<polygons.size(); i+=4) {

            // First element refers to index of vertex, draw triangle based off of points
            Point3D cVertex1 = vertices[polygons[i]];
            Point3D cVertex2 = vertices[polygons[i+1]];
            Point3D cVertex3 = vertices[polygons[i+2]];
            Point3D cVertex4 = vertices[polygons[i+3]];

            glBegin(GL_LINE_STRIP);

                glVertex3d(cVertex1.x + xpos, cVertex1.y + ypos, cVertex1.z + zpos);
                glVertex3d(cVertex2.x + xpos, cVertex2.y + ypos, cVertex2.z + zpos);
                glVertex3d(cVertex3.x + xpos, cVertex3.y + ypos, cVertex3.z + zpos);
                glVertex3d(cVertex4.x + xpos, cVertex4.y + ypos, cVertex4.z + zpos);

            glEnd();

        }

    }
}

// This integer will represent the current stage of the game
int stage = 0;

// All Rocket objects/components loaded into the program to be used for custom rocket construction
vector<vector<Object> > components;
// The menu displaying the components for the player to select and add to the assembly
vector<vector<Object> > menu;
// This is a list of all objects that a user has selected but not applied to the rocket (i.e., in the "workspace" but not in assembly)
vector<vector<Object> > workspace;

// The union assembly variable used to represent the final assembly to be used in the simulation
Union assembly;

// Index/ID of the currently selected component to be moved (initialize with null value)
int selected = -1;
// Index/ID of the currently selected menu part to be added to the main assembly (initialize with null value)
int menuSelection = -1;

// The x and y coordinates of the left mouse click, scaled between 0 and 1000 (used to determine which menu item has been clicked
double leftX = -1;
double leftY = -1;

// The translation values of the currently selected component
double sxpos = 0;
double sypos = 0;
double szpos = 0;

// Boolean variable stating whether the middle mouse button is being held
bool mhold = false;
// Global Perspective (gp) middle mouse button values (delta x, delta y, current x + y)
int gpx1, gpx2, gpy1, gpy2, gpcx, gpcy;

// This void method takes in a list of items to be displayed "rotating" in display menu. The screen is assumed to be (0, 1000, 0, 1000, -1000, 1000). The method pipes the scaled models to the menu global vector
void initMenu (vector<vector<Object> > items) {

    // Draw each of the objects in the menu. Scale down first and then draw
    for (vector<Object> objList : items) {

        // Create a new temporary vector of objects to store the scaled model
        vector<Object> temp;

        for (Object obj : objList) {

            Object nObj = scaleObject(obj, 200, 0, 200, 0, 200, -200);
            temp.push_back(nObj);

        }

        // Push the new temporary vector into the global menu variable
        menu.push_back(temp);

    }

}

// This void method draws the menu vector on the side
void drawMenu () {

    // Double variables used to keep track of the starting position of each draw
    double startX = 0.0;
    double startY = 750.0;
    double startZ = 1000.0;

    for (vector<Object> component : menu) {

        // Set the polyon color to white
        glColor3f(1, 1, 1);

        // Draw a square polygon surrounding the current object
        glBegin(GL_POLYGON);
            glVertex3d(startX + 250, startY, startZ);
            glVertex3d(startX + 250, startY + 250, startZ);
            glVertex3d(startX, startY + 250, startZ);
            glVertex3d(startX, startY, startZ);
        glEnd();

        // Draw a bounding box around the polygon

        // Set the line drawing color to black
        glColor3f(0, 0, 0);

        // Draw a square polygon surrounding the current object
        glBegin(GL_LINE_LOOP);
            glVertex3d(startX, startY, 1200);
            glVertex3d(startX + 250, startY, 1200);
            glVertex3d(startX + 250, startY + 250, 1200);
            glVertex3d(startX, startY + 250, 1200);
        glEnd();

        // Draw the actual mini-sized model

        glColor3f(0, 0, 1);

        drawObject(component, startX, startY, startZ);
        startY -= 250;

    }

}

// This method checks to see if any components needs to be added to the workspace
void updateWorkspace () {

    // Check to see if the mouse left click selection lands on a valid menu item. Each menu item is bounded by a 250 by 250 box
    if (leftX <= 250) {

        int index = (int) (leftY/250);

        cout << index << endl;

        // Check to see if the X value is inside the range of the menu
        if (index < menu.size()) {
            // Update the menu item at the selected menu "square" by adding it to the workspace
            workspace.push_back(components[index]);
        }

    }

}

// This void method takes in a nested vector of objects and pipes all subobjects as objects in the assembly union. It also clears the entire workspace
void assembleComponents(vector <vector <Object> > parts) {

    // Iterate through every object group
    for (int i=0; i<parts.size(); i++) {

        // Get the current set of components to be added to the assembly and removed from the workspace
        vector<Object> objects = parts[i];

        // Add the current sub-component/object group to the main assembly
        assembly.components.push_back(objects);
        workspace.erase(workspace.begin() + i);

    }

}

// This void method increments all the points at index in the components vector (pre-setting a translation)
void setPreTranslate (vector< vector <Object> > &components, int index, int nx, int ny, int nz) {

    // Get the pointer objects vector to be modified
    vector<Object> &objects = components[selected];

    // Iterate through all "sub-objects" within the current object and update each vertice seperately
    for (int m=0; m<objects.size(); m++) {

        // Get all vertices in the current sub-object
        vector<Point3D> &vertices = objects[m].vertices;

        // Iterate over all of the current vertices and increment each
        for (int v=0; v<vertices.size(); v++) {
            // Increment the current vertice
            Point3D &cVertex = vertices[v];
            cVertex.x += nx;
            cVertex.y += ny;
            cVertex.z += nz;
        }

    }

}

// This void method draws the entire rocket assembly screen
void drawRocketAssembly () {

    // Draw each of the different components in the current assembly
    for (int i=0; i<assembly.components.size(); i++) {

        // Get the current obect to be drawn
        vector<Object> obj = assembly.components[i];

        double xtrans = 500;
        double ytrans = 500;
        double ztrans = 0;

        // Set color to the ompleted assembly union color black
        glColor3d(0,0,0);

        glPushMatrix();

            glMatrixMode(GL_MODELVIEW);

            // Rotate the global perspective
            glRotated(gpcx, 0, 1000, 0);
            glRotated(gpcy, 1000, 0, 0);

            drawObject(obj, xtrans, ytrans, ztrans);

        glPopMatrix();

    }

    // Draw each of the different components in the current workspace (excluding assembly)
    for (int i=0; i<workspace.size(); i++) {

        // Get the current obect to be drawn
        vector<Object> obj = workspace[i];

        double xtrans = 500;
        double ytrans = 500;
        double ztrans = 0;

        // Determine drawing color depending on whether the current element is the selected one to be moved
        if (i == selected) {
            // The element is selected, set draw color to green
            glColor3d(0,1,0);
            // Also move the component
            xtrans += sxpos;
            ytrans += sypos;
            ztrans += szpos;

        } else {
            // Otherwise, not selected. Set to default draw color (blue)
            glColor3d(0,0,1);
        }

        glPushMatrix();

            glMatrixMode(GL_MODELVIEW);

            // Rotate the global perspective
            glRotated(gpcx, 0, 1000, 0);
            glRotated(gpcy, 1000, 0, 0);

            drawObject(obj, xtrans, ytrans, ztrans);

        glPopMatrix();

    }

    // Draw the components menu
    drawMenu();

}

void display(void) {

    // Clear the current color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glOrtho(0, 1000, 0, 1000, -1200, 1200);

    // Draw the rocket assembly screen
    drawRocketAssembly();

    // Flushing the matrix to the cache for double buffering
    glutSwapBuffers();

}

// Set the idle animation
void idle(void) {
    glutPostRedisplay();
}

// This method opens the .obj files and parses them to load the object models
void init() {

    // Blank template Point3D struct for future reference
    Point3D blank;
    blank.x = 0.0;
    blank.y = 0.0;
    blank.z = 0.0;

    // The number of components
    int fLength = 3;

    // The directory locatino od the .obj object model file
    string fName[] = {"C://Users/ricoz/Desktop/C++ Workspace/LoadOBJ/booster.obj", "C://Users/ricoz/Desktop/C++ Workspace/LoadOBJ/apollo.obj", "C://Users/ricoz/Desktop/C++ Workspace/LoadOBJ/rocket2.obj"};

    for (int i=0; i<fLength; i++) {
        components.push_back(loadObject(fName[i]));
    }

    // Initialize the menu once the components have been loaded
    initMenu(components);

}

void manipulateObjects (int x, int y) {

    if (mhold) {

        // Center mouse button is being held, update newest x and y coordinate changes

        gpx1 = gpx2;
        gpy1 = gpy2;

        gpx2 = x;
        gpy2 = y;

        // Get the change in x and y
        double dx = gpx2 - gpx1;
        double dy = gpy2 - gpy1;

        // Update cx and cy (the current rotation angle) depending on whether dx and dy was increasing or decreasing
        if (dx < 0) {
            // Move to the left
            gpcx += 1;
        } else if (dx > 0) {
            // Move the the right
            gpcx -= 1;
        }

        if (dy < 0) {
            // Move up
            gpcy -= 1;
        } else if (dy > 0) {
            // Move down
            gpcy += 1;
        }

        // Update the screen
        glutPostRedisplay();

    }

}

// This method will listen for all mouse button controls. This includes adjusting the global assembly perspective and
void mouseListner (int button, int state, int x, int y) {

    // Global viewing perspective changes (middle mouse button)

    // Get the change in x and change in x to determine rotation angle. Use solidworks controls (center mouse button while held)
    if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {

        // Update mouse down to alert mouse move check
        mhold = true;

        // Center mouse button is being held, update newest x and y coordinate changes

        gpx1 = gpx2;
        gpy1 = gpy2;

        gpx2 = x;
        gpy2 = y;

    } else {
        // Mouse button is not being held
        mhold = false;
    }

    // Menu component selection (left mouse button click)

    // Listner activated for a menu selection. Update the click position
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        // Scale the click position to be between 0 and 1000 instead of 0 and 600

        leftX = (double) x * (1000.0/600.0);
        leftY = (double) y * (1000.0/600.0);

        // Update the workspace
        updateWorkspace();

    }

}

void manipulateSelectedComponent (unsigned char key, int x, int y) {

    // Check to see if the key pressed was to select a certain component in the workspace (ascii values 48 - 57)
    if (key >= 48 && key <= 57 && (key - 48) < workspace.size()) {

        if (selected != -1) {
            // Update all the point values of the previous object
            setPreTranslate(workspace, selected, sxpos, sypos, szpos);
        }

        // The key is a number key and the value is valid (there are that many componenets)
        selected = key - 48;

        // Reset the translation position of the object
        sxpos = 0;
        sypos = 0;
        szpos = 0;

    }

    // Move the appropriate objects if an object is selected and a key is pressed to mvoe the selected component
    if (selected != -1 && key == 'w') {
        // Increase the y value of the selected component
        sypos += 1;
    } else if (selected != -1 && key == 's') {
        // Decrease the y value of the selected component
        sypos -= 1;
    } else if (selected != -1 && key == 'a') {
        // Decrease the x value of the selected component
        sxpos -= 1;
    } else if (selected != -1 && key == 'd') {
        // Increase the x value of the selected component
        sxpos += 1;
    } else if (selected != -1 && key == 'p') {
        // Increase the z value of the selected component
        szpos += 1;
    } else if (selected != -1 && key == 'l') {
        // Decrease the z value of the selected component
        szpos -= 1;
    }

    if (key == 'u') {

        // If there has been a translation/modification
        if (selected != -1) {
            // Update all the point values of the current workspace
            setPreTranslate(workspace, selected, sxpos, sypos, szpos);
        }

        // Assemble all existing components in the workspace together
        assembleComponents(workspace);
        // Reset the selected variable as no item is selected
        selected = -1;
    }

}

// Arrays mapping to texture and lighting/shading constants
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 1000.0f, 1000.0f, 0.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

int main( int argc, char **argv )
{
    // Initialize the new frame and clear the depth buffer
    glutInit( &argc, argv );
    init();
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( 600, 600 );
    glutCreateWindow( "GLUT .obj Demo" );

    // Set the display function to draw the solid
    glutDisplayFunc(display);
    // Set the idle animation funciton
    glutIdleFunc(idle);
    // Set the mouse event animation funciton
    glutMouseFunc(mouseListner);
    // Set the mouse motion/move function
    glutMotionFunc(manipulateObjects);
    // Set the keyboard function
    glutKeyboardFunc(manipulateSelectedComponent);
    // Clear the background
    glClearColor(1,1,1,1);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable materials rendering
 /*   glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);*/

    glutMainLoop();
    return 0;
}