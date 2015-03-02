// StarDisplay Shader 
//
// Hanno Hildenbrandt 2013
//


void GLSL_VERSION
{
  #version 400 core
};


// Transformation uniform block
void Matrices
{
  layout (std140) uniform Matrices
  {
    mat4 ModelViewProjection;
    mat4 ModelView;
    mat4 Projection;
    mat4 Ortho;
  };
};


shader [vertex] Text
{
  layout (location = 0) in vec2 TexCoord;
  layout (location = 1) in vec2 Vertex;
  layout (location = 2) in vec4 Color;

  out vec2 vTexCoord;
  out vec2 vVertex;
  out vec4 vColor;

  void main()
  {
    vColor = Color;
    vTexCoord = TexCoord;
    vVertex = Vertex;
  }
};

shader [geometry] Text
{
  layout(lines) in;
  layout(triangle_strip, max_vertices=4) out;

  in vec2 vTexCoord[2];
  in vec2 vVertex[2];
  in vec4 vColor[2];

  smooth out vec2 gTexCoord;
  flat out vec4 gColor;

  void main()
  {
    vec2 t0 = vTexCoord[0];
    vec2 t1 = vTexCoord[1];
    vec2 v0 = vVertex[0];
    vec2 v1 = vVertex[1];

    gColor = vColor[1];
    gTexCoord = vec2(t0.x, t1.y);
    gl_Position = Ortho * vec4(v0.x, v1.y, 0, 1);
    EmitVertex();

    gTexCoord = vec2(t0.x, t0.y);
    gl_Position = Ortho * vec4(v0.x, v0.y, 0, 1);
    EmitVertex();

    gTexCoord = vec2(t1.x, t1.y);
    gl_Position = Ortho * vec4(v1.x, v1.y, 0, 1);
    EmitVertex();

    gTexCoord = vec2(t1.x, t0.y);
    gl_Position = Ortho * vec4(v1.x, v0.y, 0, 1);
    EmitVertex();

    EndPrimitive();
  }
};

shader [fragment] Text
{
  #extension GL_ARB_texture_rectangle : enable
  
  uniform sampler2DRect FontTexture;

  smooth in vec2 gTexCoord;
  flat in vec4 gColor;

  layout (location = 0) out vec4 FragColor;

  void main()
  {
    vec4 color = gColor;
    color.a = texture2DRect(FontTexture, gTexCoord).r;
    FragColor = color;
  }
};

program Text
{
  vertex: *;
  geometry: Matrices *;
  fragment: *;
};


shader[vertex] NoLit
{
  layout (location = 0) in vec4 Vertex;
  layout (location = 1) in vec4 Color;
  layout (location = 2) in float ColorTexCoord;

  smooth out vec4 vColor;
  smooth out float vColorTexCoord;

  void main(void) 
  {            
    vColor = Color;
    vColorTexCoord = ColorTexCoord;
    gl_Position = ModelViewProjection * Vertex;
  }

};

shader[fragment] NoLit
{
  uniform sampler1D ColorTex; 

  smooth in vec4 vColor;
  smooth in float vColorTexCoord;
  
  out vec4 FragColor;

  void main(void) 
  {
    if (vColorTexCoord <= -90.0) discard;
    FragColor = (vColorTexCoord <= 90.0) ? texture(ColorTex, vColorTexCoord) : vColor;
  }
};

program NoLit
{
    vertex: Matrices *;
    fragment: *;
};


shader[vertex] NoLit2D
{
  layout (location = 0) in vec4 Vertex;
  layout (location = 1) in vec4 Color;
  layout (location = 2) in float ColorTexCoord;

  smooth out vec4 vColor;
  smooth out float vColorTexCoord;

  void main(void) 
  {            
    vColor = Color;
    vColorTexCoord = ColorTexCoord;
    gl_Position = Ortho * Vertex;
  }

};

program NoLit2D
{
    vertex: Matrices *;
    fragment: NoLit;
};


