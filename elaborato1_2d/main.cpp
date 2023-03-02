#include <iostream>
#include <ctime>
#include "ShaderMaker.h"
#include "Lib.h"
#include <algorithm>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "geometry.h"
using namespace std;
using namespace glm;
static unsigned int programId;
mat4 Projection;
GLuint MatProj, MatModel, loctime, locresolution;
bool hitbox = false;
int nv_P;
// viewport size
int width = 1280;
int height = 720;

int accelerate = 0;
float timer = 0.0, spawn = 0.0, cooldown = 0.0;
int nEnemies = 0;

vector<Figura> Scena;
vector<Figura> Proiettili;
vector<Figura> Nemici;
vector<Figura> Stars;
Figura player = {};
Figura life = {};

vec4 Col()
{
	int ran = rand() % 5;
	switch (ran)
	{
	case 1: // yellow
		return vec4(255.0 / 255.0, 255.0 / 255.0, 0 / 255.0, 1.0);
		break;
	case 2: // red
		return vec4(255.0 / 255.0, 0 / 255.0, 0 / 255.0, 1.0);
		break;
	case 3: // green
		return vec4(0 / 255.0, 255.0 / 255.0, 0 / 255.0, 1.0);
		break;
	case 4: // blue
		return vec4(0 / 255.0, 0 / 255.0, 255.0 / 255.0, 1.0);
		break;
	case 5: // gray
		return vec4(205 / 255.0, 205 / 255.0, 0 / 205, 1.0);
		break;
	case 0: // white
		return vec4(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 1.0);
		break;
	}
	return vec4(1.0);
}
void crea_VAO_Vector(Figura *fig)
{

	glGenVertexArrays(1, &fig->VAO);
	glBindVertexArray(fig->VAO);
	// Genero , rendo attivo, riempio il VBO della geometria dei vertici
	glGenBuffers(1, &fig->VBO_G);
	glBindBuffer(GL_ARRAY_BUFFER, fig->VBO_G);
	glBufferData(GL_ARRAY_BUFFER, fig->vertici.size() * sizeof(vec3), fig->vertici.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	// Genero , rendo attivo, riempio il VBO dei colori
	glGenBuffers(1, &fig->VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, fig->VBO_C);
	glBufferData(GL_ARRAY_BUFFER, fig->colors.size() * sizeof(vec4), fig->colors.data(), GL_STATIC_DRAW);
	// Adesso carico il VBO dei colori nel layer 2
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(1);
}
void updatePosition(Figura *proj, float deltaTime)
{
	proj->position = proj->position + proj->velocity * deltaTime + (float)(1 / 2) * proj->acceleration * deltaTime * deltaTime;
	proj->velocity = proj->velocity + proj->acceleration * deltaTime;
	if (distance(proj->velocity, vec3(0.0)) > proj->linearVel)
	{
		proj->velocity = normalize(proj->velocity) * proj->linearVel;
	}
}
void creaProiettile(vec3 pos, vec3 vel, vec3 acc)
{
	Figura *proj = new Figura;
	proj->size = vec3(10);
	proj->nTriangles = 100;
	proj->position = pos;
	proj->velocity = vel * 500.0f;
	proj->linearVel = 500;
	proj->live = 1;
	proj->acceleration = acc;
	costruisci_proiettile(0.0, 0.0, 1.0, 1.0, proj);
	crea_VAO_Vector(proj);
	Proiettili.push_back(*proj);
}

void creaNemico(int life)
{
	Figura *alien = new Figura;
	alien->size = vec3(40);
	alien->nTriangles = 100;
	alien->position = vec3(rand() % width / 2, rand() % height / 2, 0.0) + player.position;
	alien->velocity = vec3(0.0, 0.0, 0.0);
	alien->linearAcc = 60;
	alien->linearVel = 150;
	player.acceleration = normalize(vec3(cos(player.direction + PI / 2), sin(player.direction + PI / 2), 0.0)) * player.linearAcc;
	alien->live = life;
	costruisci_alieno(Col(), Col(), alien);
	crea_VAO_Vector(alien);
	Nemici.push_back(*alien);
}
void creaPlayer()
{
	player.nTriangles = 180;
	costruisci_player(&player);
	crea_VAO_Vector(&player);
	player.position = vec3(600.0, 200.0, 0.0);
	player.direction = 0;
	player.linearAcc = 800;
	player.linearVel = 200;
	player.live = 5;
	player.size = vec3(80.0, 80.0, 1.0);
	Scena.push_back(player);
}
void creaCuore()
{
	life.nTriangles = 100;
	costruisci_cuore(0.0, 0.0, 1.0, 1.0, &life);
	crea_VAO_Vector(&life);
	life.size = vec3(25.0);
}
void updateEnemy(Figura *alien, float deltaTime)
{
	vec3 dir = normalize(player.position - alien->position);
	alien->direction = atan2(dir.x, dir.y);
	alien->acceleration = dir * alien->linearAcc;

	updatePosition(alien, deltaTime);
}
void updatePlayer(float deltaTime)
{

	player.acceleration = normalize(vec3(cos(player.direction + PI / 2), sin(player.direction + PI / 2), 0.0)) * player.linearAcc;
	updatePosition(&player, deltaTime);
}

void removeDead(std::vector<Figura> &entity)
{
	entity.erase(
		std::remove_if(entity.begin(), entity.end(), [&](Figura const &pet)
					   { return pet.live == 0; }),
		entity.end());
}
void drawHitbox(Figura *fig)
{
	Figura *hitbox = new Figura;
	hitbox->vertici.push_back(vec3(fig->corner_b.x, fig->corner_b.y, 0.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->corner_b.x, fig->corner_t.y, 0.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->corner_t.x, fig->corner_t.y, 0.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->corner_t.x, fig->corner_b.y, 0.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->corner_b.x, fig->corner_b.y, 0.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->nv = hitbox->vertici.size();
	/*

	float Amaxx=(fig->Model * vec4(fig->corner[0],1.0)).x, Aminx=(fig->Model * vec4(fig->corner[0],1.0)).x;
	float Amaxy=(fig->Model * vec4(fig->corner[0],1.0)).y, Aminy=(fig->Model * vec4(fig->corner[0],1.0)).y;

	for (std::vector<vec3>::iterator it = fig->corner.begin(); it != fig->corner.end(); it++)
	{
		vec4 i =  fig->Model* vec4(*it, 1.0);
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

	hitbox->vertici.push_back(vec3(Aminx, Aminy, 0.0));
	hitbox->vertici.push_back(vec3(Amaxx, Aminy, 0.0));
	hitbox->vertici.push_back(vec3(Amaxx, Amaxy, 0.0));
	hitbox->vertici.push_back(vec3(Aminx, Amaxy, 0.0));
	hitbox->vertici.push_back(vec3(Aminx, Aminy, 0.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->colors.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->nv = hitbox->vertici.size();

	*/
	crea_VAO_Vector(hitbox);

	glBindVertexArray(hitbox->VAO);
	glDrawArrays(GL_LINE_STRIP, 0, hitbox->nv);
	glBindVertexArray(0);
	/*
	 */
}

void INIT_SHADER(void)
{
	GLenum ErrorCheckValue = glGetError();

	char *vertexShader = (char *)"vertexShader_M.glsl";
	char *fragmentShader = (char *)"fragmentShader_S.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

void INIT_VAO(void)
{

	creaPlayer();
	creaNemico(1);
	creaCuore();
	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
	loctime = glGetUniformLocation(programId, "time");
	locresolution = glGetUniformLocation(programId, "resolution");

	for (int i = 0; i < 150; i++)
	{
		Figura *star = new Figura;

		star->nTriangles = 100;
		star->position = vec3(rand() % width, rand() % height, 0.0);
		vec4 centerCol, outCol;
		float size = 0;
		switch (rand() % 7)
		{
		case 6: // Blue-violet
			size = 30;
			centerCol = vec4(51 / 255.0, 51 / 255.0, 150 / 255.0, 1.0);
			outCol = vec4(102 / 255.0, 0 / 255.0, 150 / 255.0, 1.0);
			break;
		case 1: // Blue-white
			size = 20;
			centerCol = vec4(51 / 255.0, 51 / 255.0, 204 / 255.0, 1.0);
			outCol = vec4(102 / 255.0, 0 / 255.0, 204 / 255.0, 1.0);
			break;
		case 2: // White
			size = 15;
			centerCol = vec4(204 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 1.0);
			outCol = vec4(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0, 1.0);
			break;
		case 3: // Yellow-white
			size = 13;
			centerCol = vec4(255.0 / 255.0, 255.0 / 255.0, 102 / 255.0, 1.0);
			outCol = vec4(255.0 / 255.0, 255.0 / 255.0, 153 / 255.0, 1.0);
			break;
		case 4: // Yellow
			size = 11;
			centerCol = vec4(255.0 / 255.0, 255.0 / 255.0, 0 / 255.0, 1.0);
			outCol = vec4(255.0 / 255.0, 153 / 255.0, 0 / 255.0, 1.0);
			break;
		case 5:		  // Orange
			size = 8; // 255.0, 204, 102)
			centerCol = vec4(255.0 / 255.0, 153 / 255.0, 51 / 255.0, 1.0);
			outCol = vec4(255.0 / 255.0, 204 / 255.0, 102 / 255.0, 1.0);
			break;
		case 0: // Red-orange
			size = 6;
			centerCol = vec4(255.0 / 255.0, 51 / 255.0, 0 / 255.0, 1.0);
			outCol = vec4(204 / 255.0, 0 / 255.0, 0 / 255.0, 1.0);
			break;
		}
		star->size = vec3(size);
		costruisciStella(star, centerCol, outCol);
		crea_VAO_Vector(star);
		Stars.push_back(*star);
	}

	glViewport(0, 0, width, height);
}
void drawScene(void)
{

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	vec2 resolution = vec2((float)width, (float)height);
	glUniform1f(loctime, time);
	glUniform2f(locresolution, resolution.x, resolution.y);

	/*Matrice di modellazione delle stelle*/
	for (std::vector<Figura>::iterator it = Stars.begin(); it != Stars.end(); ++it)
	{
		it->Model = mat4(1.0);
		it->Model = translate(it->Model, it->position);
		it->Model = scale(it->Model, it->size);

		// Disegno nemico
		glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(it->Model));
		glBindVertexArray(it->VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, it->nv);
		// glDrawArrays(GL_TRIANGLE_STRIP, 0, it->nv);
		glBindVertexArray(0);
	}
	/*Matrice di modellazione dei nemici*/
	for (std::vector<Figura>::iterator it = Nemici.begin(); it != Nemici.end(); ++it)
	{
		it->Model = mat4(1.0);
		it->Model = translate(it->Model, it->position);
		it->Model = scale(it->Model, it->size);

		it->Model = rotate(it->Model, -it->direction, vec3(0.0, 0.0, 1.0));
		// Disegno nemico
		glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(it->Model));
		if (hitbox)
		{
			drawHitbox(&*it);
		}
		glBindVertexArray(it->VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, it->nv);
		glBindVertexArray(0);
	}

	/*Matrice di modellazione del player */ // inversed order by matrix multiplication rules
	player.Model = mat4(1.0);
	player.Model = translate(player.Model, player.position);
	player.Model = rotate(player.Model, player.direction, vec3(0.0f, 0.0f, 1.0f));
	/*Update Scala Matrice di modellazione del player */
	player.Model = scale(player.Model, player.size);
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	// Disegno player
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(player.Model));
	if (hitbox)
	{
		drawHitbox(&player);
	}
	else
	{
		glBindVertexArray(player.VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, player.nv - 18); // punto di inizio, + n° vertex
		glDrawArrays(GL_TRIANGLES, player.nv - 18, 18);
		glBindVertexArray(0);
	}

	/*Matrice di modellazione dei proiettili*/
	for (std::vector<Figura>::iterator it = Proiettili.begin(); it != Proiettili.end(); ++it)
	{
		it->Model = mat4(1.0);
		it->Model = translate(it->Model, it->position);
		it->Model = scale(it->Model, it->size);

		// Disegno Proettile
		glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(it->Model));
		glBindVertexArray(it->VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, it->nv);
		glBindVertexArray(0);
	}
	for (int i = 1; i <= player.live; i++)
	{
		life.Model = mat4(1.0);
		life.Model = translate(life.Model, vec3(width - i * 25.0, height - 20.0, 0.0));
		life.Model = scale(life.Model, life.size);
		// Disegno Proettile
		glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(life.Model));
		glBindVertexArray(life.VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, life.nv);
		glBindVertexArray(0);
	}

	// poligonizza
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSwapBuffers();
}

void myKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{

	case ' ':
		if (timer - cooldown > 0.05)
		{
			creaProiettile(player.position, vec3(cos(player.direction + PI / 2), sin(player.direction + PI / 2), 0.0), vec3(0.0));
			player.velocity -= player.acceleration * 0.25f;
			cooldown = timer;
		}
		break;
	case 'h':
		hitbox = !hitbox;
		break;

	case 'a':
		player.direction += 0.2;
		break;

	case 'd':
		player.direction -= 0.2;
		break;
	default:
		break;
	}
}
void aim(int button, int state, int x, int y)
{
	float viewX = ((float)x / glutGet(GLUT_WINDOW_WIDTH)) * width;
	float viewY = ((float)(glutGet(GLUT_WINDOW_HEIGHT) - y) / glutGet(GLUT_WINDOW_HEIGHT)) * height;
	vec3 dir = normalize(vec3(viewX, viewY, 0.0) - player.position);
	player.direction = -atan2(dir.x, dir.y);
	switch (button)
	{
	// Con il tasto sinistro premuto si attiva la modalit� di trackball
	case GLUT_LEFT_BUTTON:
		if (timer - cooldown > 0.05)
		{
			creaProiettile(player.position, vec3(cos(player.direction + PI / 2), sin(player.direction + PI / 2), 0.0), vec3(0.0));
			player.velocity -= player.acceleration * 0.25f;
			cooldown = timer;
		}
		break;
	}
}
void reset()
{
	player.live = 5;
	player.position = vec3(600.0, 200.0, 0.0);
	nEnemies = 0;
	Nemici.clear();
	Proiettili.clear();
	spawn = timer;
}

