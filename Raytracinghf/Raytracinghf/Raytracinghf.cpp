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
	bool isDirVector = false;

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

	Vector normalize() {
		return (*this) * (1 / (Length()));
	}
	void print() {
		std::cout << x << ' ' << y << ' ' << z << '\n';
	}
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
	Color operator-(const Color& c) {
		return Color(r - c.r, g - c.g, b - c.b);
	}
	void print() {
		std::cout << r << ' ' << g << ' ' << b << '\n';
	}
};

const int screenWidth = 600;	// alkalmazås ablak felbontåsa
const int screenHeight = 600;
int maxDepth = 10;
float epszilon = 0.001;

struct Matrix {
	MatrixBase m = {
		{ 1,0,0,0 },
		{ 0,1,0,0 },
		{ 0,0,1,0 },
		{ 0,0,0,1 }
	};
	Matrix() {}
	Matrix(MatrixBase mm) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				m[i][j] = mm[i][j];
			}
		}
	}

	Vector operator*(Vector& v) {
		Vector res;
		/*res.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.isDirVector;
		res.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.isDirVector;
		res.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.isDirVector;
		res.z = m[0][3] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.isDirVector;*/
		res.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.isDirVector;
		res.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.isDirVector;
		res.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.isDirVector;
		return res;
	}
	Matrix operator*(Matrix& mm) {
		Matrix res;
		/*res.m[0][0] = m[0][0] * mm.m[0][0] + m[0][1] * mm.m[1][0] + m[0][2] * mm.m[2][0] + m[0][3] * mm.m[3][0];
		res.m[0][1] = m[0][0] * mm.m[0][1] + m[0][1] * mm.m[1][1] + m[0][2] * mm.m[2][1] + m[0][3] * mm.m[3][1];
		res.m[0][0] = m[0][0] * mm.m[0][0] + m[0][1] * mm.m[1][0] + m[0][2] * mm.m[2][0] + m[0][3] * mm.m[3][0];*/

		for (int k = 0; k < 4; ++k) {
			for (int j = 0; j < 4; ++j) {
				float ress = 0;
				for (int i = 0; i < 4; ++i) {
					ress += m[k][i] * mm.m[i][j];
				}
				res.m[k][j] = ress;
			}
		}
		return res;
	}
	void print() {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				std::cout << m[i][j] << ' ';
			}
			std::cout << '\n';
		}
	}
};

struct ScaleMatrix :public Matrix {
	ScaleMatrix(float x, float y, float z) {
		m[0][0] = x;
		m[1][1] = y;
		m[2][2] = z;
	}
};

struct TranslateMatrix :public Matrix {
	TranslateMatrix(float x, float y, float z) {
		m[0][3] = x;
		m[1][3] = y;
		m[2][3] = z;
	}
};

struct RotateX :public Matrix {
	RotateX(float cosfi, float sinfi) {
		m[1][1] = cosfi;
		m[1][2] = sinfi;
		m[1][2] = -sinfi;
		m[2][2] = cosfi;
	}
};
ScaleMatrix scale(0.5f, 1.5f, 0.5f);
ScaleMatrix invscale(1.0f / 0.5f, 1.0f / 1.5f, 1.0f / 0.5f);



Color image[screenWidth*screenHeight];	// egy alkalmazås ablaknyi kÊp
struct Material {
	Color F0;
	float n = 1.0f;
	Color kd, ks;
	bool isReflective, isRefRactive;
	float  shininess = 50;
	virtual Vector reflect(Vector V, Vector N) {
		return V - N*(N*V)*2.0f;
	}
	/*Vector refract(Vector V, Vector N) {
	float ior = n;
	float cosalfa = -(N*V);
	if (cosalfa < 0) { cosalfa = -cosalfa; N = N*(-1.0f); ior = 1.0f / n; }
	float cosnegyzetbeta = 1.0f - (1.0f - cosalfa*cosalfa) / (ior*ior);
	if (cosnegyzetbeta < 0) { return reflect(V, N); }
	return V *(1.0f / ior) + N*(cosalfa / ior - sqrtf(cosnegyzetbeta));
	}*/
	Vector refract(Vector V, Vector N, float eta) {
		float k = 1.0f - eta*eta*(1.0f - N*V)*(N*V);
		if (k < 0) { return Vector(0, 0, 0); }
		else {
			return V*eta - N*(eta*(N*V) + sqrtf(k));
		}
	}
	Color Fresnel(Vector V, Vector N) {
		float cosalfa = fabs(V*N);
		return F0 + (Color(1.0f, 1.0f, 1.0f) - F0)*powf(1.0f - cosalfa, 5.0f);
	}
	Color shade(Vector normal, Vector viewDir, Vector lightDir, Color inRad)
	{
		Color reflRad(0, 0, 0);
		float cosTheta = normal* lightDir;
		if (cosTheta < 0) return reflRad;
		reflRad = kd*inRad * cosTheta;
		//inRad.print();
		//std::cout << cosTheta<<'\n';
		Vector halfway = (viewDir + lightDir).normalize();
		float cosDelta = normal* halfway;
		if (cosDelta < 0) return reflRad;
		return reflRad + ks*inRad * pow(cosDelta, shininess);
	}
};


