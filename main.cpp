#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#define TEST_OPENGL_ERROR()                                                             \
  do {                                                                                  \
    GLenum err = glGetError();                                                          \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR: "                               \
                                      << gluErrorString(err)                            \
                                      << " file " << __FILE__                           \
                                      << " line " << __LINE__ << std::endl;             \
  } while(0)


static const std::vector<GLfloat> vertex_buffer_data {
    -0.5, 0.0, +0.5,
    +0.5, 0.0, +0.5,
    +0.5, 0.0, -0.5,
    -0.5, 0.0, -0.5
};

GLuint plane_vao_id;
GLuint program_id;
float anim_time = 0.f;

void anim() {
    GLint anim_time_location;
    //glUseProgram(program_id);
    anim_time_location =
            glGetUniformLocation(program_id, "anim_time");
    glUniform1f(anim_time_location, anim_time);
    anim_time += 0.1;
    // testez une borne max.
    glutPostRedisplay();
}

void timer(int value) {
    anim();
    glutTimerFunc(33,timer,0);
}

void init_anim() {
    glutTimerFunc(33,timer,0);
}

void window_resize(int width, int height) {
  //std::cout << "glViewport(0,0,"<< width << "," << height << ");TEST_OPENGL_ERROR();" << std::endl;
  glViewport(0,0,width,height);TEST_OPENGL_ERROR();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();

  GLuint view_loc = glGetUniformLocation(program_id, "view_matrix");TEST_OPENGL_ERROR();
  auto view_matrix = glm::mat4(1.0f);
  view_matrix = glm::rotate(
          glm::translate(view_matrix, glm::vec3(0.0f, -.25f, -3.0f)),
          glm::radians(45.f), glm::vec3(1.f, 0.f, 0.f)
  );
  glUniformMatrix4fv(view_loc, 1, GL_FALSE, &view_matrix[0][0]);TEST_OPENGL_ERROR();

  GLuint mv_loc = glGetUniformLocation(program_id, "model_matrix");TEST_OPENGL_ERROR();
  auto model_matrix = glm::mat4(1.0f);
  glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_matrix[0][0]);TEST_OPENGL_ERROR();

  GLuint proj_loc = glGetUniformLocation(program_id, "projection_matrix");TEST_OPENGL_ERROR();
  glm::mat4 proj_matrix = glm::perspective(glm::radians(45.0f), 1.7f, .1f, 100.f);
  glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &proj_matrix[0][0]);TEST_OPENGL_ERROR();

  glBindVertexArray(plane_vao_id);TEST_OPENGL_ERROR();

  glPatchParameteri(GL_PATCH_VERTICES, 4);TEST_OPENGL_ERROR();
  glDrawArrays(GL_PATCHES, 0, 4);TEST_OPENGL_ERROR();

  glBindVertexArray(0);TEST_OPENGL_ERROR();
  glutSwapBuffers();
}

void init_glut(int &argc, char *argv[]) {
  glewExperimental = GL_TRUE;
  glutInit(&argc, argv);
  glutInitContextVersion(3,3);
  glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(1024, 1024);
  glutInitWindowPosition ( 100, 100 );
  glutCreateWindow("Shader Programming");
  glutDisplayFunc(display);
  glutReshapeFunc(window_resize);
}

bool init_glew() {
  if (glewInit()) {
    std::cerr << " Error while initializing glew";
    return false;
  }
  return true;
}

void init_GL() {
  glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
  //glEnable(GL_CULL_FACE);TEST_OPENGL_ERROR();
  glDisable(GL_CULL_FACE);TEST_OPENGL_ERROR();
  glClearColor(0.4,0.4,0.4,1.0);TEST_OPENGL_ERROR();
}


void init_object_vbo() {
  int max_nb_vbo = 5;
  int nb_vbo = 0;
  int index_vbo = 0;
  GLuint vbo_ids[max_nb_vbo];

  GLint vertex_location = glGetAttribLocation(program_id,"position");TEST_OPENGL_ERROR();

  glGenVertexArrays(1, &plane_vao_id);TEST_OPENGL_ERROR();
  glBindVertexArray(plane_vao_id);TEST_OPENGL_ERROR();

  if (vertex_location!=-1) nb_vbo++;
  glGenBuffers(nb_vbo, vbo_ids);TEST_OPENGL_ERROR();

  if (vertex_location!=-1) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);TEST_OPENGL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size()*sizeof(float), vertex_buffer_data.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
    glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);TEST_OPENGL_ERROR();
    glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
  }

  glBindVertexArray(0);
}


std::string load(const std::string &filename) {
  std::ifstream input_src_file(filename, std::ios::in);
  std::string ligne;
  std::string file_content="";
  if (input_src_file.fail()) {
    std::cerr << "FAIL\n";
    return "";
  }
  while(getline(input_src_file, ligne)) {
    file_content = file_content + ligne + "\n";
  }
  file_content += '\0';
  input_src_file.close();
  return file_content;
}