shader[vertex] SkyBox
{
  layout (location = 0) in vec4 Vertex;

  uniform mat4 ModelViewProjectionMatrix;

  smooth out vec3 ViewDirection;

  void main(void) 
  {            
    gl_Position = ModelViewProjectionMatrix * Vertex;
    ViewDirection = Vertex.xyz;
  }
};

shader[fragment] SkyBox
{
  uniform vec4 colorFact;
  uniform samplerCube CubeMap;

  smooth in vec3 ViewDirection;
  out vec4 FragColor;

  void main(void) 
  { 
    FragColor = colorFact * texture( CubeMap, ViewDirection );
    gl_FragDepth = 1.0;
  }
};

program SkyBox
{
    vertex: *;
    fragment: *;
};


shader[vertex] Instancing
{
  layout (location = 0) in vec2 TexCoord;
  layout (location = 1) in vec4 Normal;
  layout (location = 2) in vec4 Vertex;
  layout (location = 3) in mat3x4 T;
  layout (location = 6) in float part;
  layout (location = 7) in vec4 force;

  const vec4 Eye = vec4(0.0, 0.0, 1.0, 0.0);

  uniform sampler1D  ColorTex;
  uniform float diffuse = 1.0;  
  uniform float ambient = 0.0;
  uniform vec2  alphaMask = vec2(0,1);
  uniform vec4 loc[3];
  uniform int up;
  
  flat   out float vDiscard;
  flat   out vec4  vColor;
  smooth out vec2  vTexCoord;
  smooth out float vShade;
  out vec3 fnormal;

  void main(void)
  {
     float time = 0.8*force[3];
     float rot;
	 float M_PI = 3.14159265358979323;
     rot = sin(time)*1.2+0.3;
	 float realPronation = 0.5*(sin(time) - 1);
     float outer = sin(time - 0.5*M_PI);
	 
     float upTest = mod(time, 2 * M_PI);
     int up;
     if (upTest > 0.5 * M_PI && upTest < 1.5 * M_PI) up = 1; else up = 0;
	
	
    
	
    float modelScale = T[0].w;    
    float colorTexCoord = isnan(T[2].w) ? 0.0 : T[2].w;
    mat4 M = mat4( vec4(T[0].xyz, 0.0), 
                   vec4(T[1].xyz, 0.0),
                   vec4(cross(T[0].xyz, T[1].xyz), 0.0), 
                   vec4(T[2].xyz, 1.0) );
	

    vDiscard = (colorTexCoord >= alphaMask.x && colorTexCoord <= alphaMask.y) ? 0.0 : 1.0;
    vTexCoord = TexCoord;                         // Object texture
    vColor = texture(ColorTex, colorTexCoord);    // Color mix texture

    // Normal in view space (remains normalized)
    vec4 normal = ModelView * M * vec4(Normal.xyz, 0);

    // Add an simple headlight (two-sided)
    float ds = abs(dot(normal, Eye));
    vShade = ambient + diffuse * ds;
    fnormal = normalize(normal.xyz);
    // Vertex in local space
    vec4 position = Vertex;
    position.xyz *= modelScale;
	//position += vec4(0,0.2,0,0);
	float pronationDist; float outerDist; float randomLoc; float realPronationDist; float rotationDist; 
	float headDist; float legsDist;
	
	 if (int(part) == 1) pronationDist = 2*exp(-pow(length(vec3(0.1,0.15,-1.3) - Vertex.xyz),2)*2); 
	 if (int(part) == 1) pronationDist = 2*exp(-pow(length(vec3(0.1,0.15,1.3) - Vertex.xyz),2)*2);
	 if (int(part) == 2) realPronationDist = 1-exp(-pow(length(vec3(0.1,0.04,-0.1) - position.xyz),2)*2); 
	 if (int(part) == 1) realPronationDist = 1-exp(-pow(length(vec3(0.1,0.04,0.02) - position.xyz),2)*2); 
	 if (int(part) == 2) rotationDist = 1-exp(-pow(length(vec3(0.1,0.04,-0.02) - position.xyz),2)*40); 
	 if (int(part) == 1) rotationDist = 1-exp(-pow(length(vec3(0.1,0.04,0.1) - position.xyz),2)*40); 
	 if (int(part) > 0) randomLoc = 2*exp(-pow(length(-1 - position.x),2)*2); 
	 if (int(part) == 0) randomLoc = 2*exp(-pow(length(-1.2 - position.x),2)*2); 
	 if (int(part) == 2) outerDist = (1-exp(-pow(-0.2 - position[2],2)*5)); 
	 if (int(part) == 1) outerDist = (1-exp(-pow(0.2 - position[2],2)*5));

	 headDist = 1-exp(-pow(length(0.3 - position.x),2)*5); 
	 legsDist = 1-exp(-pow(length(vec2(-0.4,-0.05) - position.xy),2)*40); 
	 //legsDist=1;
	  mat4 wingRotate = mat4( 1, 0, 0, 0,
	  0, rotationDist*cos(rot) + (1-rotationDist)*1, -rotationDist*sin(rot), 0,
	  0, rotationDist*sin(rot), rotationDist*cos(rot) + (1-rotationDist)*1, 0,
						   0, 0, 0, 1 );
     
	


	 mat4 outerMat = mat4( 1, 0, 0, 0,
	  0,  outerDist*cos(outer) + (1-outerDist)*1, -sin(outer)*outerDist, 0,
	  0, sin(outer)*outerDist,  outerDist*cos(outer) + (1-outerDist)*1, 0,
	  0, 0, 0, 1 );
	  outer = outer *0.5;
	  float random = force[0]*10;
	   mat4 randomMat = mat4( randomLoc*cos(random) + (1-randomLoc)*1, -sin(random)*randomLoc, 0, 0,
	  sin(random)*randomLoc,  randomLoc*cos(random) + (1-randomLoc)*1, 0, 0,
	  0, 0,  1, 0,
	  0, 0, 0, 1 );

	//pronationDist = 1.0f;
	 mat4 pronationMat = mat4( pronationDist*cos(outer) + (1-pronationDist)*1, -sin(outer)*pronationDist, 0, 0,
	  sin(outer)*pronationDist, pronationDist*cos(outer) + (1-pronationDist)*1, 0, 0,
	  0, 0, 1, 0,
	  0, 0, 0, 1 );

	  float legTurn = -0.8;
	  //legTurn = -rot +0.6;
	  mat4 legsMat = mat4( legsDist*cos(legTurn) + (1-legsDist)*1, sin(legTurn)*legsDist, 0, 0,
	  -sin(legTurn)*legsDist, legsDist*cos(legTurn) + (1-legsDist)*1, 0, 0,
	  0, 0, 1, 0,
	  0, 0, 0, 1 );
	   mat4 legsMatback = mat4( legsDist*cos(legTurn) + (1-legsDist)*1, -sin(legTurn)*legsDist, 0, 0,
	  sin(legTurn)*legsDist, legsDist*cos(legTurn) + (1-legsDist)*1, 0, 0,
	  0, 0, 1, 0,
	  0, 0, 0, 1 );

	  legTurn = -rot +0.6;
	  mat4 legsMat2 = mat4( legsDist*cos(legTurn) + (1-legsDist)*1, sin(legTurn)*legsDist, 0, 0,
	  -sin(legTurn)*legsDist, legsDist*cos(legTurn) + (1-legsDist)*1, 0, 0,
	  0, 0, 1, 0,
	  0, 0, 0, 1 );

	   mat4 realPronationMat = mat4( realPronationDist*cos(realPronation) + (1-realPronationDist)*1, -sin(realPronation)*realPronationDist, 0, 0,
	  sin(realPronation)*realPronationDist, realPronationDist*cos(realPronation) + (1-realPronationDist)*1, 0, 0,
	  0, 0, 1, 0,
	  0, 0, 0, 1 );


	   mat4 backwardMat = mat4( pronationDist*cos(outer+0.3) + (1-pronationDist)*1, 0, -sin(outer+0.3)*pronationDist, 0,
	  0, 1, 0, 0,
	  sin(outer+0.3)*pronationDist, 0, pronationDist*cos(outer+0.3) + (1-pronationDist)*1, 0,
	  0, 0, 0, 1 );
	if (int(part) == 2) { 
         position = backwardMat * (position-vec4(0.1,0.2,0,0)) + vec4(0.1,0.2,0,0);
		//position = pronationMat * (position-vec4(0.1,0.2,0,0)) + vec4(0.1,0.2,0,0);
		if (up==1 && position.z < -0.2){
			position = outerMat * (position-vec4(0,0.2,-0.2,0)) + vec4(0,0.2,-0.2,0);
		}
		
		position = wingRotate * (position-(vec4(0,0.05,-0.1,0))) + vec4(0,0.05,-0.1,0);
		position = realPronationMat * (position-vec4(0.1,0.04,0,0)) + vec4(0.1,0.04,0,0);
		
	}
	if (int(part) == 1) {
	    backwardMat[0][2] *= -1;
		backwardMat[2][0] *= -1;
		//pronationMat[1][2] *= -1;
		//pronationMat[2][1] *= -1;
	    mat4x4 outerMatrix = outerMat;
	    outerMatrix[1][2] *= -1;
		outerMatrix[2][1] *= -1;
		vec4 second = vec4(loc[1][0],loc[1][1],-loc[1][2],loc[1][3]);
		position = backwardMat * (position-vec4(0.1,0.2,0,0)) + vec4(0.1,0.2,0,0);
		//position = pronationMat * (position-vec4(0.1,0.2,0,0)) + vec4(0.1,0.2,0,0);

	    if (up==1 && position.z > 0.2){
			position = outerMatrix * (position-vec4(0,0.2,0.2,0)) + vec4(0,0.2,0.2,0);
		}
		
		mat4x4 Matrix = wingRotate;  
	    Matrix[1][2] *= -1;
		Matrix[2][1] *= -1;
		position = Matrix * (position-vec4(0,0.05,0.1,0))+vec4(0,0.05,0.1,0);
		position = realPronationMat * (position-vec4(0.1,0.04,0,0)) + vec4(0.1,0.04,0,0);
		
	}
	if (int(part) == 0 && position.y < -0.05 && position.x < -0.1){
        vec4 pawPosition = vec4(-0.27,-0.10,0.055,0);
		vec4 pawPosition2 = vec4(-0.27,-0.10,-0.055,0);
		if (length(pawPosition2.xyz-position.xyz) < length(pawPosition.xyz-position.xyz) ) pawPosition = pawPosition2;
	    float clawTurn = 0.7*rot;
	    float clawDist = 1-exp(-pow(length(pawPosition.xyz - position.xyz),2)*80); 
		//clawDist = 1;
	    mat4 clawTurnMat = mat4( clawDist*cos(clawTurn) + (1-clawDist), clawDist*sin(clawTurn), 0, 0,
	      -sin(clawTurn)*clawDist, clawDist*cos(clawTurn)+ (1-clawDist), 0, 0,
	      0, 0, 1, 0,
	      0, 0, 0, 1 );
		position = legsMat * (position-vec4(-0.15,-0.06,0,0)) + vec4(-0.15,-0.06,0,0);
		


		float pawDist = length(pawPosition.xyz - position.xyz); 
		float firstTurnClaw = acos(vec2(1,0) * (position.yz -pawPosition.yz) / length(pawPosition.yz - position.yz));
		vec3 firstTurnClaw2 = cross(vec3(1,0,0),((vec3(position.yz,0) -vec3(pawPosition.yz,0)) / length(pawPosition.yz - position.yz)));
		//firstTurnClaw = asin(length(firstTurnClaw2));
		//firstTurnClaw = 0.8;
		 mat4 firstTurnClawMat = mat4( 1, 0, 0, 0,
		 0,  cos(firstTurnClaw), -sin(firstTurnClaw), 0,
		  0, sin(firstTurnClaw),  cos(firstTurnClaw), 0,
		 0, 0, 0, 1 );
			 mat4 firstTurnbackClawMat = mat4( 1, 0, 0, 0,
		 0,  cos(firstTurnClaw), sin(firstTurnClaw), 0,
		  0, -sin(firstTurnClaw),  cos(firstTurnClaw), 0,
		 0, 0, 0, 1 );
		if (pawDist < 0.08 && position.x < -0.27) {
			//position =  firstTurnbackClawMat* clawTurnMat * firstTurnClawMat * (position - pawPosition) + pawPosition;
			//position = 0.5f * (position - pawPosition) + pawPosition + vec4(0,0,0,0.5);
			//vColor = vec4(1,1,1,1);
			//fnormal = vec3(1,1,1);
			}
		//legMatback 
		//position = legsMatback * (position-vec4(-0.15,-0.06,0,0)) + vec4(-0.15,-0.06,0,0);
		//position = legsMat2 * (position-vec4(-0.15,-0.06,0,0)) + vec4(-0.15,-0.06,0,0);
	}
    position = randomMat * position;
	//position = randomMat * position;
    position += vec4(0,sin(rot)*headDist*0.04,0,0);
    position = M * position;
    position = ModelViewProjection * position;
	
	gl_Position = position;
  }
};


