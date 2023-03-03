#pragma once
#include "Lib.h"
#include "Strutture.h"
#include "Enum.h"

extern int selected_obj;
extern float cameraSpeed;
extern bool visualizzaAncora;
extern vec3 asse;
extern string Operazione;
extern string stringa_asse;
extern vector<Mesh*> Scena;
extern vector<vector<MeshObj>> ScenaObj;
extern float shipSpeed;
extern bool gas, brake, rotDX, rotSX;

bool checkBoundingBox(Mesh *A, vec3 B)
{

	float Amaxx = (A->ModelM * vec4(A->boundingBoxMax,1.0)).x, Aminx = (A->ModelM * vec4(A->boundingBoxMin, 1.0)).x;
	float Amaxy = (A->ModelM * vec4(A->boundingBoxMax, 1.0)).y, Aminy = (A->ModelM * vec4(A->boundingBoxMin, 1.0)).y;
	float Amaxz = (A->ModelM * vec4(A->boundingBoxMax, 1.0)).z, Aminz = (A->ModelM * vec4(A->boundingBoxMin, 1.0)).z;
	/*
	
	vec4 A3  = (A->ModelM * vec4(A->boundingBoxMax,1.0));
	vec4 A2 = (A->ModelM * vec4(A->boundingBoxMin, 1.0));
	cout<<A->nome<<endl;
	cout<<"x: "<<A3.x<<"  "<<B.x<<"  "<<A2.x<<endl;
	cout<<"y: "<<A3.y<<"  "<<B.y<<"  "<<A2.y<<endl;
	cout<<"z: "<<A3.z<<"  "<<B.z<<"  "<<A2.z<<endl;
	*/
if(A->BoxSphere){
	vec4 center = A->ModelM *vec4(0.0,0.0,0.0,1.0);
	float lenght= distance(vec3(center),B);	
	return !(lenght>(A->size));
	
}
	
/*
	for (std::vector<vec3>::iterator it = A.corner.begin(); it != A.corner.end(); it++)
	{
		vec4 i = A->ModelM * vec4(*it, 1.0);
		if (Amaxx < i.x)
		{
			Amaxx = i.x;
		}
		if (Amaxy < i.y)
		{
			Amaxy = i.y;
		}
		if (Amaxz < i.z)
		{
			Amaxz = i.z;
		}
		if (Aminx > i.x)
		{
			Aminx = i.x;
		}
		if (Aminy > i.y)
		{
			Aminy = i.y;
		}
		if (Aminz > i.z)
		{
			Aminz = i.z;
		}
	}*/
	// collisione asse x
	bool cornerx = Aminx < B.x && Amaxx > B.x;

	// collisione asse y
	bool cornery = Aminy < B.y && Amaxy > B.y;
	// asse z
	bool cornerz = Aminz < B.z && Amaxz > B.z;
	// se ho una collisione fermo esco
	if (cornerx && cornery && cornerz)
	{
		return true;
	}
	return false;
}

int checkCollision(vec4 pos)
{
	for (int i= 1; i < Scena.size(); i++)
	{
		Mesh* entity = Scena.at(i);
		// se ho una collisione fermo esco
		if (checkBoundingBox(entity,pos))
		{
			return i;
		}
	}
	for (int j = 1; j < ScenaObj.size(); j++)
	{
		for (int k = 0; k < ScenaObj[j].size(); k++)
		{
			vec4 boundGlobT = ScenaObj[j][k].ModelM * vec4(ScenaObj[j][k].boundingBoxMax, 1.0);
			vec4 boundGlobB = ScenaObj[j][k].ModelM * vec4(ScenaObj[j][k].boundingBoxMin, 1.0);

			// collisione asse x
			bool collisionX = boundGlobB.x <= pos.x &&
							  boundGlobT.x >= pos.x;

			// collisione asse y
			bool collisionY = boundGlobB.y <= pos.y &&
							  boundGlobT.y >= pos.y;
			// collisione asse z
			bool collisionZ = boundGlobB.z <= pos.z &&
							  boundGlobT.z >= pos.z;

			// se ho una collisione fermo esco
			if (collisionX && collisionY && collisionZ)
			{
				return j;
			}
		}
	}

	return -1;
}

