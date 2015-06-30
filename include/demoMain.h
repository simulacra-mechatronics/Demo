#ifndef DEMOMAIN_H_INCLUDED
#define DEMOMAIN_H_INCLUDED

#include "openGLWindow.h"
#include "Shader.h"     // Our GLSL Shader class

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

extern glm::mat4 projectionMatrix;

void InitializeDemo();
void RenderNextDemoFrame();
void createSquare();

#endif // DEMOMAIN_H_INCLUDED