shader[fragment] Instancing
{
  uniform sampler2D Texture;
  uniform float texMix;
  
  flat   in float vDiscard;
  flat   in vec4  vColor;
  smooth in vec2  vTexCoord;
  smooth in float vShade;
  in vec3 fnormal;

  out vec4 FragColor;
  
  void main( void )
  {
    if (vDiscard == 1.0) discard;
    FragColor = (1 + 0.0001*vShade) * mix( texture( Texture, vTexCoord ), vColor, texMix )*(0.1 + abs(fnormal.z)) + vec4(0.8, 0.9, 0.7, 1.0) * pow(abs(fnormal.z), 40.0);
  }

};

program Instancing
{
  vertex: Matrices *;
  fragment: *;
};


shader[vertex] Ribbon
{
  layout (location = 0) in vec4 side_T;       // vSide vector & simulation time
  layout (location = 1) in vec4 pos_U;        // position & color texture coord

  out float vColorTexCoord;    // color texture
  out float vTime;             // Time
  out vec3 vPosition;
  out vec3 vSide;              // Side vector

  void main(void)
  {
    vTime = side_T.w;
    vPosition = pos_U.xyz;
    vColorTexCoord = pos_U.w;
    vSide = side_T.xyz;
  }
};


shader[geometry] Ribbon
{
  layout(lines_adjacency) in;
  layout(triangle_strip, max_vertices=4) out;

  const vec4 eye = vec4(0.0, 0.0, 1.0, 0.0);
  const float diffuse = 0.5;
  const float ambient = 1.0 - diffuse;

  uniform sampler1D colorTex;
  uniform float halfWidth;

  in float vTime[4];              // time
  in float vColorTexCoord[4];     // color texture
  in vec3 vPosition[4];           // position
  in vec3 vSide[4];               // vSide vector

  smooth out vec4 gColor;
  smooth out float gSimTime;
  smooth out float gShade;

  float shade(vec3 normal)
  {
    float ds = abs(dot(ModelView * vec4(normal, 0.0), eye));
    return diffuse * ds + ambient;
  }

  void Emit(vec3 v)
  {
    gl_Position = ModelViewProjection * vec4(v, 1.0);
    EmitVertex();
  }

  void main(void)
  {
    float skip = vTime[0] * vTime[1] * vTime[2] * vTime[3];
    if (skip == 0) return;

    vec3 tangent01 = normalize(vPosition[1]-vPosition[0]);
    vec3 tangent21 = normalize(vPosition[2]-vPosition[1]);
    vec3 tangent23 = normalize(vPosition[3]-vPosition[2]);

    vec3 up0 = cross(vSide[0], tangent01);
    vec3 up1 = cross(vSide[1], tangent21);
    vec3 up2 = cross(vSide[2], tangent23);
    
    float shade0 = shade(up0);
    float shade1 = shade(up1);
    float shade2 = shade(up2);
    
    vec4 color0 = texture(colorTex, vColorTexCoord[0]);
    vec4 color1 = texture(colorTex, vColorTexCoord[1]);
    vec4 color2 = texture(colorTex, vColorTexCoord[2]);

    gColor = mix(color0, color1, 0.5);
    gShade = mix(shade0, shade1, 0.5);
    gSimTime = vTime[1];
    Emit(vPosition[1] - halfWidth * vSide[1]);
    Emit(vPosition[1] + halfWidth * vSide[1]);
    
    gColor = mix(color1, color2, 0.5);
    gShade = mix(shade1, shade2, 0.5);
    gSimTime = vTime[2];
    Emit(vPosition[2] - halfWidth * vSide[2]);
    Emit(vPosition[2] + halfWidth * vSide[2]);
  }
};


