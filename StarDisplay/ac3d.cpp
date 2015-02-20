// ac3d.cpp .ac importer.
//
// Hanno Hildenbrandt 2010

#include <fstream>
#include <algorithm>
#include <numeric>
#include <exception>
#include <iterator>
#include "filesystem.hpp"
#include <boost/circular_buffer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glmutils/bbox.hpp>
#include <glmutils/istream.hpp>
#include <glmutils/homogeneous.hpp>
#include <hrtree/isfc/key_gen.hpp>
#include <hrtree/isfc/hilbert.hpp>
#include "ac3d.hpp"
#include "Globals.hpp"
#include "GLSLState.hpp"


HRTREE_ADAPT_POINT_FUNCTION(glm::vec3, float, 3, glm::value_ptr);


namespace {

  typedef std::vector<glm::vec3> pos_vect;


  struct eps_eq_vertex
  {
    eps_eq_vertex(const T2F_N3F_V3F& pivot) : pivot_(pivot) {}
    bool operator() (const T2F_N3F_V3F& x)
    {
      return glm::all(glm::epsilonEqual(pivot_.t, x.t, 0.0001f)) && 
             glm::all(glm::epsilonEqual(pivot_.v, x.v, 0.0001f)) && 
             glm::all(glm::epsilonEqual(pivot_.n, x.n, 0.2f));
    }
    T2F_N3F_V3F pivot_;
  };


  struct acTriangle
  {
    int vi[3];
    glm::vec2 tex[3];
  };


  struct hilbert_cmp_tri_center
  {
    typedef hrtree::hilbert<3,21>::type key_type;
    typedef hrtree::key_gen<key_type, glm::vec3> gen_type;

    hilbert_cmp_tri_center(const pos_vect& V, const glmutils::bbox3& d) : v(V), domain(d.p0(), d.p1()) {}
    bool operator () (const acTriangle& a, const acTriangle& b) const
    {
      glm::vec3 ca = (1.0f/3.0f) * (v[a.vi[0]] + v[a.vi[1]] + v[a.vi[2]]);
      glm::vec3 cb = (1.0f/3.0f) * (v[b.vi[0]] + v[b.vi[1]] + v[b.vi[2]]);
      return domain(ca) < domain(cb);
    }

    gen_type domain;
    const pos_vect& v;
  };


  struct acObject
  {
    acObject(): kids(-1), twoSided(false) {}

    int kids;
    int mat;
    std::string name;
    std::string texture;
    float crease;
    glm::mat3 rot;
    glm::vec3 loc[3];
    pos_vect vert;
    std::vector<acTriangle> triangles;
    bool twoSided;
    std::vector<float> part;
  };


  class acParser
  {
  public:
    acParser(const filesystem::path& ShaderPath, const std::string& acFile)
      : ShaderPath_(ShaderPath), acFileName_(acFile), is_((ShaderPath / acFile).string().c_str()) {}

    // Parse ac3d file. May throw.
    void Start();

    // Calculate T4F_N4F_V4F vector from object objectID and its kids.
    ac3d_model Triangulate(size_t objectID);
  
  public:
    void takeAction(const std::string& token);
    void read_ignore();
    void read_MATERIAL();
    void read_OBJECT();
    void read_texture();
    void read_crease();
    void read_rot();
    void read_loc();
    void read_numvert();
    void read_numsurf();
    void read_kids();
    void read_rgb();
    void read_amb();
    void read_emis();
    void read_shi();
    void read_trans();
    void read_fail();
  
  private:
    // Translate the object and its kids.
    void TranslateObj(size_t i, glm::mat4 M, pos_vect& v);
    glm::vec3 vertexNormal(const acObject& obj, int face, int id);
    void computeACMR(const ac3d_model& model) const;

  private:
    std::fstream is_;
    ac3d_material material_;
    std::vector<acObject> objects_;
    const filesystem::path ShaderPath_;
    std::string acFileName_;
	int startNumb[4];
	int partIndex;
	
  };


