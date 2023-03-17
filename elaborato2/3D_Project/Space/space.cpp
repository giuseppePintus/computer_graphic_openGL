#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <cmath>
#endif //!_USE_MATH_DEFINES

#include <iostream>
// #include <windows.h>
#include "AssimpLoader.h"
#include "BaseMaterial.h"
#include "CameraHandler.h"
#include "Enum.h"
#include "Geometries.h"
#include "Lib.h"
#include "MenuHandler.h"
#include "MouseHandler.h"
#include "ShaderMaker.h"
#include "Strutture.h"
#include "TextureHandler.h"
#include "quaternion.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

glm::quat orient = angleAxis(float(0), vec3(0.0f, 0.0f, 1.0f));

int width = 1024;
int height = 600;
float timer = 0.0;
// Gestione input utente
int last_mouse_pos_Y;
int last_mouse_pos_X;
bool moving_trackball = false;
bool firstMouse = true;
float lastX = (float)width / 2;
float lastY = (float)height / 2;
float raggio_sfera = 50.0;

// spaceship movement
float xoffset = 0, yoffset = 0;
float shipSpeed = 0.0f;
float acceleration = 1000.0;
float maxVel = 1000.0f;
bool gas = false, brake = false, rotDX = false, rotSX = false;
vec3 velocity = vec3(0.0);
vec3 position = vec3(0.0);
mat4 shipOrientation = mat4(1.0);

// Gestione ancora
bool visualizzaAncora = false;

// Varibili per il reshape
int w_up = width;
int h_up = height;

// Inizializzazione dei modelli

vector<Mesh *> Scena;
static vector<MeshObj> Model3D;
vector<vector<MeshObj>> ScenaObj;
mat4 Projection, Model, View;
string stringa_asse;

// modifica modelli
int selected_obj = 0;
int selected_meshobj = 0;

// variabili per la comunicazione delle variabili uniformi con gli shader
static unsigned int programId, programId_text, programId1, MatrixProj, MatModel, MatView;
static unsigned int lsceltaFS, lsceltaVS, loc_texture, MatViewS, MatrixProjS;
static unsigned int loc_view_pos, MatModelR, MatViewR, MatrixProjR, loc_view_posR, loc_cubemapR;
unsigned int idTex, textureNull, textureSun, cubemapTexture, programIdr;
vector<unsigned int> planetTexture;
// Camera
string Operazione;
vec3 asse = vec3(0.0, 1.0, 0.0);
float cameraSpeed = 0.1;

// Locazione file
string meshDir = "Meshes/";
string imageDir = "Textures/";
string skyboxDir = "Skybox/";

/*
	 loads a cubemap texture from 6 individual texture faces
	 order:
	 +X (right)
	 -X (left)
	 +Y (top)
	 -Y (bottom)
	 +Z (front)
	 -Z (back)
*/
vector<string> faces{
	skyboxDir + "posx.png",
	skyboxDir + "negx.png",
	skyboxDir + "posy.png",
	skyboxDir + "negy.png",
	skyboxDir + "posz.png",
	skyboxDir + "negz.png"};
typedef struct
{
	vec3 pos;
	vector<Mesh *> planet;
	vector<vec3> planetPos;
	vector<unsigned int> textureId;
	vector<float> rotationSpeed;
	vector<float> angle;
	vector<vec3> rotatioAxis;

	vector<float> InternalRotationSpeed;
	vector<float> InternalAngle;
	vector<vec3> InternalRotationAxis;
	vector<float> scale;
} systemPlanet;

vector<systemPlanet *> universe = {};
// Creazione sfera
float Theta = -90.0f;
float Phi = 0.0f;

// Vettori di materiali e shader
vector<Material> materials = {};
vector<Shader> shaders = {};

// Luce
float angolo = 0.0;
point_light light;
LightShaderUniform light_unif = {};

//////////////////////////////////////////////////////////////////////////////////////////////
//									UTILITY FUNCTION
float linearRand(float min, float max)
{
	return min + (max - min) * static_cast<float>(std::rand()) / RAND_MAX;
}

vec3 sphericalRand(float r)
{
	// Generate random spherical coordinates (theta, phi)
	float theta = linearRand(0.0f, 2.0f * glm::pi<float>());
	float phi = linearRand(-glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);

	// Convert spherical coordinates to Cartesian coordinates
	return r * glm::vec3(
				   std::sin(phi) * std::cos(theta),
				   std::sin(phi) * std::sin(theta),
				   std::cos(phi));
}

