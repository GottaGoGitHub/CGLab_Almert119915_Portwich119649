#version 150
#extension GL_OES_standard_derivatives : enable

in vec3 pass_Normal, pass_Position, pass_Camera_Position;
in mat4 pass_ViewMatrix, pass_ModelMatrix, pass_NormalMatrix;
in vec2 pass_TexCoord;

out vec4 out_Color;

uniform sampler2D TextureSampler;
uniform sampler2D NormalSampler;
uniform vec3 planet_color;
uniform vec3 light_color;
uniform vec3 light_position;
uniform float light_intensity;
uniform float ambient_intensity;

void main() {
  //get current diffuse color
  vec4 planetTexture = texture(TextureSampler, pass_TexCoord);
  // get current normal from normal map
  vec4 normalTexture = texture(NormalSampler, pass_TexCoord);

  //define scale of normals
  float normalScale = 5.0;

  // normalize normal texture from [-1,1] to [0,1]
  vec3 mapN = normalTexture.xyz * 2.0 - 1.0;

  // multiply with a scaling factor
  mapN.xy = normalScale * mapN.xy;

  vec3 normal = normalize(pass_Normal);
  vec3 vertex_pos = pass_Position;

  //calculate the difference in x and y direction on the vertex
  vec3 q0 = dFdx(vertex_pos.xyz);
  vec3 q1 = dFdy(vertex_pos.xyz);

  //calculates the difference in x and y direction on the uv plane
  vec2 st0 = dFdx(pass_TexCoord.st);
  vec2 st1 = dFdy(pass_TexCoord.st);

  // direct tangent calculation using the matrix equation explained here: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
  vec3 S = normalize(vec3(pass_NormalMatrix * vec4(normalize(q0 * st1.t - q1 * st0.t), 0.0)));
  // direct bitangent calculation
  vec3 T = normalize(vec3(pass_NormalMatrix * vec4(normalize(-q0 * st1.s + q1 * st0.s), 0.0)));
  // normal
  vec3 N = normalize(normal);

  mat3 tsn = mat3(S, T, N);
  normal = normalize(tsn * mapN);


  vec3 specular_color = vec3(1.0, 1.0, 1.0);

  float diffuse_reflection_factor = 0.9;
  float specular_reflection_factor = 0.4;
  int n = 10;

  vec3 camera_Position = pass_Camera_Position;

  vec3 transformed_light_position = (pass_ViewMatrix * vec4(light_position, 1.0)).xyz;

  vec3 light_Direction = normalize(transformed_light_position - pass_Position);
  vec3 view_Direction = normalize(-pass_Position);
  vec3 h = normalize(view_Direction + light_Direction);

  float diffuse_light_intensity = light_intensity * diffuse_reflection_factor * max(dot(normal, light_Direction), 0);
  float specular_light_intensity = light_intensity * specular_reflection_factor * pow(max(dot(h, normal), 0), n);

  vec3 ambient = ambient_intensity * light_color;
  vec3 diffuse = diffuse_light_intensity * light_color;
  vec3 specular =  specular_light_intensity * specular_color;

  //out_Color = vec4((ambient + diffuse) * planet_color + specular * light_color,1.0);
  out_Color = vec4((ambient + diffuse) * planetTexture.rgb + specular * light_color, 1.0);

}