  struct acAction 
  {
    typedef  void (acParser::*apmf)();
    acAction(const char* tok, apmf p): token(tok), pmf(p) {}
    const char* token;
    apmf pmf;
  } acActions[] = 
  {
    acAction("AC3Db",   &acParser::read_ignore),
    acAction("MATERIAL",&acParser::read_MATERIAL),
    acAction("OBJECT",  &acParser::read_OBJECT),
    acAction("name",    &acParser::read_ignore),
    acAction("data",    &acParser::read_ignore),
    acAction("texture", &acParser::read_texture),
    acAction("texrep",  &acParser::read_ignore),
    acAction("crease",  &acParser::read_crease),
    acAction("rot",     &acParser::read_rot),
    acAction("loc",     &acParser::read_loc),
    acAction("url",     &acParser::read_ignore),
    acAction("numvert", &acParser::read_numvert),
    acAction("numsurf", &acParser::read_numsurf),
    acAction("kids",    &acParser::read_kids),
    acAction("fail",    &acParser::read_fail)
  };


  void acParser::Start()
  {
    std::string tok;
    while (!is_.eof())
    {
      is_ >> tok;
      takeAction(tok);  
    }
  }


  void acParser::takeAction(const std::string& token)
  {
    bool taken = false;
    for (int i=0; i<(sizeof(acActions)/sizeof(acAction)); ++i) 
    {
      if (0 == std::strcmp(acActions[i].token, token.c_str())) 
      {
        (this->*acActions[i].pmf)();
        taken = true;
        break;
      }
    }
    if (!taken) read_fail();
  }


  void acParser::read_ignore() 
  { 
    char buf[128]; is_.getline(buf, 128); 
  }


  void acParser::read_OBJECT() 
  {
    objects_.emplace_back();
	partIndex = 0;
	startNumb[0] = 0;
	startNumb[1] = 0;
	startNumb[2] = 0;
    std::string tok;
    is_ >> tok;
    while (!is_.eof())
    {
      is_ >> tok;        // skip
      takeAction(tok);
      if (objects_.back().kids >= 0) break;  // last object token
    }
	acObject& obj(objects_.back());
	startNumb[3] = obj.vert.size();
	for (int i = 0; i < partIndex; i++)
	{
		std::cout << "\nint i: "<< i;
		std::cout << "\nStartNumb: " << startNumb[i];
		glm::mat4 M = glm::translate(glm::mat4(1), obj.loc[i]);
		glmutils::transformPoints(M, static_cast<int>(startNumb[i + 1] - startNumb[i]), obj.vert.begin() + startNumb[i], obj.vert.begin() + startNumb[i]);
	}
  }


  void acParser::read_MATERIAL() 
  {
    std::string tok;
    is_ >> tok;            // material name
    is_ >> tok >> material_.rgb;
    is_ >> tok >> material_.amb;
    is_ >> tok >> material_.emis;
    is_ >> tok >> material_.spec;
    is_ >> tok >> material_.shi;
    is_ >> tok >> material_.trans;
  }


  void acParser::read_texture() 
  { 
    std::string texName;
    is_ >> texName;
    texName.erase(0, 1);          // strip '"'
    texName.erase(texName.length()-1, 1);  // strip '"'
    objects_.back().texture = (ShaderPath_ / filesystem::path(texName).filename()).string(); 
  }


  void acParser::read_crease() 
  { 
    is_ >> objects_.back().crease; 
  }
  

  void acParser::read_rot() 
  { 
    is_ >> objects_.back().rot; 
  }
  

  void acParser::read_loc() 
  { 
	  is_ >> objects_.back().loc[partIndex];
  }
  

  void acParser::read_numvert() 
  {
	  startNumb[partIndex] = objects_.back().vert.size();
    int N; is_ >> N;
    for (int i=0; i<N; ++i)
    {
      glm::vec3 v; is_ >> v;
      objects_.back().vert.push_back(v);
	  objects_.back().part.push_back(float(partIndex));
    }
  }
  