//////////////////////////////////////////////////////////////////////////////////////////////

void translateFrom(int obj)
{
	if (Scena[obj]->BoxSphere)
	{
		vec4 center = Scena[obj]->ModelM * vec4(0.0, 0.0, 0.0, 1.0);
		vec3 dir = normalize(vec3(center) - position);
		position = vec3(center) - dir * (Scena[obj]->size + 50);
	}
}

void INIT_SHADER(void)
{
	GLenum ErrorCheckValue = glGetError();

	char *vertexShader = (char *)"vertexShader_C.glsl";
	char *fragmentShader = (char *)"fragmentShader_C.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

	vertexShader = (char *)"vertexShader_CubeMap.glsl";
	fragmentShader = (char *)"fragmentShader_CubeMap.glsl";
	programId1 = ShaderMaker::createProgram(vertexShader, fragmentShader);

	vertexShader = (char *)"vertexShader_riflessione.glsl";
	fragmentShader = (char *)"fragmentShader_riflessione.glsl";
	programIdr = ShaderMaker::createProgram(vertexShader, fragmentShader);
}

void INIT_Illuminazione()
{
	// Setup della luce
	light.position = {-7.0, 17.0, 12.0};
	light.color = {1.0, 1.0, 1.0};
	light.power = 2.f;

	// Setup dei materiali
	materials.resize(8);
	materials[MaterialType::RED_PLASTIC].name = "RED PLASTIC";
	materials[MaterialType::RED_PLASTIC].ambient = red_plastic_ambient;
	materials[MaterialType::RED_PLASTIC].diffuse = red_plastic_diffuse;
	materials[MaterialType::RED_PLASTIC].specular = red_plastic_specular;
	materials[MaterialType::RED_PLASTIC].shininess = red_plastic_shininess;

	materials[MaterialType::EMERALD].name = "EMERALD";
	materials[MaterialType::EMERALD].ambient = emerald_ambient;
	materials[MaterialType::EMERALD].diffuse = emerald_diffuse;
	materials[MaterialType::EMERALD].specular = emerald_specular;
	materials[MaterialType::EMERALD].shininess = emerald_shininess;

	materials[MaterialType::BRASS].name = "BRASS";
	materials[MaterialType::BRASS].ambient = brass_ambient;
	materials[MaterialType::BRASS].diffuse = brass_diffuse;
	materials[MaterialType::BRASS].specular = brass_specular;
	materials[MaterialType::BRASS].shininess = brass_shininess;

	materials[MaterialType::SNOW_WHITE].name = "WHITE";
	materials[MaterialType::SNOW_WHITE].ambient = snow_white_ambient;
	materials[MaterialType::SNOW_WHITE].diffuse = snow_white_diffuse;
	materials[MaterialType::SNOW_WHITE].specular = snow_white_specular;
	materials[MaterialType::SNOW_WHITE].shininess = snow_white_shininess;

	materials[MaterialType::YELLOW].name = "YELLOW";
	materials[MaterialType::YELLOW].ambient = yellow_ambient;
	materials[MaterialType::YELLOW].diffuse = yellow_diffuse;
	materials[MaterialType::YELLOW].specular = yellow_specular;
	materials[MaterialType::YELLOW].shininess = yellow_shininess;

	materials[MaterialType::ROSA].name = "ROSA";
	materials[MaterialType::ROSA].ambient = rosa_ambient;
	materials[MaterialType::ROSA].diffuse = rosa_diffuse;
	materials[MaterialType::ROSA].specular = rosa_specular;
	materials[MaterialType::ROSA].shininess = rosa_shininess;

	materials[MaterialType::MARRONE].name = "MARRONE";
	materials[MaterialType::MARRONE].ambient = marrone_ambient;
	materials[MaterialType::MARRONE].diffuse = marrone_diffuse;
	materials[MaterialType::MARRONE].specular = marrone_specular;
	materials[MaterialType::MARRONE].shininess = marrone_shininess;

	materials[MaterialType::NO_MATERIAL].name = "NO_MATERIAL";
	materials[MaterialType::NO_MATERIAL].ambient = glm::vec3(1, 1, 1);
	materials[MaterialType::NO_MATERIAL].diffuse = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].specular = glm::vec3(0, 0, 0);
	materials[MaterialType::NO_MATERIAL].shininess = 1.f;

	// Setup degli shader
	shaders.resize(5);
	shaders[ShaderOption::NONE].value = 0;
	shaders[ShaderOption::NONE].name = "NONE";

	shaders[ShaderOption::GOURAD_SHADING].value = 1;
	shaders[ShaderOption::GOURAD_SHADING].name = "GOURAD SHADING";

	shaders[ShaderOption::PHONG_SHADING].value = 2;
	shaders[ShaderOption::PHONG_SHADING].name = "BLINN-PHONG SHADING INTERPOLATIVO";

	shaders[ShaderOption::ONDE_SHADING].value = 3;
	shaders[ShaderOption::ONDE_SHADING].name = "PHONG SHADING";

	shaders[ShaderOption::BANDIERA_SHADING].value = 4;
	shaders[ShaderOption::BANDIERA_SHADING].name = "CARTOON SHADING";
}

