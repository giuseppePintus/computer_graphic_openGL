#pragma once
#include "BaseMaterial.h"
#include "Strutture.h"
#include "Enum.h"

extern int selected_obj;
extern vector<Mesh*> Scena;
extern vector<Material> materials;
extern vector<Shader> shaders;
extern int selected_meshobj;

void main_menu_func(int option)
{
	switch (option)
	{
	case MenuOption::WIRE_FRAME:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case MenuOption::FACE_FILL:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	default:
		break;
	}
}

void material_menu_function(int option)
{
	/*
	if (selected_obj > -1)
	{Scena[selected_obj].material = (MaterialType)option;
		
	if (selected_meshobj > -1)
		for (int i = 0; i < ScenaObj[selected_meshobj].size(); i++)
			ScenaObj[selected_meshobj][i].sceltaVS = shaders[option].value;
	}*/
	if (selected_obj > -1)
		//Scena[selected_obj]->sceltaVS = shaders[option].value;
		Scena[selected_obj]->material = (MaterialType)option;
		cout<<"index "<<selected_obj<<endl;

}

void shader_menu_function(int option)
{
	cout<<"shader menu"<<endl;
	if (selected_obj > -1)
	{
		Scena[selected_obj]->sceltaVS = shaders[option].value;
	}
}

void buildOpenGLMenu()
{
	int materialSubMenu = glutCreateMenu(material_menu_function);
	glutAddMenuEntry(materials[MaterialType::EMERALD].name.c_str(), MaterialType::EMERALD);
	glutAddMenuEntry(materials[MaterialType::BRASS].name.c_str(), MaterialType::BRASS);
	glutAddMenuEntry(materials[MaterialType::SNOW_WHITE].name.c_str(), MaterialType::SNOW_WHITE);
	glutAddMenuEntry(materials[MaterialType::YELLOW].name.c_str(), MaterialType::YELLOW);
	glutAddMenuEntry(materials[MaterialType::ROSA].name.c_str(), MaterialType::ROSA);
	glutAddMenuEntry(materials[MaterialType::MARRONE].name.c_str(), MaterialType::MARRONE);
	glutAddMenuEntry(materials[MaterialType::NO_MATERIAL].name.c_str(), MaterialType::NO_MATERIAL);


	int shaderSubMenu = glutCreateMenu(shader_menu_function);
	glutAddMenuEntry(shaders[ShaderOption::NONE].name.c_str(), ShaderOption::NONE);
	glutAddMenuEntry(shaders[ShaderOption::GOURAD_SHADING].name.c_str(), ShaderOption::GOURAD_SHADING);
	glutAddMenuEntry(shaders[ShaderOption::PHONG_SHADING].name.c_str(), ShaderOption::PHONG_SHADING);
	glutAddMenuEntry(shaders[ShaderOption::ONDE_SHADING].name.c_str(), ShaderOption::ONDE_SHADING);
	glutAddMenuEntry(shaders[ShaderOption::BANDIERA_SHADING].name.c_str(), ShaderOption::BANDIERA_SHADING);

	// richiama main_menu_func() alla selezione di una voce menu
	glutCreateMenu(main_menu_func);
	glutAddMenuEntry("Wireframe", MenuOption::WIRE_FRAME);
	glutAddMenuEntry("Face fill", MenuOption::FACE_FILL);
	glutAddSubMenu("Material", materialSubMenu);
	glutAddSubMenu("Shader", shaderSubMenu);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}