shader[fragment] Ribbon
{
  uniform float oneOverTickInterval;
  uniform float oneMinusTickWidth;

  smooth in vec4 gColor;
  smooth in float gSimTime;
  smooth in float gShade;

  out vec4 FragColor;

  void main( void )
  {
    float tickIntensity = step(oneMinusTickWidth, fract(oneOverTickInterval * gSimTime));
    FragColor = gShade * clamp(gColor + tickIntensity, 0.0, 1.0) + 0.25 * tickIntensity;
  }
};


program Ribbon
{
  vertex: *;
  geometry: Matrices *;
  fragment: *;
};


shader [vertex] Disk
{
  layout (location = 0) in float Radius;
  out float vRadius;

  void main()
  {
    vRadius = Radius;
  }
};

shader [fragment] Stripe
{
  const float colorFact = 0.75;
  uniform float stripeLen = 10.0;

  smooth in float s;
  out vec4 FragColor;

  void main()
  {
    float rg = colorFact * step(stripeLen, mod(s, 2.0 * stripeLen));
    FragColor = vec4(colorFact, rg, rg, 1.0);
  }
};

shader [geometry] DiskRadii
{
  layout(points) in;
  layout(line_strip, max_vertices=108) out;

  in float vRadius[1];
  smooth out float s;

  void main()
  {
    for (int i=0; i<360; i+=10)
    {
      float angle = radians(float(i));
      vec3 d = vec3(cos(angle), 0.0, sin(angle));

      s = 0.0;
      gl_Position = ModelViewProjection * vec4(0.0, 0.0, 0.0, 1.0);
      EmitVertex();
      
      s = vRadius[0];
      gl_Position = ModelViewProjection * vec4(d * vRadius[0], 1.0);
      EmitVertex();
      
      EndPrimitive();
    }
  }
};