void crea_VAO_Vector_MeshObj(MeshObj *mesh)
{
	glGenVertexArrays(1, &mesh->VAO);
	glBindVertexArray(mesh->VAO);

	// Genero, rendo attivo, riempio il VBO della geometria dei vertici
	glGenBuffers(1, &mesh->VBO_G);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_G);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertici.size() * sizeof(vec3), mesh->vertici.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	// Genero , rendo attivo, riempio il VBO dei colori
	glGenBuffers(1, &mesh->VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_C);
	glBufferData(GL_ARRAY_BUFFER, mesh->colori.size() * sizeof(vec4), mesh->colori.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(1);

	// Genero , rendo attivo, riempio il VBO delle normali
	glGenBuffers(1, &mesh->VBO_normali);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_normali);
	glBufferData(GL_ARRAY_BUFFER, mesh->normali.size() * sizeof(vec3), mesh->normali.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(2);

	// EBO di tipo indici
	glGenBuffers(1, &mesh->EBO_indici);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO_indici);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indici.size() * sizeof(GLuint), mesh->indici.data(), GL_STATIC_DRAW);
}

void crea_VAO_Vector(Mesh *mesh)
{
	glGenVertexArrays(1, &mesh->VAO);
	glBindVertexArray(mesh->VAO);

	// Genero, rendo attivo, riempio il VBO della geometria dei vertici
	glGenBuffers(1, &mesh->VBO_G);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_G);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertici.size() * sizeof(vec3), mesh->vertici.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	// Genero , rendo attivo, riempio il VBO dei colori
	glGenBuffers(1, &mesh->VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_C);
	glBufferData(GL_ARRAY_BUFFER, mesh->colori.size() * sizeof(vec4), mesh->colori.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(1);

	// Genero , rendo attivo, riempio il VBO delle normali
	glGenBuffers(1, &mesh->VBO_normali);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_normali);
	glBufferData(GL_ARRAY_BUFFER, mesh->normali.size() * sizeof(vec3), mesh->normali.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &mesh->VBO_coord_texture);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO_coord_texture);
	glBufferData(GL_ARRAY_BUFFER, mesh->texCoords.size() * sizeof(vec2), mesh->texCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(3);

	// EBO di tipo indici
	glGenBuffers(1, &mesh->EBO_indici);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO_indici);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indici.size() * sizeof(GLuint), mesh->indici.data(), GL_STATIC_DRAW);
}
void drawHitbox(Mesh *fig)
{
	Mesh *hitbox = new Mesh;

	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMax.y, fig->boundingBoxMax.z)); // 4
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMax.y, fig->boundingBoxMax.z)); // 1
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMin.y, fig->boundingBoxMax.z)); // 2
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMin.y, fig->boundingBoxMax.z)); // 3
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMax.y, fig->boundingBoxMax.z)); // 4
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMax.y, fig->boundingBoxMin.z)); // 8
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMin.y, fig->boundingBoxMin.z)); // 7
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMin.y, fig->boundingBoxMin.z)); // 6
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMax.y, fig->boundingBoxMin.z)); // 5
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMax.y, fig->boundingBoxMax.z)); // 1
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMin.y, fig->boundingBoxMax.z)); // 2
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMin.y, fig->boundingBoxMin.z)); // 6
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMin.y, fig->boundingBoxMin.z)); // 7
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMin.y, fig->boundingBoxMax.z)); // 3
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMax.y, fig->boundingBoxMax.z)); // 4
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMin.x, fig->boundingBoxMax.y, fig->boundingBoxMin.z)); // 8
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));
	hitbox->vertici.push_back(vec3(fig->boundingBoxMax.x, fig->boundingBoxMax.y, fig->boundingBoxMin.z)); // 5
	hitbox->colori.push_back(vec4(1.0, 0.0, 0.0, 1.0));

	crea_VAO_Vector(hitbox);

	glBindVertexArray(hitbox->VAO);
	glDrawArrays(GL_LINE_STRIP, 0, hitbox->vertici.size());
	glBindVertexArray(0);
	/*
	 */
}
void INIT_VAO(void)
{
	// Cubemap
	Mesh *Sky = new Mesh;

	// Variables for meshes and texture
	bool obj;
	int nmeshes;

	// texture sole
	string name = "sun.jpg";
	string path = imageDir + name;
	textureSun = loadTexture(path.c_str(), 0);

	for (int i = 0; i < 12; i++)
	{
		// load multiple planet texture
		name = to_string(i) + ".jpg";
		path = imageDir + name;
		planetTexture.push_back(loadTexture(path.c_str(), 0));
		cout << ((glIsTexture(planetTexture[i]) == GL_FALSE) ? "GL_FALSE" : "ok");
	}

	// Sky
	cubemapTexture = loadCubemap(faces, 0);
	crea_cubo(Sky);
	crea_VAO_Vector(Sky);
	Scena.push_back(Sky);

	vec4 colori = vec4(1., 0.5, 0.1, 1.0);
	for (int i = 0; i < 1; i++)
	{
		///////////////////////////////////////////////////
		//              system generation                //
		///////////////////////////////////////////////////
		systemPlanet *system = new systemPlanet;
		Mesh *Sole = new Mesh;

		crea_sfera(Sole, colori);
		crea_VAO_Vector(Sole);
		Sole->nome = "System" + to_string(i) + " star_";
		Sole->sceltaVS = 3;
		Sole->sceltaFS = 1;
		Sole->material = MaterialType::NO_MATERIAL;
		Sole->BoxSphere = true;
		Scena.push_back(Sole); // for collision
		system->planet.push_back(Sole);
		float scalarPos = 4000.0f + (float)(std::rand() % (20000) + 10000) * i;
		cout << "scalar pos:" << scalarPos;
		system->pos = sphericalRand(1.0) * scalarPos; // should be random
		// starting point of system, real position is in the offset
		system->planetPos.push_back(vec3(0.0, 0.0, 0.0));
		system->scale.push_back((float)(std::rand() % 500 + 500));
		Sole->size = system->scale[0];
		cout << "system " << i << "pos";
		printV(system->pos);
		system->textureId.push_back(textureSun);

		system->rotationSpeed.push_back((float)(i * 0.005f));
		system->rotatioAxis.push_back(sphericalRand(1.0));
		system->angle.push_back(0.0);

		system->InternalRotationSpeed.push_back((float)(0.5 + 0.2f * i));
		system->InternalRotationAxis.push_back(sphericalRand(1.0));
		system->InternalAngle.push_back(0.0);

		///////////////////////////////////////////////////
		//              planet generation                //
		///////////////////////////////////////////////////
		int npla = std::rand() % 6 + 5;
		for (int j = 1; j <= npla; j++)
		{
			Mesh *Planet = new Mesh;
			crea_sfera(Planet, colori);

			crea_VAO_Vector(Planet);
			Planet->BoxSphere = true;
			vec3 randomSpawn = sphericalRand(system->scale[0] + (float)(std::rand() % 800 + 200));
			system->scale.push_back((float)(std::rand() % 80 + 20));
			Planet->size = system->scale[j];
			system->planetPos.push_back(normalize(cross(randomSpawn, vec3(1.0, 0.0, 0.0))) * (system->scale[0] + (100.0f + system->scale[j] * j * 2.00f)));

			//////////////////////ROTATION AXIS,SPEED,CURRENT ANGLE//////////////////////////////////
			system->rotatioAxis.push_back(cross(system->InternalRotationAxis[0], normalize((system->pos - randomSpawn))));
			system->angle.push_back(0.0);
			system->rotationSpeed.push_back((float)(std::rand() % 50 + 20) * 0.001);

			system->InternalRotationSpeed.push_back((float)(std::rand() % 100 + 1) * 0.005);
			system->InternalRotationAxis.push_back(sphericalRand(1.0));
			system->InternalAngle.push_back(0.0);
			system->textureId.push_back(planetTexture[(unsigned int)(std::rand() % 11 + 1)]);
			// system->textureId.push_back(j + 1);
			////////////////////////////////////////////////////////////////////////////////////////
			Planet->nome = "System_" + to_string(i) + "_planet_" + to_string(j);
			Planet->sceltaVS = 3;
			Planet->sceltaFS = 3;
			Planet->material = MaterialType::NO_MATERIAL;
			system->planet.push_back(Planet);
			Scena.push_back(Planet); // for collision
		}
		universe.push_back(system);
	}

	name = "VulcanDKyrClass.obj";
	path = meshDir + name;
	obj = loadAssImp(path.c_str(), Model3D);
	nmeshes = Model3D.size();

	for (int i = 0; i < nmeshes; i++)
	{
		crea_VAO_Vector_MeshObj(&Model3D[i]);
		Model3D[i].nome = "spaceship";
		Model3D[i].sceltaVS = 3;
		Model3D[i].sceltaFS = 6;
	}
	ScenaObj.push_back(Model3D);
	Model3D.clear();
}