void modifyModelMatrix(vec3 translation_vector, vec3 rotation_vector, GLfloat angle, GLfloat scale_factor)
{
	// ricordare che mat4(1) costruisce una matrice identità di ordine 4
	mat4 traslation = translate(mat4(1), translation_vector);
	mat4 scales = scale(mat4(1), vec3(scale_factor, scale_factor, scale_factor));
	mat4 rotation = rotate(mat4(1), angle, rotation_vector);

	// Modifica la matrice di Modellazione dell'oggetto della scena selezionato postmolitplicando per le matrici scale*rotarion*traslation
	Scena[selected_obj]->ModelM = Scena[selected_obj]->ModelM * scales * rotation * traslation;

	glutPostRedisplay();
}

void moveZoomInCamera()
{
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec4 pos = ViewSetup.position + ViewSetup.direction * cameraSpeed;
	if (checkCollision(pos)==-1)
	{
		ViewSetup.position = pos;
	}
}

void moveZoomOutCamera()
{
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec4 pos = ViewSetup.position - ViewSetup.direction * cameraSpeed;
	if (checkCollision(pos)==-1)
	{
		ViewSetup.position = pos;
	}
}

void moveCameraLeft()
{
	// Calcolo la direzione perpendicolare alla direzione della camera e l'alto della camera
	// e muovo la camera a sinistra lungo questa direzione
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec3 direzione_scorrimento = cross(vec3(ViewSetup.direction), vec3(ViewSetup.upVector));
	vec4 pos = ViewSetup.position - vec4((direzione_scorrimento), .0) * cameraSpeed;
	if (checkCollision(pos)==-1)
	{
		ViewSetup.position = pos;
		ViewSetup.target -= vec4((direzione_scorrimento), .0) * cameraSpeed;
	}
}

void moveCameraRight()
{
	// Calcolo la direzione perpendicolare alla direzione della camera e l'alto della camera
	// e muovo la camera a destra lungo questa direzione
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec3 direzione_scorrimento = cross(vec3(ViewSetup.direction), vec3(ViewSetup.upVector)) * cameraSpeed;
	vec4 pos = ViewSetup.position + vec4(direzione_scorrimento, 0);
	if (checkCollision(pos)==-1)
	{
		ViewSetup.position = pos;
		ViewSetup.target += vec4(direzione_scorrimento, 0);
	}
}
void moveCameraBackward()
{
	// Calcolo la direzione perpendicolare alla direzione della camera e l'alto della camera
	// e muovo la camera a sinistra lungo questa direzione
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec3 direzione_scorrimento = vec3(ViewSetup.direction);
	vec4 pos = ViewSetup.position - vec4((direzione_scorrimento), .0) * cameraSpeed;
	if (checkCollision(pos)==-1)
	{
		ViewSetup.position = pos;
		// ViewSetup.target -= vec4((direzione_scorrimento), .0) * cameraSpeed;
		ViewSetup.target -= vec4(direzione_scorrimento, .0);
	}
}
void moveCameraForward()
{
	// Calcolo la direzione perpendicolare alla direzione della camera e l'alto della camera
	// e muovo la camera a sinistra lungo questa direzione
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec3 direzione_scorrimento = vec3(ViewSetup.direction);
	vec4 pos = ViewSetup.position + vec4((direzione_scorrimento), .0) * cameraSpeed;
	if (checkCollision(pos)==-1)
	{
		ViewSetup.position = pos;
		// ViewSetup.target -= vec4((direzione_scorrimento), .0) * cameraSpeed;
		ViewSetup.target += vec4(direzione_scorrimento, .0);
	}
}
void moveCameraUp()
{
	ViewSetup.direction = ViewSetup.target - ViewSetup.position; // Direzione lungo cui si sposta la telecamera in coordinate del mondo
	vec3 direzione_scorrimento = normalize(cross(vec3(ViewSetup.direction), vec3(ViewSetup.upVector)));
	vec3 upDirection = cross(vec3(ViewSetup.direction), direzione_scorrimento) * cameraSpeed;
	vec4 pos = ViewSetup.position - vec4(upDirection, 0.0);

	if (checkCollision(pos)==-1)
	{
		ViewSetup.target -= vec4(upDirection, 0.0);
		ViewSetup.position = pos;
	}
}

void moveCameraDown()
{
	ViewSetup.direction = ViewSetup.target - ViewSetup.position;
	vec3 direzione_scorrimento = normalize(cross(vec3(ViewSetup.direction), vec3(ViewSetup.upVector)));
	vec3 upDirection = cross(vec3(ViewSetup.direction), direzione_scorrimento) * cameraSpeed;
	vec4 pos = ViewSetup.position + vec4(upDirection, 0.0);

	if (checkCollision(pos)==-1)
	{
		ViewSetup.target += vec4(upDirection, 0.0);
		ViewSetup.position = pos;
	}
}

