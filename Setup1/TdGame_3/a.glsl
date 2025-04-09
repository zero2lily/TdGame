#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv; 

//相当于一个int类型的整数，通过外界传过来的数字来访问对应的纹理单元
//uniform sampler2D sampler;

uniform sampler2D grassSampler;
uniform sampler2D landSampler;
uniform sampler2D noiseSampler;

uniform sampler2D zerotwoSampler;

uniform float width;
uniform float height;

void main()
{
    /*
        //根据噪声图片对两个纹理进行权重采样
        //vec4 grassColor = texture(grassSampler,uv);
        //vec4 landColor = texture(landSampler,uv);
        //vec4 noiseColor = texture(noiseSampler,uv);
        
        //float weight = noiseColor.r;

        //vec4 finalColor = grassColor * weight + landColor * (1 - weight);

        //gl有内置的函数 权重指的是第二个参数的权重
        //vec4 finalColor = mix(grassColor,landColor,weight);
        //FragColor = vec4(finalColor,1.0);
        
    */

    /*
            //贴图片
        	vec4 Color = texture(zerotwoSampler,uv);
        	FragColor = texture(zerotwoSampler,uv);
}
    */
