#pragma once
#include "Lib.h"

extern float raggio_sfera;
extern int selected_obj, selected_meshobj;
extern int width, height,w_up,h_up;
extern int last_mouse_pos_X;
extern int last_mouse_pos_Y;
extern bool moving_trackball;
extern float lastX, lastY;
extern bool firstMouse;
extern float Theta;
extern float Phi;
extern float timer;
extern mat4 View, Projection;
extern float xoffset, yoffset;
float scaleFactor;

vec3 get_ray_from_mouse(float mouse_x, float mouse_y)
{
	mouse_y = h_up - mouse_y;

	// Mappiamo le coordinate di viewport del mouse [0,width], [0,height] in coordinate normalizzate [-1,1]
	float x = (2.0f * mouse_x) / w_up - 1.0;
	float y = (2.0f * mouse_y) / h_up - 1.0;
	float z = 1.0f;

	// Nel clip space hanno coordinate: nota bene la terza coordinata uguale a -1
	vec3 ray_nds = vec3(x, y, z);
	vec4 ray_clip = vec4(x, y, -1.0, 1.0);

	// Le coordinate Nell' eye space si ottengono premoltiplicando per l'inversa della matrice Projection.
	vec4 ray_eye = inverse(Projection) * ray_clip;

	// Direzione del raggio
	ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	// Le coordinate nel world space: si ottengono premoltiplicando per l'inversa della matrice di Vista
	vec3 ray_wor = vec3(inverse(View) * ray_eye);
	ray_wor = normalize(ray_wor);
	cout<<"rey eye  :"<<ray_wor.x<<" y: "<<ray_wor.y<<" z: "<<ray_wor.z;
	return ray_wor;
}

/*
	Controlla se un raggio interseca una sfera. In caso negativo, restituisce false. Rigetta
	le intersezioni dietro l'origine del raggio, e pone  intersection_distance all'intersezione p i� vicina.
*/
bool ray_sphere(vec3 ray_origin_wor, vec3 ray_direction_wor, vec3 sphere_centre_wor, float sphere_radius, float *intersection_distance)
{
	// Calcoliamo O-C
	vec3 dist_sfera = ray_origin_wor - sphere_centre_wor;
	float b = dot(dist_sfera, ray_direction_wor);
	float cc = dot(dist_sfera, dist_sfera) - sphere_radius * sphere_radius;
	float delta = b * b - cc;
	// Il raggio non interseca la sfera
	if (delta < 0)
	{
		return false;
	}

	// Calcolo i valori di t per cui il raggio interseca la sfera e restituisco il valore dell'intersezione
	// pi� vicina all'osservatore (la t pi� piccola)
	if (delta > 0.0f)
	{
		// calcola le due intersezioni
		float t_a = -b + sqrt(delta);
		float t_b = -b - sqrt(delta);
		*intersection_distance = t_b;

		// Caso di intersezioni dietro l'osservatore
		if (t_a < 0.0)
		{
			if (t_b < 0)
			{
				return false;
			}
		}

		return true;
	}

	// Caso in cui il raggio � tangente alla sfera: un interesezione con molteplicit� doppia.
	if (delta == 0)
	{
		float t = -b + sqrt(delta);
		if (t < 0)
		{
			return false;
		}

		*intersection_distance = t;
		return true;
	}

	return false;
}

void mouse(int button, int state, int x, int y)
{
	cout<<"mouse pressed"<<endl;
	switch (button)
	{
	// Con il tasto sinistro premuto si attiva la modalit� di trackball
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			moving_trackball = true;
		}
		if (state == GLUT_UP)
		{
			moving_trackball = false;
		}

		last_mouse_pos_X = x;
		last_mouse_pos_Y = y;
		break;

	case GLUT_RIGHT_BUTTON:
		// Con il tasto destro si selezionano gli oggetti nella scena
		if (state == GLUT_DOWN)
		{
			cout << "rigth click";

			float xmouse = x;
			float ymouse = y;
			vec3 ray_wor = get_ray_from_mouse(xmouse, ymouse);

			selected_obj = -1;
			float closest_intersection = 100000000.0f;
			for (int i = 1; i < Scena.size(); i++)
			{
				float t_dist = 0.0f;

				// Interseco il raggio che esce dalla camera nella direzione del mouse con la sfera centrata nell'ancora di tutti gli oggetti
				// posizionati nel mondo per individuare se c'� intersezione con l'oggetto
				if (ray_sphere(ViewSetup.position, ray_wor, Scena[i]->ancora_world, Scena.at(i)->size, &t_dist))
				{
					//raysphere works only if the objects are present on the perimeter of the sphere, being inside or outside doesn't work correctly
					
					//scaledDistance = distance(ancora_world-ViewSetup.position)
					//need to try the reywor- normalize(ancora_world)*scaledDistance
					//with it, i calculate perpendiculare distance on the straigth line of mouse-eye ray 
					//after need to check if
					if (selected_obj == -1 || t_dist < closest_intersection)
					{
						selected_obj = i;
						
						cout<<"Oggetto selezionato"<<selected_obj<<" distace:  "<< t_dist<<endl;
						closest_intersection = t_dist;
					}
				}
			}
			if (selected_obj > -1)
			{
				
				printf("Oggetto selezionato %d -> %s \n", selected_obj, Scena[selected_obj]->nome.c_str());
			}
		}
		break;
	default:
		break;
	}
}

