#version 420 core

// Interpolated values from the vertex shaders

in vec4 ourColor;

uniform int type;
uniform float time;
out vec4 FragColor;

vec3 color;
uniform vec2 resolution;
void main(){
	vec2 ndc= vec2((gl_FragCoord.x/resolution.x-0.5)*2,(gl_FragCoord.y/resolution.y-0.5)*2);
	//conversione del frammento alle coordinate del device normalizzate
	if(ndc.y>-1.0 && ndc.y<1.0){


	color=vec3(0,abs(sin(ndc.y*10.5+time)),abs(sin(ndc.y*time)));
	FragColor=vec4(color,1.0);
	}
	else if(distance(ndc.xy, vec2(0,0))<0.2)
	{
		FragColor=vec4(0.0,0.0,abs(sin(ndc.y*10.5+time)) ,1.0);
	}
	else{
//Viene assegnato ad ogni frammento il colore interpolato
	}
	FragColor=ourColor;

}
