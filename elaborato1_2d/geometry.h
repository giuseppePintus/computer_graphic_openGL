
#include "Lib.h"
#define PI 3.14159265358979323846

using namespace std;
using namespace glm;

typedef struct
{
	GLuint VAO;
	GLuint VBO_G;
	GLuint VBO_C;
	int nTriangles;
	// Vertici
	vector<vec3> vertici;
	vector<vec4> colors;
	// Numero vertici
	int nv;
	// Matrice di Modellazione: Traslazione*Rotazione*Scala
	mat4 Model;
	vec3 corner_b;
	vec3 corner_t;
	vector<vec3> corner;
	vec3 size;
	vec3 position;
	vec3 velocity;
	vec3 acceleration;
	float direction;
	float linearVel;
	float linearAcc;
	int live;

	GLuint EBO_indici;
	vector<vec3> CP;
	vector<int> indici;
} Figura;
// int sceltaVS;
// int sceltaFS;
// string name;
int pval = 140;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

vec4 crossVec4(vec4 _v1, vec4 _v2)
{
	vec3 vec1 = vec3(_v1[0], _v1[1], _v1[2]);
	vec3 vec2 = vec3(_v2[0], _v2[1], _v2[2]);
	vec3 res = cross(vec1, vec2);
	return vec4(res[0], res[1], res[2], 1);
}
bool intersection(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D)
{
	glm::vec3 direction_AB = B - A;
	glm::vec3 direction_CD = D - C;
	glm::vec3 cross_product = glm::cross(direction_AB, direction_CD);
	if (glm::length(cross_product) < 1e-6)
	{
		return false;
	}
	glm::vec4 line_AB = glm::vec4(glm::cross(direction_AB, A), 0.0f);
	glm::vec4 line_CD = glm::vec4(glm::cross(direction_CD, C), 0.0f);
	glm::vec4 intersection_point = crossVec4(line_AB, line_CD);
	if ((0 <= intersection_point.w && intersection_point.w <= 1) &&
		(0 <= intersection_point.z && intersection_point.z <= 1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void boundingBox(Figura *fig)
{
	fig->corner_b = vec4(fig->vertici[0],1.0);
	fig->corner_t = vec4(fig->vertici[0],1.0);
	float maxx=fig->vertici[0].x, minx=fig->vertici[0].x;
	float maxy=fig->vertici[0].y, miny=fig->vertici[0].y;
	for (std::vector<vec3>::iterator it = fig->vertici.begin(); it != fig->vertici.end(); it++)
	{
		vec3 i = *it;
		if (minx > i.x)
		{
			fig->corner_b.x = i.x;
			minx = i.x;
		}
		if (maxx < i.x)
		{
			fig->corner_t.x = i.x;
			maxx = i.x;
		}

		if (miny > i.y)
		{
			fig->corner_b.y = i.y;
			miny = i.y;
		}
		if (maxy < i.y)
		{
			fig->corner_t.y = i.y;
			maxy = i.y;
		}
	}
	fig->corner.push_back(vec3(maxx, miny, 0.0));
	fig->corner.push_back(vec3(maxx, miny, 0.0));
	fig->corner.push_back(vec3(minx, maxy, 0.0));
	fig->corner.push_back(vec3(minx, maxy, 0.0));
}
bool checkBoundingBox(Figura *A, Figura *B)
{
	/*//segments intersection not working
	for (int i = 0; i < A->corner.size(); i++)
	{
		vec4 a = A->Model * vec4(A->corner[(i + 1) % A->corner.size()], 1.0);
		vec4 b = A->Model * vec4(A->corner[i % A->corner.size()], 1.0);
		for (int j = 0; j < B->corner.size(); j++)
		{
			vec4 C = B->Model * vec4(B->corner[j % B->corner.size()], 1.0);
			vec4 D = B->Model * vec4(B->corner[(j + 1) % B->corner.size()], 1.0);

			if (intersection(vec3(a.x, a.y, a.z), vec3(b.x, b.y, b.z), vec3(C.x, C.y, C.z), vec3(D.x, D.y, D.z)))
				return true;
		}
	}
	return false;*/
	
	float Amaxx=(A->Model * vec4(A->corner[0],1.0)).x, Aminx=(A->Model * vec4(A->corner[0],1.0)).x;
	float Amaxy=(A->Model * vec4(A->corner[0],1.0)).y, Aminy=(A->Model * vec4(A->corner[0],1.0)).y;

	float Bmaxx=(B->Model * vec4(B->corner[0],1.0)).x, Bminx=(B->Model * vec4(B->corner[0],1.0)).x;
	float Bmaxy=(B->Model * vec4(B->corner[0],1.0)).y, Bminy=(B->Model * vec4(B->corner[0],1.0)).y;

	for (std::vector<vec3>::iterator it = A->corner.begin(); it != A->corner.end(); it++)
	{
		vec4 i = A->Model * vec4(*it, 1.0);
		if (Amaxx < i.x)
		{
			Amaxx = i.x;
		}
		if (Amaxy < i.y)
		{
			Amaxy = i.y;
		}
		if (Aminx > i.x)
		{
			Aminx = i.x;
		}
		if (Aminy > i.y)
		{
			Aminy = i.y;
		}
	}
	for (std::vector<vec3>::iterator it = B->corner.begin(); it != B->corner.end(); it++)
	{
		vec4 i = B->Model * vec4(*it, 1.0);
		if (Bmaxx < i.x)
		{
			Bmaxx = i.x;
		}
		if (Bmaxy < i.y)
		{
			Bmaxy = i.y;
		}
		if (Bminx > i.x)
		{
			Bminx = i.x;
		}
		if (Bminy > i.y)
		{
			Bminy = i.y;
		}
	}
	// collisione asse x
	bool cornerbot = Aminx > Bminx && Aminx < Bmaxx && Aminy > Bminy && Aminy < Bmaxy;

	// collisione asse y
	bool cornertop = Amaxx > Bminx && Amaxx < Bmaxx && Amaxy > Bminy && Amaxy < Bmaxy;
	// se ho una collisione fermo esco
	if (cornerbot || cornertop)
	{
		return true;
	}
	return false;
}
void costruisci_player(Figura *fig)
{
	vec4 gun = vec4(vec4(0.7, 0.7, 0.7, 1.0));
	vec4 body = vec4(vec4(0.3, 0.3, 0.6, 1.0));
	vec4 head = vec4(vec4(0.7, 0.1, 0.1, 1.0));
	int i;
	float stepA = (PI) / fig->nTriangles;
	float t;

	fig->vertici.push_back(vec3(0.0, 0.4, 0.0));

	fig->colors.push_back(head);

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(0.25 * cos(t), 0.4 + 0.25 * sin(t), 0.0));
		// Colore
		fig->colors.push_back(head);
	}

	// head
	fig->vertici.push_back(vec3(-0.5, -0.25, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(-0.25, 0.4, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(0.25, 0.4, 0.0));
	fig->colors.push_back(body);

	fig->vertici.push_back(vec3(-0.5, -0.25, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(0.25, 0.4, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(0.5, -0.25, 0.0));
	fig->colors.push_back(body);

	// trhuster
	fig->vertici.push_back(vec3(-0.25, -0.25, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(0.25, -0.25, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(-0.15, -0.5, 0.0));
	fig->colors.push_back(body);

	fig->vertici.push_back(vec3(0.25, -0.25, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(-0.15, -0.5, 0.0));
	fig->colors.push_back(body);
	fig->vertici.push_back(vec3(0.15, -0.5, 0.0));
	fig->colors.push_back(body);
	// gun
	fig->vertici.push_back(vec3(-0.15, 0.5, 0.0));
	fig->colors.push_back(gun);
	fig->vertici.push_back(vec3(0.15, 0.0, 0.0));
	fig->colors.push_back(gun);
	fig->vertici.push_back(vec3(0.15, 0.5, 0.0));
	fig->colors.push_back(gun);

	fig->vertici.push_back(vec3(-0.15, 0.5, 0.0));
	fig->colors.push_back(gun);
	fig->vertici.push_back(vec3(-0.15, 0.0, 0.0));
	fig->colors.push_back(gun);
	fig->vertici.push_back(vec3(0.15, 0.0, 0.0));
	fig->colors.push_back(gun);

	fig->nv = fig->vertici.size();
	boundingBox(fig);
}
void costruisci_proiettile(float cx, float cy, float raggiox, float raggioy, Figura *fig)
{

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;

	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(255.0 / 255.0, 75.0 / 255.0, 0.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * cos(t), cy + raggioy * sin(t), 0.0));
		// Colore
		fig->colors.push_back(vec4(1.0, 204.0 / 255.0, 0.0, 1.0));
	}

	fig->nv = fig->vertici.size();

	boundingBox(fig);
}

void costruisciAlieno(float cx, float cy, float raggiox, float raggioy, Figura *fig)
{

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;

	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(150.0 / 255.0, 75.0 / 255.0, 0.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * (sin(t) * (exp(cos(t)) - 2 * cos(4 * t)) + pow(sin(t / 12), 5)) / 4, cy + raggioy * (cos(t) * (exp(cos(t)) - 2 * cos(4 * t)) + pow(sin(t / 12), 5)) / 4, 0.0));
		// Colore
		fig->colors.push_back(vec4(1.0, 0.0, 0.0, 0.0));
	}
	fig->nv = fig->vertici.size();
	boundingBox(fig);
}

void costruisciStella(Figura *fig, vec4 color1, vec4 color2)
{

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;

	fig->vertici.push_back(vec3(0.0, 0.0, 0.0));

	fig->colors.push_back(color1);

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;

		float x = (1 - 0.7 * pow(pow(cos(2.5 * t), 2), 0.15)) * cos(t);

		float y = (1 - 0.7 * pow(pow(cos(2.5 * t), 2), 0.15)) * sin(t);
		fig->vertici.push_back(vec3(x, y, 0.0));

		// Colore
		fig->colors.push_back(color2);
	}
	fig->nv = fig->vertici.size();
	//boundingBox(fig);
}
void costruisci_cuore(float cx, float cy, float raggiox, float raggioy, Figura *fig)
{

	int i;
	float stepA = (2 * PI) / fig->nTriangles;
	float t;

	fig->vertici.push_back(vec3(cx, cy, 0.0));

	fig->colors.push_back(vec4(255.0 / 255.0, 75.0 / 255.0, 0.0, 1.0));

	for (i = 0; i <= fig->nTriangles; i++)
	{
		t = (float)i * stepA;
		fig->vertici.push_back(vec3(cx + raggiox * (16 * pow(sin(t), 3)) / 16, cy + raggioy * ((13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t)) / 16), 0.0));
		// Colore
		fig->colors.push_back(vec4(1.0, 0.0 / 255.0, 0.0, 1.0));
	}
	fig->nv = fig->vertici.size();
	//boundingBox(fig);
}

/// /////////////////////////////////// Disegna geometria //////////////////////////////////////
// Per Curve di hermite
#define PHI0(t) (2.0 * t * t * t - 3.0 * t * t + 1)
#define PHI1(t) (t * t * t - 2.0 * t * t + t)
#define PSI0(t) (-2.0 * t * t * t + 3.0 * t * t)
#define PSI1(t) (t * t * t - t * t)
float dx(int i, float *t, float Tens, float Bias, float Cont, Figura *Fig)
{
	if (i == 0)
		return 0.5 * (1 - Tens) * (1 - Bias) * (1 - Cont) * (Fig->CP[i + 1].x - Fig->CP[i].x) / (t[i + 1] - t[i]);
	if (i == Fig->CP.size() - 1)
		return 0.5 * (1 - Tens) * (1 - Bias) * (1 - Cont) * (Fig->CP[i].x - Fig->CP[i - 1].x) / (t[i] - t[i - 1]);

	if (i % 2 == 0)
		return 0.5 * (1 - Tens) * (1 + Bias) * (1 + Cont) * (Fig->CP.at(i).x - Fig->CP.at(i - 1).x) / (t[i] - t[i - 1]) + 0.5 * (1 - Tens) * (1 - Bias) * (1 - Cont) * (Fig->CP.at(i + 1).x - Fig->CP.at(i).x) / (t[i + 1] - t[i]);
	else
		return 0.5 * (1 - Tens) * (1 + Bias) * (1 - Cont) * (Fig->CP.at(i).x - Fig->CP.at(i - 1).x) / (t[i] - t[i - 1]) + 0.5 * (1 - Tens) * (1 - Bias) * (1 + Cont) * (Fig->CP.at(i + 1).x - Fig->CP.at(i).x) / (t[i + 1] - t[i]);
}
float dy(int i, float *t, float Tens, float Bias, float Cont, Figura *Fig)
{
	if (i == 0)
		return 0.5 * (1 - Tens) * (1 - Bias) * (1 - Cont) * (Fig->CP.at(i + 1).y - Fig->CP.at(i).y) / (t[i + 1] - t[i]);
	if (i == Fig->CP.size() - 1)
		return 0.5 * (1 - Tens) * (1 - Bias) * (1 - Cont) * (Fig->CP.at(i).y - Fig->CP.at(i - 1).y) / (t[i] - t[i - 1]);

	if (i % 2 == 0)
		return 0.5 * (1 - Tens) * (1 + Bias) * (1 + Cont) * (Fig->CP.at(i).y - Fig->CP.at(i - 1).y) / (t[i] - t[i - 1]) + 0.5 * (1 - Tens) * (1 - Bias) * (1 - Cont) * (Fig->CP.at(i + 1).y - Fig->CP.at(i).y) / (t[i + 1] - t[i]);
	else
		return 0.5 * (1 - Tens) * (1 + Bias) * (1 - Cont) * (Fig->CP.at(i).y - Fig->CP.at(i - 1).y) / (t[i] - t[i - 1]) + 0.5 * (1 - Tens) * (1 - Bias) * (1 + Cont) * (Fig->CP.at(i + 1).y - Fig->CP.at(i).y) / (t[i + 1] - t[i]);
}

void InterpolazioneHermite(float *t, Figura *Fig, vec4 color_extern, vec4 color_center)
{
	float p_t = 0, p_b = 0, p_c = 0, x, y;
	float passotg = 1.0 / (float)(pval - 1);
	float tg = 0, tgmapp, ampiezza;
	int i = 0;
	int is = 0; // indice dell'estremo sinistro dell'intervallo [t(i),t(i+1)] a cui il punto tg
				// appartiene

	for (tg = 0; tg <= 1; tg += passotg)
	{
		if (tg > t[is + 1])
			is++;

		ampiezza = (t[is + 1] - t[is]);
		tgmapp = (tg - t[is]) / ampiezza;

		x = Fig->CP[is].x * PHI0(tgmapp) + dx(is, t, p_t, p_b, p_c, Fig) * PHI1(tgmapp) * ampiezza + Fig->CP[is + 1].x * PSI0(tgmapp) + dx(is + 1, t, p_t, p_b, p_c, Fig) * PSI1(tgmapp) * ampiezza;
		y = Fig->CP[is].y * PHI0(tgmapp) + dy(is, t, p_t, p_b, p_c, Fig) * PHI1(tgmapp) * ampiezza + Fig->CP[is + 1].y * PSI0(tgmapp) + dy(is + 1, t, p_t, p_b, p_c, Fig) * PSI1(tgmapp) * ampiezza;
		Fig->vertici.push_back(vec3(x, y, 0.0));
		Fig->colors.push_back(color_extern);
	}
}

void costruisci_alieno(vec4 color_extern, vec4 color_center, Figura *forma)
{

	float *t;
	forma->CP.push_back(vec3(1.0, 0.0, 0.0));
	for (int quad = 0; quad < 4; quad++)
	{

		int n = rand() % 3 + 1;
		int w = 0, bisec = (n * 2 + 1);
		forma->CP.push_back(vec3(cos(quad * PI / 2), sin(quad * PI / 2), 0.0));
		for (int w = 0; w < bisec; w++)
		{
			float ang = w * PI / (2 * bisec);
			if (w % 2 == 0)
			{
				forma->CP.push_back(vec3(0.4 * cos(quad * PI / 2.0 + ang), 0.4 * sin(quad * PI / 2.0 + ang), 0.0));
			}
			else
			{
				float curve = PI / (4 * bisec);
				forma->CP.push_back(vec3(0.5 * cos(quad * PI / 2.0 + (ang - curve)), 0.5 * sin(quad * PI / 2.0 + (ang - curve)), 0.0));
				forma->CP.push_back(vec3(cos(quad * PI / 2.0 + ang), sin(quad * PI / 2.0 + ang), 0.0));
				forma->CP.push_back(vec3(0.5 * cos(quad * PI / 2.0 + (ang + curve)), 0.5 * sin(quad * PI / 2.0 + (ang + curve)), 0.0));
			}
		}
		forma->CP.push_back(vec3(cos((quad + 1) * PI / 2), sin((quad + 1) * PI / 2), 0.0));
	}

	t = new float[forma->CP.size()];
	int i;
	float step = 1.0 / (float)(forma->CP.size() - 1);

	for (i = 0; i < forma->CP.size(); i++)
		t[i] = i * step;

	forma->vertici.push_back(vec3(0.0, 0.0, 0.0));
	forma->colors.push_back(color_center);
	InterpolazioneHermite(t, forma, color_extern, color_center);
	forma->nv = forma->vertici.size();
	boundingBox(forma);
}