shader [geometry] DiskCircles
{
  layout(points) in;
  layout(line_strip, max_vertices=200) out;

  in float vRadius[1];
  smooth out float s;

  void main()
  {
    for (int i=0; i<=360; i+=2)
    {
      float angle = radians(float(i));
      vec3 d = vec3(cos(angle), 0.0, sin(angle));

      s = angle * vRadius[0];
      gl_Position = ModelViewProjection * vec4(d * vRadius[0], 1.0);
      EmitVertex();
    }
    EndPrimitive();
  }
};


program DiskRadii
{
  vertex: Disk;
  geometry: Matrices *;
  fragment: Stripe;
};

program DiskCircles
{
  vertex: Disk;
  geometry: Matrices *;
  fragment: Stripe;
};


shader [vertex] Grid
{
  layout (location = 0) in vec3 Vertex;
  out vec4 vVertex;

  void main()
  {
    vVertex = vec4(Vertex, 1.0);
  }
};


shader [geometry] Grid
{
  layout(lines) in;
  layout(line_strip, max_vertices=200) out;

  in vec4 vVertex[2];
  smooth out float s;

  void main()
  {
    s = 0.0;
    gl_Position = ModelViewProjection * vVertex[0];
    EmitVertex();

    s = distance(vVertex[0], vVertex[1]);
    gl_Position = ModelViewProjection * vVertex[1];
    EmitVertex();

    EndPrimitive();
  }
};


