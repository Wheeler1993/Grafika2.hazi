// Raytracinghf.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2014-tol.          
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk. 
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat. 
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni (printf is fajlmuvelet!)
// - new operatort hivni az onInitialization fßggvÊnyt kivÊve, a lefoglalt adat korrekt felszabadítåsa nÊlkßl 
// - felesleges programsorokat a beadott programban hagyni
// - tovabbi kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan gl/glu/glut fuggvenyek hasznalhatok, amelyek
// 1. Az oran a feladatkiadasig elhangzottak ES (logikai AND muvelet)
// 2. Az alabbi listaban szerepelnek:  
// Rendering pass: glBegin, glVertex[2|3]f, glColor3f, glNormal3f, glTexCoord2f, glEnd, glDrawPixels
// Transzformaciok: glViewport, glMatrixMode, glLoadIdentity, glMultMatrixf, gluOrtho2D, 
// glTranslatef, glRotatef, glScalef, gluLookAt, gluPerspective, glPushMatrix, glPopMatrix,
// Illuminacio: glMaterialfv, glMaterialfv, glMaterialf, glLightfv
// Texturazas: glGenTextures, glBindTexture, glTexParameteri, glTexImage2D, glTexEnvi, 
// Pipeline vezerles: glShadeModel, glEnable/Disable a kovetkezokre:
// GL_LIGHTING, GL_NORMALIZE, GL_DEPTH_TEST, GL_CULL_FACE, GL_TEXTURE_2D, GL_BLEND, GL_LIGHT[0..7]
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : <VEZETEKNEV(EK)> <KERESZTNEV(EK)>
// Neptun : <NEPTUN KOD>
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy 
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem. 
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a 
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb 
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem, 
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.  
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat 
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>

#if defined(__APPLE__)                                                                                                                                                                                                            
#include <OpenGL/gl.h>                                                                                                                                                                                                            
#include <OpenGL/glu.h>                                                                                                                                                                                                           
#include <GLUT/glut.h>                                                                                                                                                                                                            
#else                                                                                                                                                                                                                             
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)                                                                                                                                                                       
#include <windows.h>                                                                                                                                                                                                              
#endif                                                                                                                                                                                                                            
#include <GL/gl.h>                                                                                                                                                                                                                
#include <GL/glu.h>                                                                                                                                                                                                               
#include <GL/glut.h>                                                                                                                                                                                                              
#endif          

#include<iostream>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

//--------------------------------------------------------
// 3D Vektor
//--------------------------------------------------------
typedef float MatrixBase[4][4];

struct Vector {
	float x, y, z;
	bool isDirVector;

	Vector() {
		x = y = z = 0;
	}
	Vector(float x0, float y0, float z0 = 0) {
		x = x0; y = y0; z = z0;
	}
	Vector operator*(float a) {
		return Vector(x * a, y * a, z * a);
	}
	Vector operator+(const Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	Vector operator-(const Vector& v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	float operator*(const Vector& v) { 	// dot product
		return (x * v.x + y * v.y + z * v.z);
	}
	Vector operator%(const Vector& v) { 	// cross product
		return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}
	
	float Length() { return sqrt(x * x + y * y + z * z); }
};

//--------------------------------------------------------
// Spektrum illetve szin
//--------------------------------------------------------
struct Color {
	float r, g, b;

	Color() {
		r = g = b = 0;
	}
	Color(float r0, float g0, float b0) {
		r = r0; g = g0; b = b0;
	}
	Color operator*(float a) {
		return Color(r * a, g * a, b * a);
	}
	Color operator*(const Color& c) {
		return Color(r * c.r, g * c.g, b * c.b);
	}
	Color operator+(const Color& c) {
		return Color(r + c.r, g + c.g, b + c.b);
	}
};

const int screenWidth = 600;	// alkalmazås ablak felbontåsa
const int screenHeight = 600;

struct Matrix {
	MatrixBase m;

	Vector operator*(Vector& v) {
		Vector res;
			res.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.isDirVector;
			res.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.isDirVector;
			res.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.isDirVector;
	}
	Matrix operator*(Matrix& mm) {
		Matrix res;
		/*res.m[0][0] = m[0][0] * mm.m[0][0] + m[0][1] * mm.m[1][0] + m[0][2] * mm.m[2][0] + m[0][3] * mm.m[3][0];
		res.m[0][1] = m[0][0] * mm.m[0][1] + m[0][1] * mm.m[1][1] + m[0][2] * mm.m[2][1] + m[0][3] * mm.m[3][1];
		res.m[0][0] = m[0][0] * mm.m[0][0] + m[0][1] * mm.m[1][0] + m[0][2] * mm.m[2][0] + m[0][3] * mm.m[3][0];*/
		
		for (int k = 0; k < 3; ++k) {
			for (int j = 0; j < 3; ++j) {
				float ress = 0;
				for (int i = 0; i < 3; ++i) {
					ress += m[0][i] * mm.m[i][0];
				}
				res.m[k][j] = ress;
			}
		}
		return res;
	}
	void print() {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				std::cout << m[i][j] << ' ';
			}
			std::cout << '\n';
		}
	}
};


