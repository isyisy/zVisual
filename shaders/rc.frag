#version 330 core

out vec4 vFragColor;
smooth in vec3 vUV;

uniform vec3 cameraPosition;

uniform vec3 volumeSizeRatio; 

uniform float mingray;
uniform float maxgray;
uniform sampler3D texMap;

const int MAX_SAMPLE_NUMBERS = 300;

float stepSize = 1.732/float(MAX_SAMPLE_NUMBERS);

//transfer fuction knots
vec4 samplesheet0 = vec4(0,0,0,0);
vec4 samplesheet1 = vec4(1.0,0.64,0.33,0.01);
vec4 samplesheet2 = vec4(1.0,0.35,0.14,0.13);
vec4 samplesheet3 = vec4(1,1,1,0.8);
float point0=0.0;
float point1=0.3;
float point2=0.73;
float point3=1.0;

void main()
{
	vec3 c_out = vec3(0,0,0);
	float a_out = 0.0;
	
	vec3 cameraLocalPos = vec3(cameraPosition.x/volumeSizeRatio.x,cameraPosition.y/volumeSizeRatio.y,cameraPosition.z/volumeSizeRatio.z);
	vec3 cameraTexPos = cameraLocalPos/2.0+0.5;
	vec3 dir = normalize(vUV-cameraTexPos);
	
	vec3 pos = vUV;
	for(int i=0;i<MAX_SAMPLE_NUMBERS;i++)
	{
		vec3 color = vec3(0,0,0);
		float alpha = 0.0;
		pos += dir*stepSize;
		
		if(pos.x<0.0||pos.x>1.0||pos.y<0.0||pos.y>1.0||pos.z<0.0||pos.z>1.0)
			break;
		
		float sample = (texture(texMap,pos).r-mingray)/(maxgray-mingray);

		if(sample<point1)
		{
			vec4 comp = mix(samplesheet0,samplesheet1,(sample-point0)/(point1-point0));
			color = comp.xyz;
			alpha = comp.w;
		}else if(sample<point2)
		{
			vec4 comp = mix(samplesheet1,samplesheet2,(sample-point1)/(point2-point1));
			color = comp.xyz;
			alpha = comp.w;
		}else if(sample<point3)
		{
			vec4 comp = mix(samplesheet2, samplesheet3, (sample-point2)/(point3-point2));
			color = comp.xyz;
			alpha = comp.w;
		}else{
			color = samplesheet3.xyz;
			alpha = samplesheet3.w;
		}
		c_out += (1.0-a_out)*alpha*color;
		a_out += (1.0-a_out)*alpha;
		if(a_out>1.0)
			break;
	} 
	vFragColor = vec4(c_out,a_out);
}