program Grid
{
  vertex: *;
  geometry: Matrices *;
  fragment: Stripe;
};


shader [vertex] PointSprite
{
  layout (location = 0) in vec3 pos;
  layout (location = 1) in float colorTexCoord;
  layout (location = 2) in float pointScale;

  uniform sampler1D ColorTex;
  uniform vec2  alphaMask = vec2(0,1);
  uniform vec4  Color;

  flat out float vDiscard;
  flat out vec3  vColor;
  flat out vec3  posEye;

  void main()
  {
    float ctex = isnan(colorTexCoord) ? 0.0 : colorTexCoord;
    vDiscard = (ctex >= alphaMask.x && ctex <= alphaMask.y) ? 0.0 : 1.0;
    vColor = mix(Color.rgb, texture(ColorTex, ctex).rgb, Color.a);
    vec4 Vertex = vec4(pos, 1.0);
    gl_Position = ModelViewProjection * Vertex;
    
    // calculate window-space point size
    posEye = vec3(ModelView * Vertex);
	  float dist = length(posEye);
	  gl_PointSize = pointScale / dist;
  }
};


shader [fragment] PointSprite
{
  uniform float PointRadius;    // point size in world

  flat in float vDiscard;
  flat in vec3  vColor;
  flat in vec3  posEye;

  layout (location = 0) out vec4 FragColor;

  void main( void )
  {
    if (vDiscard == 1.0) discard;   // killed by alpha mask

	  vec3 N;
    N.xy = 2.0 * gl_PointCoord.st - vec2(1.0, 1.0);  // [0 1] to [-1 1]
	  float r = dot(N.xy, N.xy);
	  if (r > 1.0) discard;   // kill pixels outside circle
    N.z = sqrt(1.0 - r);    // from sphere equation

    vec4 spherePosEye =vec4(posEye + N * PointRadius, 1.0);
    vec4 clipSpacePos = Projection * spherePosEye;
    float normDepth = clipSpacePos.z / clipSpacePos.w;
    
    gl_FragDepth = 0.5 * ((gl_DepthRange.diff * normDepth) + (gl_DepthRange.far + gl_DepthRange.near));
    FragColor.xyz = (1.0 - r) * vColor;
  }
};