vec3 getTrackBallPoint(float x, float y)
{
	// Dalla posizione del mouse al punto proiettato sulla semisfera con centro l'origine e raggio 1
	float Delta, tmp;
	vec3 point;
	point.x = (2.0f * x - width) / width;
	point.y = (height - 2.0f * y) / height;

	// Cooordinata z del punto di coordinate (x,y,z) che si muove sulla sfera virtuale con centro (0,0,0) e raggio r=1
	tmp = pow(point.x, 2.0) - pow(point.y, 2.0);
	Delta = 1.0f - tmp;

	if (Delta > 0.0f)
	{
		point.z = sqrt(Delta);
	}
	else
	{
		point.z = 0;
	}

	return point;
}

void mouseActiveMotion(int x, int y)
{
	// Spostamento tramite trackball del vettore posizione Camera
	float pi = glm::pi<float>();
	float velocity = 180.0 / pi;

	if (!moving_trackball)
	{
		return;
	}

	vec3 corrente = normalize(getTrackBallPoint(x, y));
	vec3 precedente = normalize(getTrackBallPoint(last_mouse_pos_X, last_mouse_pos_Y));
	vec3 rotation_vec = cross(precedente, corrente);

	// Calcoliamo il coseno dell'angolo tra il segmento che collega l'origine e il click corrente e il segmento
	// che collega l'origine ed il click corrente
	float cosangolo = dot(precedente, corrente);

	// Il prodotto scalare tra due vettori normalizzati che hanno lunghezza 1,
	// pu� variare tra -1 ed 1.
	// Per problemi numerici dovuti al fatto che lavoriamo con i numeri finiti, per evitare che acos
	// dia risulato nan, nel caso in cui cosangolo risulti maggiore di 1 rimettiamolo ad 1
	if (cosangolo > 1)
	{
		cosangolo = 1.0;
	}

	// nel caso in cui cosangolo risulti minore di -1 rimettiamolo a -1
	if (cosangolo < -1)
	{
		cosangolo = -1.0;
	}

	// Calcoliamo l'angolo utilizzando l'arcocoseno
	float theta = (acos(cosangolo)) * velocity;

	// Costruiamo il quaternione a partire dall'angolo di rotazione e dalla direzione lungo cui ruotare
	quat rotation = angleAxis(theta, rotation_vec);

	// Trasformiamo il quaternione in matrice di rotazione
	mat4 rotation_matrix = toMat4(rotation);

	// Aggiorniamo direzione e posizione della telecamera
	vec4 dir = ViewSetup.direction;
	ViewSetup.direction = ViewSetup.position - ViewSetup.target;
	vec4 pos = ViewSetup.target + rotation_matrix * ViewSetup.direction;

	if (!checkCollision(pos))
	{
		ViewSetup.position = pos;
	}
	else
	{
		ViewSetup.direction = dir;
	}
	// Memorizzo l' ultima posizione del mouse
	last_mouse_pos_X = x;
	last_mouse_pos_Y = y;
}

void mouseActiveMotionTrack(int x, int y)
{
	return;
	float pi = glm::pi<float>();
	float velocity = 180.0 / pi;

	// Spostamento tramite trackball del vettore posizione Camera
	if (!moving_trackball)
	{
		return;
	}

	vec3 corrente = normalize(getTrackBallPoint(x, y));
	vec3 precedente = normalize(getTrackBallPoint(last_mouse_pos_X, last_mouse_pos_Y));
	vec3 rotation_vec = cross(precedente, corrente);

	// Per evitare le instabilit� dovute ad acos, si pu� stimare l'angolo di rotazione
	// nel seguente modo:
	float theta = length(precedente - corrente) * (180.0 / pi);

	// Costruiamo il quaternione a partire dall'angolo di rotazione e dalla direzione lungo cui ruotare
	quat rotation = angleAxis(theta, rotation_vec);

	// Trasformiamo il quaternione in matrice di rotazione
	mat4 rotation_matrix = toMat4(rotation);

	// Aggiorniamo direzione e posizione della telecamera
	ViewSetup.direction = ViewSetup.position - ViewSetup.target;
	ViewSetup.position = ViewSetup.target + rotation_matrix * ViewSetup.direction;

	// Memorizzo l' ultima posizione del mouse
	last_mouse_pos_X = x;
	last_mouse_pos_Y = y;
}

void printQ(vec4 quat)
{
	cout << quat.x << " " << quat.y << " " << quat.z << " " << quat.w << endl;
}
void printQ2(quat quat)
{
	cout << quat.x << " " << quat.y << " " << quat.z << " " << quat.w << endl;
}
void printV(vec3 quat)
{
	cout << quat.x << " " << quat.y << " " << quat.z << endl;
}

void my_passive_mouse(int xpos, int ypos)
{
	scaleFactor = 0.25f;

	// Calcola l'offset del mouse
	xoffset = ((float)(xpos - w_up / 2) / (float)w_up) * 1.0f;
	yoffset = ((float)(ypos - h_up / 2) / (float)h_up) * 1.0f;

	// Utilizza la funzione esponenziale per mappare l'offset lineare in un intervallo più ampio
	// float nonLinearXoffset = pow(2.0f, pow(abs(xoffset),3.0f)/3 * 6.0f);
	float nonLinearXoffset = (xoffset < 0 ? +1 : -1) * 45.0f*pow( abs(xoffset),3);


	// Trasforma i valori di offset in angoli
	xoffset = (xoffset < 0 ? +1 : -1) * nonLinearXoffset * scaleFactor;
	xoffset = nonLinearXoffset * scaleFactor;



	float nonLinearYoffset = (yoffset < 0 ? +1 : -1) * 45.0f*pow( abs(yoffset),3);
	// float nonLinearYoffset = pow(2.0f, pow(abs(yoffset),3.0f)/3 * 6.0f);
	yoffset =nonLinearYoffset * scaleFactor;
}