  void acParser::read_numsurf() 
  {
	
	std::cout << "\n Startnumb: " << startNumb;
    int S; is_ >> S;
    for (int i=0; i<S;) 
    {
      int refs;
      unsigned flags;
      std::string tok; is_ >> tok;
      if (0 == strcmp(tok.c_str(), "SURF")) 
      {
        std::hex(is_); 
        is_ >> flags;
        std::dec(is_);
        if (flags >> 5) objects_.back().twoSided = true;
      }
      else if (0 == strcmp(tok.c_str(), "mat")) is_ >> refs;
      else if (0 == strcmp(tok.c_str(), "refs")) 
      {
        is_ >> refs;
        if (3 != refs) read_fail();
        acTriangle surf;
        for (int r=0; r<refs; ++r) 
        {
		  is_>> surf.vi[r] ;
		  surf.vi[r] += startNumb[partIndex];
		  //std::cout << "\n surf.vi[r]: "<< surf.vi[r];
          is_ >> surf.tex[r];
		  //std::cout << "\n surf.tex[r]: " << surf.tex[r];
        }
        objects_.back().triangles.push_back(surf);
        ++i;
      }
      else read_fail();
    }
	partIndex++;
  }


  void acParser::read_kids() 
  { 
    is_ >> objects_.back().kids; 
  }
  

  void acParser::read_fail() 
  {  
    throw std::exception((std::string("Parsing AC3D file '") + acFileName_ + "' failed").c_str()); 
  }


  void acParser::computeACMR(const ac3d_model& model) const
  {
    if (PARAMS.DebugLevel < 1) return;
    int cacheMiss = 0;
    boost::circular_buffer<GLint> cache(32, -1);    // FIFO cache
    for (size_t i=0; i < model.indices.size(); ++i)
    {
      if (cache.end() == std::find(cache.begin(), cache.end(), model.indices[i]))
      {
        cache.push_back(model.indices[i]);
        ++cacheMiss;
      }
    }
    std::cerr << acFileName_ << ":\n";
    std::cerr << "  Vertices: " << model.vertices.size() << '\n';
    std::cerr << "  Surfaces: " << (model.indices.size() / 3) << '\n';
    std::cerr << "  ACMR:     " << double(cacheMiss) / (model.indices.size() / 3) << '\n';
  }


  ac3d_model acParser::Triangulate(size_t objectID)
  {
    ac3d_model model;
    pos_vect v; 
	TranslateObj(objectID, glm::rotate(glm::mat4(1), 90.0f, glm::vec3(0,1,0)), v);
    model.bbox = glmutils::bbox3(static_cast<int>(v.size()), v.begin());
    acObject& obj(objects_[objectID]);
	float* ptr = &(obj.vert[0]).x;
	std::cout << "\n total vertices:  " << obj.vert.size() << "\n first: " << ptr[0] << "\n total triangles:  " << obj.triangles.size();
    // Sort triangle by hilbert value (increase cache coherence)
    hilbert_cmp_tri_center cmp(v, model.bbox);
    std::sort(obj.triangles.begin(), obj.triangles.end(), std::ref(cmp));
    if (obj.vert.empty()) return model;
    model.twoSided = obj.twoSided;
	std::cout << "\n total vertices:  " << obj.vert.size() << "\n first: " << ptr[0] << "\n total triangles:  " << obj.triangles.size();
	int countDouble = 0;
    for (long i=0; i<static_cast<long>(obj.triangles.size()); ++i)
    {
      const acTriangle& tri(obj.triangles[i]);
      T2F_N3F_V3F vertex[3];
      for (int j=0; j<3; ++j) 
      {
		  
        vertex[j].v = v[tri.vi[j]];
        vertex[j].t = glm::vec2(tri.tex[j].x, 1.0f - tri.tex[j].y);
        vertex[j].n = vertexNormal(obj, i, tri.vi[j]);
		vertex[j].part = obj.part[tri.vi[j]];
		//std::cout << "\npart: " << obj.part[tri.vi[j]];
		
        std::vector<T2F_N3F_V3F>::iterator it = 
          std::find_if(model.vertices.begin(), model.vertices.end(), eps_eq_vertex(vertex[j]));
        if (it != model.vertices.end())
        {
			
			countDouble++;
          model.indices.push_back(static_cast<GLuint>(std::distance(model.vertices.begin(), it)));
        }
        else
        {
          model.vertices.push_back(vertex[j]);
          model.indices.push_back(static_cast<GLuint>(model.vertices.size() - 1));
        }
      }
    }
	for (int i = 0; i < 3; i++){
		model.loc[i] = obj.loc[i];
	}
	std::cout << "\n"<< countDouble << "\n";
	std::cout << "\n total vertices:  " << model.vertices.size() << "\n";
    model.texFile = obj.texture;
    model.material = material_;
    computeACMR(model);
    return model;
  }


