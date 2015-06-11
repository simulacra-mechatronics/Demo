#include "Shader.h"



/**
	Given a shader program, validateProgram will request from OpenGL, any information
	related to the validation or linking of the program with it's attached shaders. It will
	then output any issues that have occurred.
*/
static void validateProgram(GLuint program) {
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;

    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the program
    if (length > 0) // If we have any information to display
        std::cerr << "Program " << program << " link error: " << buffer << std::endl; // Output the information

    glValidateProgram(program); // Get OpenGL to try validating the program
    GLint status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status); // Find out if the shader program validated correctly
    if (status == GL_FALSE) // If there was a problem validating
		std::cerr << "Error validating shader " << program << std::endl; // Output which program had the error
}



Shader::Shader()
{
    //ctor
}

Shader::Shader(const std::string& vsFileName, const std::string& fsFileName){
    inited = false;                                             // Shader has not been initialized
    init(vsFileName, fsFileName);
}

void Shader::init(const std::string& vsFileName, const std::string& fsFileName){

    if (inited) // If we have already initialized the shader
        return;

    inited = true; // Mark that we have initialized the shader

    hVertexShader.id = glCreateShader(GL_VERTEX_SHADER);        // Create a vertex shader and assign id to our hVertexShader ID variable
    hFragmentShader.id = glCreateShader(GL_FRAGMENT_SHADER);    // Create a fragment shader and assign id to our hVertexShader ID variable


    hVertexShader.source = readFile(vsFileName);                // Read in our vertex shader
    hFragmentShader.source = readFile(fsFileName);              // Read in our fragment shader

    if (hVertexShader.source.empty() || hFragmentShader.source.empty()) // Make sure the shader files are not empty
    {
        std::cerr << "Could not initialize the shaders." << std::endl;  // Output error to debug terminal
        return;
    }

    const GLchar* tmp = static_cast<const GLchar*>(hVertexShader.source.c_str());
    glShaderSource(hVertexShader.id, 1, (const GLchar**)&tmp, NULL);

    tmp = static_cast<const GLchar*>(hFragmentShader.source.c_str());
    glShaderSource(hFragmentShader.id, 1, (const GLchar**)&tmp, NULL);

    if (!Shader::compileShader(hVertexShader) || !Shader::compileShader(hFragmentShader))
    {
        std::cerr << "Shaders are invalid! Could not compile shaders." << std::endl;
        std::cerr << "Could not initialize shaders." << std::endl;
        return;
    }

    hShaderProgram = glCreateProgram();

    glAttachShader(hShaderProgram, hVertexShader.id);
    glAttachShader(hShaderProgram, hFragmentShader.id);

    //Bind the attribute locations
    glBindAttribLocation(hShaderProgram, 0, "in_Position");         // Bind a constant attribute location for positions of vertices
    glBindAttribLocation(hShaderProgram, 1, "in_Color");         // Bind a constant attribute location for color

    glLinkProgram(hShaderProgram);
    validateProgram(hShaderProgram);
}


string Shader::readFile(const string& filename)
{
    ifstream fileIn(filename.c_str());

    if (!fileIn.good())
    {
        std::cerr << "Could not load shader: " << filename << std::endl;
        return string();
    }

    string stringBuffer(std::istreambuf_iterator<char>(fileIn), (std::istreambuf_iterator<char>()));
    return stringBuffer;
}


bool Shader::compileShader(const Shader::hShader& shader)
{
    glCompileShader(shader.id);
    GLint result = GL_FALSE;
    glGetShaderiv(shader.id, GL_COMPILE_STATUS, &result);

    if (!result)
    {
        std::cout << "Could not compile shader: " << shader.id << std::endl;
        echoShaderLog(shader.id);
        return false;
    }

    return true;
}


void Shader::echoShaderLog(unsigned int shaderID)
{
    vector<char> infoLog;
    GLint infoLen;
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLen);
    infoLog.resize(infoLen);

    std::cerr << "Shader contains errors!" << std::endl;
    glGetShaderInfoLog(shaderID, sizeof(infoLog), &infoLen, &infoLog[0]);

    std::cerr << string(infoLog.begin(), infoLog.end()) << std::endl;
}

GLuint Shader::id(){
    return hShaderProgram;
}

void Shader::Bind(){
    glUseProgram(hShaderProgram);   //Enable our shader
}

void Shader::unbind(){
    glUseProgram(0);
}

Shader::~Shader()
{
    glDetachShader(hShaderProgram, hVertexShader.id);
    glDetachShader(hShaderProgram, hFragmentShader.id);

    glDeleteShader(hVertexShader.id);
    glDeleteShader(hFragmentShader.id);

    glDeleteProgram(hShaderProgram);
}