struct Hit {
	float t;
	Vector pos;
	Vector normal;
	Material material;
	Hit() { t = -1.0f; }
	Hit(float t, Vector pos, Vector nor, Material mat) :t(t), pos(pos), normal(nor), material(mat) {}
};

struct Intersectable
{
	Material material;
	virtual Hit intersect(Vector o, Vector d) = 0;
};

struct Sphere : public Intersectable {
	Vector center;
	float radius;

	Hit intersect(Vector o, Vector d) {
		//float t1 = ((o - center)*d*(-2.0f) + sqrtf(powf((o - center)*d*(2.0f), 2) - ((o - center)*(o - center) - radius*radius)*4.0f*(d*d))) / ((d*d)*(d*d));
		//float t2 = ((o - center)*d*(-2.0f) - sqrtf(powf((o - center)*d*(2.0f), 2) - ((o - center)*(o - center) - radius*radius)*4.0f*(d*d))) / ((d*d)*(d*d));
		float t1, t2, tGood;
		o.isDirVector = false;
		d.isDirVector = true;
		Vector ot = invscale*o;
		Vector dt = (invscale*d).normalize();
		dt.isDirVector = true;
		float underSqrt = powf(ot*dt*(2.0f), 2) - (ot*ot - 1.0f)*4.0f*(dt*dt);
		if (underSqrt > 0) {
			t1 = (ot*dt*(-2.0f) + sqrtf(underSqrt)) / ((dt*dt)*(dt*dt));
			t2 = (ot*dt*(-2.0f) - sqrtf(underSqrt)) / ((dt*dt)*(dt*dt));
			/*t1 = (o*d*(-2.0f) + sqrtf(underSqrt)) / ((d*d)*(d*d));
			t2 = (o*d*(-2.0f) - sqrtf(underSqrt)) / ((d*d)*(d*d));*/
			if (t1 < t2) { tGood = t1; }
			else { tGood = t2; }
			//return Vector(o.x + d.x*tGood, o.y + d.y*tGood, o.z + d.z*tGood);
			Vector pos = Vector(o.x + d.x*tGood, o.y + d.y*tGood, o.z + d.z*tGood);
			Vector dir = pos.normalize();
			dir.isDirVector = true;
			Vector post = scale*pos;
			Vector dirt = (scale*dir).normalize();
			float tVeryGood = (post - o).Length();
			return Hit(tVeryGood, post, dirt, material);
		}
		else {
			return Hit();
		}
	}
};
struct Plane :public Intersectable {
	Vector P, N;
	Hit intersect(Vector o, Vector d) {
		float t = ((P - o)*N) / (d*N);
		Vector pos = Vector(o + d*t);
		//pos.print();
		return Hit(t, pos, N, material);
	}
};

struct Paraboloid :public Intersectable {
	Vector P;
	Hit intersect(Vector o, Vector d) {
		float undersqrt = powf((o.x*d.x + o.z*d.z - d.y)*2.0f, 2.0f) - (o.x*o.x + o.z*o.z - o.y)*(d.x*d.x + d.z*d.z)*4.0f;
		if (undersqrt > 0) {

		}
	}
};
struct Ray {
	Vector o, d;
	Ray() {}
	Ray(Vector o, Vector d) :o(o), d(d) {}
	Ray(const Ray& r) :o(r.o), d(r.d) {}

};


struct Camera {
	Vector pos, lookat, ahead, up, right;
	Ray getRay(float goUp, float goRight) {
		Ray r;
		r.o = lookat + right*(2.0f*goRight / 600.0f - 1) + up*(2.0f*goUp / 600.0f - 1);
		r.d = (r.o - pos).normalize();
		return r;
	}
};