  struct cmp_normal
  {
    bool operator() (const glm::vec3& a, const glm::vec3& b) const 
    {
      if (a.x < b.x) return true;
      if (a.x > b.x) return false;
      if (a.y < b.y) return true;
      if (a.y > b.y) return false;
      return (a.z < b.z);
    }
  };


  struct crease_cmp_normal
  {
    crease_cmp_normal(float crease): crease_(crease) {}

    bool operator() (const glm::vec3& a, const glm::vec3& b) const
    {
      float cdot = glm::dot(a,b);
      return (cdot > 0.999f) || (cdot < crease_);
    }

    float crease_;
  };


  void acParser::TranslateObj(size_t i, glm::mat4 M, pos_vect& v)
  {
    acObject& obj(objects_[i]);
    M = glm::translate(M, obj.loc[0]);
    glmutils::transformPoints(M, static_cast<int>(obj.vert.size()), obj.vert.begin(), std::back_inserter(v));
    for (int k=0; k<obj.kids; ++k) 
    {
      TranslateObj(i+1+k, M, v);
    }
  }


  glm::vec3 acParser::vertexNormal(const acObject& obj, int face, int id)
  {
    // Terrible ineffective but how cares?
    std::vector<int> adjacent;
    adjacent.push_back(face);
    for (size_t i=0; i<obj.triangles.size(); ++i)
    {
      if ((id == obj.triangles[i].vi[0]) || (id == obj.triangles[i].vi[1]) || (id == obj.triangles[i].vi[2]))
      {
        adjacent.push_back(static_cast<int>(i));
      }
    }
    pos_vect normals;
    for (size_t i=0; i<adjacent.size(); ++i)
    {
      glm::vec3 v0 = obj.vert[obj.triangles[adjacent[i]].vi[0]];
      glm::vec3 v1 = obj.vert[obj.triangles[adjacent[i]].vi[1]];
      glm::vec3 v2 = obj.vert[obj.triangles[adjacent[i]].vi[2]];
      glm::vec3 normal = glm::normalize( glm::cross(glm::normalize(v2-v0), glm::normalize(v1-v0)) );
      normals.push_back(normal);
    }
    std::sort(normals.begin()+1, normals.end(), cmp_normal());
    pos_vect::iterator uend = 
      std::unique(normals.begin()+1, normals.end(), crease_cmp_normal(cos(glm::radians(obj.crease))));
    normals.erase(uend, normals.end());
    glm::vec3 normal = std::accumulate(normals.begin(), normals.end(), glm::vec3(0));
    return glm::normalize(normal / static_cast<float>(normals.size()));
  }

}


ac3d_model ImportAC3D(const filesystem::path& ShaderPath, const std::string& ac3dFile)
{
  acParser parser(ShaderPath, ac3dFile);
  parser.Start();
  return parser.Triangulate(1);
}