//Color image[screenWidth*screenHeight];	// egy alkalmazås ablaknyi kÊp
class SmoothMaterial {
	Vector F0;
	float n;
	Vector reflect(Vector V, Vector N) {
		return V - N*(N*V)*2.0f;
	}

	Vector refract(Vector V, Vector N) {
		float ior = n;
		float cosalfa = -(N*V);
		if (cosalfa < 0) { cosalfa = -cosalfa; N = N*(-1.0f); ior = 1.0f / n; }
		float cosnegyzetbeta = 1.0f - (1.0f - cosalfa*cosalfa) / (ior*ior);
		if (cosnegyzetbeta < 0) { return reflect(V, N); }
		return V *(1.0f / ior) + N*(cosalfa / ior - sqrtf(cosnegyzetbeta));
	}

	Vector Fresnel(Vector V, Vector N) {
		float cosalfa = fabs(V*N);
		return F0 + (Vector(1, 1, 1) - F0)*powf(1 - cosalfa, 5);
	}

};
										// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);
	Matrix a;
	MatrixBase mm = {
		{1,2,3,4},
		{2,2,2,2},
		{3,3,3,3},
		{1,1,1,1}
	};
	a.m = mm;
	// Peldakent keszitunk egy kepet az operativ memoriaba
	/*for (int Y = 0; Y < screenHeight; Y++)
		for (int X = 0; X < screenWidth; X++)
			image[Y*screenWidth + X] = Color((float)X / screenWidth, (float)Y / screenHeight, 0);*/

}

// Rajzolas, ha az alkalmazas ablak ervenytelenne valik, akkor ez a fuggveny hivodik meg
void onDisplay() {
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);		// torlesi szin beallitasa
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

														// ..

														// Peldakent atmasoljuk a kepet a rasztertarba
	//glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);
	// Majd rajzolunk egy kek haromszoget
	/*glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex2f(-0.2f, -0.2f);
	glVertex2f(0.2f, -0.2f);
	glVertex2f(0.0f, 0.2f);
	glEnd();*/

	// ...

	glutSwapBuffers();     				// Buffercsere: rajzolas vege

}

// Billentyuzet esemenyeket lekezelo fuggveny (lenyomas)
void onKeyboard(unsigned char key, int x, int y) {
	if (key == 'd') glutPostRedisplay(); 		// d beture rajzold ujra a kepet

}

// Billentyuzet esemenyeket lekezelo fuggveny (felengedes)
void onKeyboardUp(unsigned char key, int x, int y) {

}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)   // A GLUT_LEFT_BUTTON / GLUT_RIGHT_BUTTON illetve GLUT_DOWN / GLUT_UP
		glutPostRedisplay(); 						 // Ilyenkor rajzold ujra a kepet
}

// Eger mozgast lekezelo fuggveny
void onMouseMotion(int x, int y)
{

}

// `Idle' esemenykezelo, jelzi, hogy az ido telik, az Idle esemenyek frekvenciajara csak a 0 a garantalt minimalis ertek
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido

}

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char **argv) {
	glutInit(&argc, argv); 				// GLUT inicializalasa
	glutInitWindowSize(600, 600);			// Alkalmazas ablak kezdeti merete 600x600 pixel 
	glutInitWindowPosition(100, 100);			// Az elozo alkalmazas ablakhoz kepest hol tunik fel
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	// 8 bites R,G,B,A + dupla buffer + melyseg buffer

	glutCreateWindow("Grafika hazi feladat");		// Alkalmazas ablak megszuletik es megjelenik a kepernyon

	glMatrixMode(GL_MODELVIEW);				// A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);			// A PROJECTION transzformaciot egysegmatrixra inicializaljuk
	glLoadIdentity();

	onInitialization();					// Az altalad irt inicializalast lefuttatjuk

	glutDisplayFunc(onDisplay);				// Esemenykezelok regisztralasa
	glutMouseFunc(onMouse);
	glutIdleFunc(onIdle);
	glutKeyboardFunc(onKeyboard);
	glutKeyboardUpFunc(onKeyboardUp);
	glutMotionFunc(onMouseMotion);

	glutMainLoop();					// Esemenykezelo hurok

	return 0;
}