struct Light {
	Vector pos;
	float power = 5.0f;
	Color color = Color(1.0f, 1.0f, 1.0f);
};
Sphere gomb;
Plane planes[5];
Camera cam;
Light light;


Hit firstIntersect(Ray r) {
	Hit bestHit, hit;
	bestHit.t = 50000000.0f;
	hit = gomb.intersect(r.o, r.d);
	if (hit.t < bestHit.t && hit.t>0) { bestHit = hit; }
	for (int i = 0; i < 5; ++i) {
		hit = planes[i].intersect(r.o, r.d);
		if (hit.t < bestHit.t && hit.t>0) { bestHit = hit; }
	}
	return bestHit;
}


Color trace(Ray r, int depth, bool isprevMaterialAir) {
	if (depth > maxDepth) { return Color(); }
	Hit hit = firstIntersect(r);
	if (hit.t < 0) { return Color(/*hit.material.kd*0.1f*/); }
	Color outRadiance = Color();
	Vector Li = Vector(light.pos - hit.pos);
	Vector V = Vector(r.d*(-1.0f));
	//std::cout << V.Length()<<'\n';
	Color inRadiance = light.color*light.power*(1 / (1 + (powf(Li.Length(), 2.0f))));
	Ray shadowRay = Ray(hit.pos + hit.normal*epszilon, Li.normalize());
	Hit shadowHit = firstIntersect(shadowRay);
	if (shadowHit.t<0 || shadowHit.t>Li.Length()) {
		outRadiance = outRadiance + hit.material.shade(hit.normal, V, Li.normalize(), inRadiance);
	}
	if (hit.material.isReflective) {
		bool isThismatAir;
		if (hit.material.n == 1.0f) { isThismatAir = true; }
		else { isThismatAir = false; }
		Vector reflectionDir = hit.material.reflect(V, hit.normal);
		Ray reflectedRay = Ray(hit.pos + hit.normal*epszilon, reflectionDir);
		outRadiance = outRadiance + trace(reflectedRay, depth + 1, isThismatAir)*hit.material.Fresnel(V, hit.normal);
	}
	if (hit.material.isRefRactive) {
		float eta;
		if (isprevMaterialAir == true) { eta = 1.0f / 1.5f; }
		else { eta = 1.5f / 1.0f; }
		bool isThismatAir;
		if (hit.material.n == 1.0f) { isThismatAir = true; }
		else { isThismatAir = false; }
		Vector refractionDir = hit.material.refract(V, hit.normal, eta);
		Ray refractedRay = Ray(hit.pos + hit.normal*epszilon, refractionDir);
		outRadiance = outRadiance + trace(refractedRay, depth + 1, isThismatAir)*(Color(1.0f, 1.0f, 1.0f) - hit.material.Fresnel(V, hit.normal));
	}
	return outRadiance;
}

void toneMap(Color& color) {
	Vector I = Vector(0.21f, 0.72f, 0.07f);
	float L = I*(Vector((color.r + 0.0001f), (color.g + 0.0001f), (color.b + 0.0001f)));
	float Lf = L / (1 + L);
	color = color*(Lf / L);
	color.r = powf(color.r, 1.0f / 2.2f);
	color.g = powf(color.g, 1.0f / 2.2f);
	color.b = powf(color.b, 1.0f / 2.2f);
	if (color.r > 1.0f || color.g > 1.0f || color.b > 1.0f) {
		color.print();
	}
}

void render() {
	for (int Y = 0; Y < screenHeight; Y++) {
		for (int X = 0; X < screenWidth; X++) {
			Ray r = cam.getRay(X, Y);
			Color color = trace(r, 0, 0);
			toneMap(color);
			image[Y*screenWidth + X] = color;
		}
	}
}


// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization() {
	glViewport(0, 0, screenWidth, screenHeight);
	light.pos = Vector(-4.0f, 0.0f, -4.0f);
	cam.pos = Vector(0.0f, 0, -4.0f);
	cam.lookat = Vector(0.0f, 0, -3.0f);
	cam.ahead = Vector((cam.lookat - cam.pos).normalize());
	cam.up = Vector(0.0f, 1.0f, 0.0f);
	cam.right = Vector(1.0f, 0.0f, 0.0f);

	/*for (int Y = 0; Y < screenHeight; Y++) {
	for (int X = 0; X < screenWidth; X++) {
	image[Y*screenWidth + (X-1)] = Color(0.0f, 1.0f, 1.0f);
	}
	}*/

	gomb.center = Vector();
	gomb.radius = 1.0f;
	gomb.material.kd = Color(0.0f, 0.0f, 0.0f);
	gomb.material.ks = Color(0.0f, 0.0f, 0.0f);
	gomb.material.shininess = 80.0f;
	gomb.material.n = 1.55277;
	gomb.material.isReflective = true;
	gomb.material.isRefRactive = true;
	//gomb.material.F0 = Color((powf(0.17f - 1.0f, 2.0f) + powf(3.1f, 2.0f)) / (powf(0.17f + 1.0f, 2.0f) + powf(3.1f, 2.0f)), (powf(0.35f - 1.0f, 2.0f) + powf(2.7f, 2.0f)) / (powf(0.35f + 1.0f, 2.0f) + powf(2.7f, 2.0f)), (powf(1.5f - 1.0f, 2.0f) + powf(1.9f, 2.0f)) / (powf(1.5f + 1.0f, 2.0f) + powf(1.9f, 2.0f)));
	gomb.material.F0 = Color((powf(1.5f - 1.0f, 2.0f)) / (powf(1.5f + 1.0f, 2.0f)), (powf(1.5f - 1.0f, 2.0f)) / (powf(1.5f + 1.0f, 2.0f)), (powf(1.5f - 1.0f, 2.0f)) / (powf(1.5f + 1.0f, 2.0f)));

	planes[0].P = Vector(0.0f, -5.0f, 0.0f);
	planes[0].N = Vector(0.0f, 1.0f, 0.0f);
	planes[0].material.kd = Color(1.0f, 1.0f, 0.0f);
	planes[0].material.ks = Color(0, 1.0f, 0);
	planes[0].material.isReflective = false;
	planes[0].material.isRefRactive = false;

	planes[1].P = Vector(0.0f, 5.0f, 0.0f);
	planes[1].N = Vector(0.0f, -1.0f, 0.0f);
	planes[1].material.kd = Color(0.0f, 1.0f, 1.0f);
	planes[1].material.ks = Color(1.0f, 0.0f, 0);
	planes[1].material.isReflective = false;
	planes[1].material.isRefRactive = false;

	planes[2].P = Vector(5.0f, 0.0f, 0.0f);
	planes[2].N = Vector(-1.0f, 0.0f, 0.0f);
	planes[2].material.kd = Color(0.0f, 1.0f, 0.0f);
	planes[2].material.ks = Color(0, 1.0f, 0);
	planes[2].material.isReflective = false;
	planes[2].material.isRefRactive = false;

	planes[3].P = Vector(-5.0f, 0.0f, 0.0f);
	planes[3].N = Vector(1.0f, 0.0f, 0.0f);
	planes[3].material.kd = Color(0.0f, 0.0f, 1.0f);
	planes[3].material.ks = Color(0.0f, 1.0f, 0);
	planes[3].material.isReflective = false;
	planes[3].material.isRefRactive = false;

	planes[4].P = Vector(0.0f, 0.0f, 10.0f);
	planes[4].N = Vector(0.0f, 0.0f, -1.0f);
	planes[4].material.kd = Color(1.0f, 0.0f, 0.0f);
	planes[4].material.ks = Color(1.0f, 1.0f, 0);
	planes[4].material.isReflective = false;
	planes[4].material.isRefRactive = false;

	render();
	/*Vector nor = light.pos.normalize();
	nor.print();
	float f = cam.ahead.Length();
	float g = light.pos.Length();
	std::cout << f;*/



	/*			MatrixBase mm = {
	{1,2,3,4},
	{2,2,2,2},
	{3,3,3,3},
	{1,1,1,1}
	};
	MatrixBase mmm = {
	{ 2,1,3,4 },
	{ 2,1,2,2 },
	{ 3,2,3,3 },
	{ 1,3,1,1 }
	};
	Matrix a(mm);
	Matrix b(mmm);
	Matrix c = b*a;
	a.print();
	c.print();*/




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
	glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_FLOAT, image);
	// Majd rajzolunk egy kek haromszoget
	/*glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex2f(0.0f, 0.0f);
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