void INIT_CAMERA_PROJECTION(void)
{
	// Imposto la telecamera
	ViewSetup = {};
	ViewSetup.position = vec4(0.0, 20, 70.0, 0.0);
	ViewSetup.target = vec4(0.0, 0.0, 0.0, 0.0);
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	ViewSetup.upVector = vec4(0.0, 1.0, 0.0, 0.0);

	// Imposto la proiezione prospettica
	PerspectiveSetup = {};
	PerspectiveSetup.aspect = (GLfloat)width / (GLfloat)height;
	PerspectiveSetup.fovY = 45.0f;
	PerspectiveSetup.far_plane = 10000.0f;
	PerspectiveSetup.near_plane = 0.1f;
}

// int counter = 0;
void drawScene(void)
{
	float delta = timer;
	timer = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	delta = timer - delta;
	//////////////////PLAYER MOVEMENT////////////////////////
	// gas,brake,rotDX,rotSX;

	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	if (gas)
	{
		velocity += normalize(vec3(ViewSetup.direction)) * delta * acceleration;
		if (distance(velocity, vec3(0.0)) > maxVel)
		{
			velocity = normalize(velocity) * maxVel;
		}
	}
	else
	{
		if (brake)
		{
			velocity = velocity - velocity * 0.8f * delta;
		}
		else
		{
			velocity = velocity - velocity * 0.3f * delta;
		}
	}
	position = position + velocity * delta;

		float angle=0.0;
	mat4 rotationroll=mat4(1.0);
	if (rotSX)
	{
		angle=-(M_PI/4);
		rotationroll=glm::rotate(angle*delta,normalize(vec3(ViewSetup.direction)));
	}
	if (rotDX)
	{
		angle=(M_PI/4);
		rotationroll=rotationroll*glm::rotate(angle*delta,normalize(vec3(ViewSetup.direction)));
	}

	
	///////////////////camera rotation////////////////////////

	vec3 campos = glm::vec3(ViewSetup.position - ViewSetup.target);
	glm::mat4 rotationyaw = glm::rotate(xoffset * delta, vec3(ViewSetup.upVector));
	glm::mat4 rotationpitch = glm::rotate(yoffset * delta, cross(vec3(ViewSetup.upVector), campos));
	glm::mat4 trot = rotationroll*rotationyaw * rotationpitch;
	glm::mat4 shipOrientation2 =glm::rotate(angle,normalize(vec3(ViewSetup.direction)))
		* glm::rotate(xoffset / 2, vec3(ViewSetup.upVector)) 
		* glm::rotate(yoffset / 2, cross(vec3(ViewSetup.upVector), campos)) * shipOrientation;

	shipOrientation = trot * shipOrientation;
	ViewSetup.upVector = trot * ViewSetup.upVector;
	// ViewSetup.position = (trot * (ViewSetup.position - ViewSetup.target))+ViewSetup.target;
	ViewSetup.position = shipOrientation * vec4(0.0, 20, 70.0, 0.0);
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;

	View = lookAt(vec3(ViewSetup.position) + position, vec3(ViewSetup.target) + position, vec3(ViewSetup.upVector));
	////////////////////////////////////////////////////////////////////

	glUniformMatrix4fv(MatrixProj, 1, GL_FALSE, value_ptr(Projection));
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Disegno Sky box
	glDepthMask(GL_FALSE);
	glUseProgram(programId1);
	glUniform1i(glGetUniformLocation(programId1, "skybox"), 0);
	glUniformMatrix4fv(MatrixProjS, 1, GL_FALSE, value_ptr(Projection));
	glUniformMatrix4fv(MatViewS, 1, GL_FALSE, value_ptr(View));
	glBindVertexArray(Scena[0]->VAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, Scena[0]->indici.size() * sizeof(GLuint), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);

	// Costruisco la matrice di Vista che applicata ai vertici in coordinate del mondo li trasforma nel sistema di riferimento della camera.
	// Passo al Vertex Shader il puntatore alla matrice View, che sara' associata alla variabile Uniform mat4 Projection
	// all'interno del Vertex shader. Uso l'identificatio MatView
	glPointSize(10.0);

	// Cambio program per renderizzare tutto il resto della scena
	glUseProgram(programId);
	glUniformMatrix4fv(MatView, 1, GL_FALSE, value_ptr(View));

	// Passo allo shader il puntatore alla posizione della camera
	glUniform3f(loc_view_pos, ViewSetup.position.x, ViewSetup.position.y, ViewSetup.position.z);

	// draw systems
	for (int k = 0; k < universe.size(); k++)
	{
		light.position = universe[k]->pos;

		// Trasformazione delle coordinate dell'ancora dal sistema di riferimento dell'oggetto in sistema
		// di riferimento del mondo premoltiplicando per la matrice di Modellazione.
		for (int j = 0; j < universe[k]->planet.size(); j++)
		{
			light.power = 1.0f;
			// Passo allo shader il puntatore a  colore luce, posizione ed intensit�
			// glUniform3f(light_unif.light_position_pointer, light.position.x + 10 * cos(radians(angolo)), light.position.y, light.position.z + 10 * sin(radians(angolo)));

			glUniform3f(light_unif.light_position_pointer, light.position.x, light.position.y, light.position.z);
			glUniform3f(light_unif.light_color_pointer, light.color.r, light.color.g, light.color.b);
			glUniform1f(light_unif.light_power_pointer, light.power);
			// redefine modelM per rotazione pianeta
			universe[k]->planet[j]->ModelM = mat4(1.0);
			universe[k]->planet[j]->ModelM = translate(universe[k]->planet[j]->ModelM, universe[k]->pos); // translate of entire systems
			universe[k]->angle[j] = universe[k]->angle[j] + universe[k]->rotationSpeed[j] * delta;
			universe[k]->planet[j]->ModelM = rotate(universe[k]->planet[j]->ModelM, universe[k]->angle[j], universe[k]->rotatioAxis[j]);

			// vec3 Voffset = vec3(universe[k]->pos.x,universe[k]->pos.y,-universe[k]->pos.z);
			universe[k]->InternalAngle[j] = universe[k]->InternalAngle[j] + universe[k]->InternalRotationSpeed[j] * delta * 0.5f;
			universe[k]->planet[j]->ModelM = rotate(universe[k]->planet[j]->ModelM, universe[k]->InternalAngle[j], universe[k]->InternalRotationAxis[j]);
			universe[k]->planet[j]->ModelM = translate(universe[k]->planet[j]->ModelM, universe[k]->planetPos[j]); // offset from sun
			universe[k]->planet[j]->ModelM = scale(universe[k]->planet[j]->ModelM, vec3(universe[k]->scale[j]));

			universe[k]->planet[j]->ancora_world = universe[k]->planet[j]->ancora_obj;
			universe[k]->planet[j]->ancora_world = universe[k]->planet[j]->ModelM * universe[k]->planet[j]->ancora_world;
			// Passo al Vertex Shader il puntatore alla matrice Model dell'oggetto k-esimo della Scena,
			// che sara' associata alla variabile Uniform mat4 Projection
			// all'interno del Vertex shader. Uso l'identificatio MatModel
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(universe[k]->planet[j]->ModelM));

			glUniform1i(lsceltaVS, universe[k]->planet[j]->sceltaVS);
			glUniform1i(lsceltaFS, universe[k]->planet[j]->sceltaFS);
			// Passo allo shader il puntatore ai materiali
			glUniform3fv(light_unif.material_ambient, 1, value_ptr(materials[universe[k]->planet[j]->material].ambient));
			glUniform3fv(light_unif.material_diffuse, 1, value_ptr(materials[universe[k]->planet[j]->material].diffuse));
			glUniform3fv(light_unif.material_specular, 1, value_ptr(materials[universe[k]->planet[j]->material].specular));
			glUniform1f(light_unif.material_shininess, materials[universe[k]->planet[j]->material].shininess);
			Mesh *A = universe[k]->planet[j];
			vec4 A3 = (A->ModelM * vec4(A->boundingBoxMax, 1.0));
			vec4 A2 = (A->ModelM * vec4(A->boundingBoxMin, 1.0));
			// drawHitbox(universe[k]->planet[j]);

			glBindVertexArray(universe[k]->planet[j]->VAO);
			if (visualizzaAncora == true)
			{
				// Visualizzo l'ancora dell'oggetto
				int ind = universe[k]->planet[j]->indici.size() - 1;
				glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, BUFFER_OFFSET(ind * sizeof(GLuint)));
			}
			else
			{

				glUniform1i(loc_texture, 0);

				glActiveTexture(GL_TEXTURE0);

				glBindTexture(GL_TEXTURE_2D, universe[k]->textureId[j]);
				glDrawElements(GL_TRIANGLES, (universe[k]->planet[j]->indici.size() - 1) * sizeof(GLuint), GL_UNSIGNED_INT, 0);
			}

			glBindVertexArray(0);
		}
	}
	// check collision
	int collision = checkCollision(vec4(position, 0.0));
	if (collision > -1)
	{
		cout << "collision item: " << collision << endl;
		translateFrom(collision);
	}

	// Visualizzo gli oggetti di tipo Mesh Obj caricati dall'esterno:
	// la j-esima Mesh e' costituita da ScenaObj[j].size() mesh.
	for (int j = 0; j < ScenaObj.size(); j++)
	{
		for (int k = 0; k < ScenaObj[j].size(); k++)
		{
			// Passo al Vertex Shader il puntatore alla matrice Model dell'oggetto k-esimo della Scena, che sara' associata alla variabile Uniform mat4 Projection
			// all'interno del Vertex shader. Uso l'identificatio MatModel
			ScenaObj[j][k].ModelM = mat4(1.0);
			ScenaObj[j][k].ModelM = translate(ScenaObj[j][k].ModelM, position + vec3(ViewSetup.target));
			//+
			//(distance(vec3(ViewSetup.target),vec3(ViewSetup.position))/10.0f)*vec3(-ViewSetup.upVector));
			ScenaObj[j][k].ModelM = ScenaObj[j][k].ModelM * shipOrientation2;
			ScenaObj[j][k].ModelM = scale(ScenaObj[j][k].ModelM, vec3(2.0));

			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(ScenaObj[j][k].ModelM));
			glUniform1i(lsceltaVS, ScenaObj[j][k].sceltaVS);
			glUniform1i(lsceltaFS, ScenaObj[j][k].sceltaFS);
			// Passo allo shader il puntatore ai materiali
			glUniform3fv(light_unif.material_ambient, 1, value_ptr(ScenaObj[j][k].materiale.ambient));
			glUniform3fv(light_unif.material_diffuse, 1, value_ptr(ScenaObj[j][k].materiale.diffuse));
			glUniform3fv(light_unif.material_specular, 1, value_ptr(ScenaObj[j][k].materiale.specular));
			glUniform1f(light_unif.material_shininess, ScenaObj[j][k].materiale.shininess);
			glBindVertexArray(ScenaObj[j][k].VAO);
			glDrawElements(GL_TRIANGLES, (ScenaObj[j][k].indici.size()) * sizeof(GLuint), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
	glutSwapBuffers();
}

void update(int a)
{
	glutPostRedisplay();
	glutTimerFunc(10, update, 0);
}

// Window resizing
void resize(int w, int h)
{
	// Imposto la matrice di proiezione per la scena da diegnare
	Projection = perspective(radians(PerspectiveSetup.fovY), PerspectiveSetup.aspect, PerspectiveSetup.near_plane, PerspectiveSetup.far_plane);

	// Rapporto larghezza altezza di tutto cio' che e' nel mondo
	float AspectRatio_mondo = (float)(width) / (float)(height);

	// Se l'aspect ratio del mondo e' diversa da quella della finestra
	if (AspectRatio_mondo > w / h)
	{
		glViewport(0, 0, w, w / AspectRatio_mondo);
		w_up = (float)w;
		h_up = w / AspectRatio_mondo;
	}
	else
	{
		glViewport(0, 0, h * AspectRatio_mondo, h);
		w_up = h * AspectRatio_mondo;
		h_up = (float)h;
	}
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	// comment this for a freeglut bug compatibility
	// glutInitContextVersion(4, 0);
	// glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// Inizializzo finestra per il rendering della scena 3d
	// con tutti i suoi eventi le sue inizializzazioni e le sue impostazioni
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Scena 3D");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);

	// gestione input utente
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboardPressedEvent);
	glutKeyboardUpFunc(keyboardReleasedEvent);
	// glutMotionFunc(mouseActiveMotion);
	// glutMotionFunc(my_passive_mouse);

	// Uncomment here if needed
	glutPassiveMotionFunc(my_passive_mouse);

	glutTimerFunc(10, update, 0);
	glewExperimental = GL_TRUE;
	glewInit();

	// Inizializzazione setup illuminazione, materiali
	INIT_Illuminazione();

	// Inizializzazione setup Shader
	INIT_SHADER();

	// Inizializzazione VAO
	INIT_VAO();

	// Inizializzazione setup telecamera
	INIT_CAMERA_PROJECTION();

	// Menu collegato al tasto centrale
	buildOpenGLMenu();

	// Abilita l'uso del Buffer di Profondit� per la gestione dell'eliminazione dlele superifici nascoste
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Projection (in vertex shader).
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice Projection al Vertex Shader
	MatrixProj = glGetUniformLocation(programId, "Projection");

	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Model (in vertex shader)
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice Model al Vertex Shader
	MatModel = glGetUniformLocation(programId, "Model");

	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 View (in vertex shader)
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice View al Vertex Shader
	MatView = glGetUniformLocation(programId, "View");

	lsceltaVS = glGetUniformLocation(programId, "sceltaVS");
	lsceltaFS = glGetUniformLocation(programId, "sceltaFS");
	loc_view_pos = glGetUniformLocation(programId, "ViewPos");
	loc_texture = glGetUniformLocation(programId, "id_tex");
	// Location delle variabili uniformi per la gestione della luce
	light_unif.light_position_pointer = glGetUniformLocation(programId, "light.position");
	light_unif.light_color_pointer = glGetUniformLocation(programId, "light.color");
	light_unif.light_power_pointer = glGetUniformLocation(programId, "light.power");

	// Location delle variabili uniformi per la gestione dei materiali
	light_unif.material_ambient = glGetUniformLocation(programId, "material.ambient");
	light_unif.material_diffuse = glGetUniformLocation(programId, "material.diffuse");
	light_unif.material_specular = glGetUniformLocation(programId, "material.specular");
	light_unif.material_shininess = glGetUniformLocation(programId, "material.shininess");

	// location variabili uniformi per lo shader della gestione della cubemap
	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Model
	// (in vertex shader)
	// Questo identificativo sar� poi utilizzato per il trasferimento della matrice Model
	// al Vertex Shader
	MatrixProjS = glGetUniformLocation(programId1, "Projection");
	MatViewS = glGetUniformLocation(programId1, "View");

	// Chiedo che mi venga restituito l'identificativo della variabile uniform mat4 Model
	// (in vertex shader)
	// QUesto identificativo sar� poi utilizzato per il trasferimento della matrice Model
	// al Vertex Shader
	MatModelR = glGetUniformLocation(programIdr, "Model");
	MatViewR = glGetUniformLocation(programIdr, "View");
	MatrixProjR = glGetUniformLocation(programIdr, "Projection");
	loc_view_posR = glGetUniformLocation(programIdr, "ViewPos");
	loc_cubemapR = glGetUniformLocation(programIdr, "cubemap");

	// PlaySound(TEXT("sigla.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);

	glutMainLoop();
}