// Gestione eventi tastiera per il movimento della telecamera
void keyboardPressedEvent(unsigned char key, int x, int y)
{
	char *intStr;
	string str;
	switch (key)
	{
	// Ortogonal movemente
	case 'w': // gas
		//moveCameraForward();
		gas = true;
		break;
	case 'a':
		// moveCameraLeft();
		rotSX = true;
		break;
	case 's':
		brake = true;
		gas=false;
		// moveCameraBackward();
		break;
	case 'd':
		rotDX = true;
		break;
	// zoom
	case 'q':
		moveZoomInCamera();
		break;
	case 'e':
		moveZoomOutCamera();
		break;
	// Up and Down
	case 'l':
		moveCameraUp();
		break;
	case 'k':

		break;
	case 'j':
		moveCameraDown();
		break;

	// Visualizzazione ancora
	case 'v':
		visualizzaAncora = true;
		break;

	// Si entra in modalit� di operazione traslazione
	case 'i':
		OperationMode = TRASLATING;
		Operazione = "TRASLAZIONE";
		break;
	// Si entra in modalit� di operazione rotazione
	case 'o':
		OperationMode = ROTATING;
		Operazione = "ROTAZIONE";
		break;
	// Si entra in modalit� di operazione scalatura
	case 'p':
		OperationMode = SCALING;
		Operazione = "SCALATURA";
		break;

	// Exit GUI
	case 27:
		glutLeaveMainLoop();
		break;

	// Seleziona l'asse X come asse lungo cui effettuare l'operazione selezionata (tra traslazione, rotazione, scalatura)
	case 'x':
		WorkingAxis = X;
		stringa_asse = " Asse X";
		break;
	// Seleziona l'asse Y come asse lungo cui effettuare l'operazione selezionata (tra traslazione, rotazione, scalatura)
	case 'y':
		WorkingAxis = Y;
		stringa_asse = " Asse Y";
		break;
	// Seleziona l'asse Z come asse lungo cui effettuare l'operazione selezionata (tra traslazione, rotazione, scalatura)
	case 'z':
		WorkingAxis = Z;
		stringa_asse = " Asse Z";
		break;
	default:
		break;
	}

	// Selezione dell'asse per le trasformazioni
	switch (WorkingAxis)
	{
	case X:
		asse = vec3(1.0, 0.0, 0.0);
		break;
	case Y:
		asse = vec3(0.0, 1.0, 0.0);
		break;
	case Z:
		asse = vec3(0.0, 0.0, 1.0);
		break;
	default:
		break;
	}

	// I tasti + e -  aggiornano lo spostamento a destra o a sinistra,
	// la rotazione in segno antiorario o in senso orario, la scalatura come
	// amplificazione o diminuizione delle dimensioni
	float amount = .01;
	if (key == 'm')
	{
		amount *= 1;
	}

	if (key == 'n')
	{
		amount *= -1;
	}

	switch (OperationMode)
	{

	// la funzione modifyModelMatrix() definisce la matrice di modellazione che si vuole postmoltiplicare
	//  alla matrice di modellazione dell'oggetto selezionato, per poterlo traslare, ruotare scalare.
	case TRASLATING:
		// si passa angle 0 e scale factor =1,
		modifyModelMatrix(asse * amount, asse, 0.0f, 1.0f);
		break;
	case ROTATING:
		// Si mette a zero il vettore di traslazione (vec3(0) e ad 1 il fattore di scale
		modifyModelMatrix(glm::vec3(0), asse, amount * 2.0f, 1.0f);
		break;
	case SCALING:
		// Si mette a zero il vettore di traslazione (vec3(0), angolo di rotazione a 0 e ad 1 il fattore di scala 1+amount.
		modifyModelMatrix(glm::vec3(0), asse, 0.0f, 1.0f + amount);
		break;
	}
}
// Keyboard movement
void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	{
		switch (key)
		{
		case 'v':
			visualizzaAncora = false;
			break;
		case 'w': //gas
			gas = false;
			break;
		case 'a':
			rotSX = false;
			break;
		case 's':
			brake = false;
			break;
		case 'd':
			rotDX = false;
			break;
		default:
			break;
		}
	}
}