bool init_shaders() {
  std::string vertex_src = load("vertex.shd");
  std::string fragment_src = load("fragment.shd");
  std::string tess_control_src = load("tessellation_control.shd");
  std::string tess_eval_src = load("tessellation_evaluation.shd");
  GLuint shader_id[4];
  GLint compile_status = GL_TRUE;
  char *vertex_shd_src = (char*)std::malloc(vertex_src.length()*sizeof(char));
  char *fragment_shd_src = (char*)std::malloc(fragment_src.length()*sizeof(char));
  char *tess_control_shd_src = (char*)std::malloc(tess_control_src.length()*sizeof(char));
  char *tess_eval_shd_src = (char*)std::malloc(tess_eval_src.length()*sizeof(char));
  vertex_src.copy(vertex_shd_src,vertex_src.length());
  fragment_src.copy(fragment_shd_src,fragment_src.length());
  tess_control_src.copy(tess_control_shd_src,tess_control_src.length());
  tess_eval_src.copy(tess_eval_shd_src,tess_eval_src.length());


  shader_id[0] = glCreateShader(GL_VERTEX_SHADER);TEST_OPENGL_ERROR();
  shader_id[1] = glCreateShader(GL_FRAGMENT_SHADER);TEST_OPENGL_ERROR();
  shader_id[2] = glCreateShader(GL_TESS_CONTROL_SHADER);TEST_OPENGL_ERROR();
  shader_id[3] = glCreateShader(GL_TESS_EVALUATION_SHADER);TEST_OPENGL_ERROR();

  glShaderSource(shader_id[0], 1, (const GLchar**)&(vertex_shd_src), 0);TEST_OPENGL_ERROR();
  glShaderSource(shader_id[1], 1, (const GLchar**)&(fragment_shd_src), 0);TEST_OPENGL_ERROR();
  glShaderSource(shader_id[2], 1, (const GLchar**)&(tess_control_shd_src), 0);TEST_OPENGL_ERROR();
  glShaderSource(shader_id[3], 1, (const GLchar**)&(tess_eval_shd_src), 0);TEST_OPENGL_ERROR();
  for(int i = 0 ; i < 4 ; i++) {
    glCompileShader(shader_id[i]);TEST_OPENGL_ERROR();
    glGetShaderiv(shader_id[i], GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE) {
      GLint log_size;
      char *shader_log;
      glGetShaderiv(shader_id[i], GL_INFO_LOG_LENGTH, &log_size);
      shader_log = (char*)std::malloc(log_size+1); /* +1 pour le caractere de fin de chaine '\0' */
      if(shader_log != 0) {
	glGetShaderInfoLog(shader_id[i], log_size, &log_size, shader_log);
	std::cerr << "SHADER " << i << ": " << shader_log << std::endl;
	std::free(shader_log);
      }
      std::free(vertex_shd_src);
      std::free(fragment_shd_src);
      std::free(tess_control_shd_src);
      std::free(tess_eval_shd_src);
      glDeleteShader(shader_id[0]);
      glDeleteShader(shader_id[1]);
      glDeleteShader(shader_id[2]);
      glDeleteShader(shader_id[3]);
      return false;
    }
  }
  std::free(vertex_shd_src);
  std::free(fragment_shd_src);
  std::free(tess_control_shd_src);
  std::free(tess_eval_shd_src);


  GLint link_status=GL_TRUE;
  program_id=glCreateProgram();TEST_OPENGL_ERROR();
  if (program_id==0) return false;
  for(int i = 0 ; i < 4 ; i++) {
    glAttachShader(program_id, shader_id[i]);TEST_OPENGL_ERROR();
  }
  glLinkProgram(program_id);TEST_OPENGL_ERROR();
  glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
  if (link_status!=GL_TRUE) {
    GLint log_size;
    char *program_log;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
    program_log = (char*)std::malloc(log_size+1); /* +1 pour le caractere de fin de chaine '\0' */
    if(program_log != 0) {
      glGetProgramInfoLog(program_id, log_size, &log_size, program_log);
      std::cerr << "Program " << program_log << std::endl;
      std::free(program_log);
    }
    glDeleteProgram(program_id);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[0]);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[1]);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[2]);TEST_OPENGL_ERROR();
    glDeleteShader(shader_id[3]);TEST_OPENGL_ERROR();
    program_id=0;
    return false;
  }
  glUseProgram(program_id);TEST_OPENGL_ERROR();
  return true;
}


int main(int argc, char *argv[]) {
  init_glut(argc, argv);
  if (!init_glew())
    std::exit(-1);
  init_GL();
  init_shaders();
  init_object_vbo();
  init_anim();
  glutMainLoop();
}