program PointSprite
{
  vertex: Matrices *;
  fragment: Matrices *;
};


shader [vertex] Widget
{
  layout (location = 0) in vec4 vAttribs;     // tex, vert

  out vec2 vTexCoord;
  out vec2 vVertex;

  void main()
  {
    vTexCoord = vAttribs.xy;
    vVertex = vAttribs.zw;
  }
};

shader [geometry] Widget
{
  layout(lines) in;
  layout(triangle_strip, max_vertices=4) out;

  in vec2 vTexCoord[2];
  in vec2 vVertex[2];

  smooth out vec2 gTexCoord;

  void main()
  {
    vec2 t0 = vTexCoord[0];
    vec2 t1 = vTexCoord[1];
    vec2 v0 = vVertex[0];
    vec2 v1 = vVertex[1];

    gTexCoord = vec2(t0.x, t1.y);
    gl_Position = Ortho * vec4(v0.x, v1.y, 0, 1);
    EmitVertex();

    gTexCoord = vec2(t0.x, t0.y);
    gl_Position = Ortho * vec4(v0.x, v0.y, 0, 1);
    EmitVertex();

    gTexCoord = vec2(t1.x, t1.y);
    gl_Position = Ortho * vec4(v1.x, v1.y, 0, 1);
    EmitVertex();

    gTexCoord = vec2(t1.x, t0.y);
    gl_Position = Ortho * vec4(v1.x, v0.y, 0, 1);
    EmitVertex();

    EndPrimitive();
  }
};

shader [fragment] Widget
{
  uniform sampler2D WidgetTexture;

  smooth in vec2 gTexCoord;
  out vec4 FragColor;

  void main()
  {
    FragColor = texture(WidgetTexture, gTexCoord);
  }
};

program Widget
{
  vertex: *;
  geometry: Matrices *;
  fragment: *;
};