void myfunc(int i)
{
	float delta = timer;
	timer = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	delta = timer - delta;

	updatePlayer(delta);
	Figura *entity;
	for (std::vector<Figura>::iterator it = Proiettili.begin(); it != Proiettili.end(); ++it)
	{

		entity = &*it;
		updatePosition(entity, delta);
		// checkCollision
		for (std::vector<Figura>::iterator enemy = Nemici.begin(); enemy != Nemici.end(); ++enemy)
		{
			if (checkBoundingBox(entity, &*enemy) && entity->live > 0 && enemy->live > 0)
			{ //
				entity->live = 0;
				enemy->live--;
			}
		}
		if (abs(entity->position.x - width / 2) > width || abs(entity->position.y - height / 2) > height)
		{
			entity->live = 0;
		}
	}

	removeDead(Proiettili);
	for (std::vector<Figura>::iterator it = Nemici.begin(); it != Nemici.end(); ++it)
	{
		entity = &*it;
		updateEnemy(entity, delta);
		if (checkBoundingBox(&player, entity) && entity->live > 0)
		{ //
			entity->live = 0;
			player.live--;
		}
	}
	removeDead(Nemici);
	if (timer - spawn > 3.0 * pow(0.98, nEnemies) && timer - spawn > 1.0)
	{
		spawn = timer;
		nEnemies++;
		int life = nEnemies / 10;
		life += life <= 0 ? 1 : 0;
		creaNemico(life);
		if (nEnemies % 10 == 0)
		{
			player.live++;
		}
	}
	if (player.live <= 0)
	{
		reset();
	}
	glutPostRedisplay();
	glutTimerFunc(5, myfunc, 0);
}
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("space ship");
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(myKeyboard);
	glutMouseFunc(aim);
	glutPassiveMotionFunc(aim);
	glutTimerFunc(5, myfunc, 0);

	glewExperimental = GL_TRUE;
	glewInit();
	INIT_SHADER();
	INIT_VAO();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}
