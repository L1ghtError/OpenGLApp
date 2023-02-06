#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Shader.hpp"
#include "stb_image.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <math.h>
#include "glm.hpp"
#include "Camera.h"
#include "Model.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool toggleCursor=1;
glm::mat4 view = glm::mat4(1.0f);
int SCR_WIDTH =1920 , SCR_HEIGHT =1080;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
float offsetNumerator=1;
GLFWwindow* window;
float zoom = 0.0;
float velocity = 1;
float Fov = 46;
bool processPause = 1;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90, pitch=0;
float deltaTime = 0.0f;	// время между текущим кадром и последним кадром
float lastFrame = 0.0f;
int FPS=0;
//IMGUI PARAMS
glm::vec3 BoxColor = { 1.0f, 0.5f, 0.31f };
glm::vec3 lightAmbientColor = { 0.2f, 0.2f, 0.2f };
glm::vec3 materialDiffusion = { 1.0f, 0.5f, 0.31f };
glm::vec3 lightDiffusion = { 0.5f, 0.5f, 0.5f };
glm::vec3 materialSpecular = { 1.0f, 1.0f, 1.0f };
glm::vec3 lightSpecular = { 1.0f, 1.0f, 1.0f };
float diffAmount=0.0f;

float innerFlashlightCutOff = 12.5f;
float outerFlashlightCutOff = 15.5f;


bool toggleSunLight = 1;
void imguiWindow() {
    
    ImGui::SetNextWindowSize(ImVec2(420, 320));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (ImGui::Begin("Project stats", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {

        std::string xPos = std::to_string(lightAmbientColor[0]).c_str();
        std::string yPos = std::to_string(lightAmbientColor[1]).c_str();
        std::string zPos = std::to_string(lightAmbientColor[2]).c_str();
        ImGui::Text(("Color R: " + xPos + " G: " + yPos + " B: " + zPos).c_str());
        //ImGui::Text(std::to_string('t').c_str());
        ImGui::Text(("FPS: "+std::to_string(FPS)).c_str());
        ImGui::Text("Color");
        ImGui::ColorEdit3("Light Ambient Color", glm::value_ptr(lightAmbientColor));
        //ImGui::ColorEdit3("Box Color", glm::value_ptr(BoxColor));
        ImGui::Text("Diffusion");
        //ImGui::ColorEdit3("Box Diff", glm::value_ptr(materialDiffusion));
        ImGui::ColorEdit3("Light Diff", glm::value_ptr(lightDiffusion));
        ImGui::Text("Specular");
        //ImGui::ColorEdit3("Box Spec", glm::value_ptr(materialSpecular));
        ImGui::ColorEdit3("Light Spec", glm::value_ptr(lightSpecular));
        ImGui::Text("Flashlight");
        ImGui::DragFloat("inner radius ", &innerFlashlightCutOff, 1.0f, 1.0f, 90.0f);
        ImGui::DragFloat("outer radius ", &outerFlashlightCutOff, 1.0f, 1.0f, 90.0f);
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("offset numerator", &offsetNumerator, 0.00001f, 1, 2);
        ImGui::Checkbox("Toggle sun light", &toggleSunLight);
        if (ImGui::Button("Button")) {
            std::cout << "button pressed";

        }

    }ImGui::End();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Fov += yoffset;
}
unsigned int loadTexture(char const* path);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window)
{
    glfwSetScrollCallback(window, scroll_callback);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        velocity = 2;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        velocity = 1;

    glfwSetKeyCallback(window, key_callback);

  
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){ //recalculate viewport
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    glViewport(0, 0, width, height);
    
    

    
    
    }

   
    if (toggleCursor) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
       
        glfwSetCursorPosCallback(window, mouse_callback);
    }
    else if(toggleCursor == 0) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true;
       glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
    }
   
    Camera_Movement myMovment;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    //cameraPos.y = 0.0f; Gmod camera
}






int main(void)
{
    


   
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Opengl App", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
   if (!(glewInit() == GLEW_OK)) { std::cout << "error"; };
   
   
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   (void)io;
   ImGui_ImplGlfw_InitForOpenGL(window,true);
   ImGui_ImplOpenGL3_Init("#version 330");

 
   Shader ourShader("..\\resources\\Shaders\\color.vs", "..\\resources\\Shaders\\color.fs");
   Shader lampShader("..\\resources\\Shaders\\lamp.vs", "..\\resources\\Shaders\\lamp.fs");
   Shader frameBufferShader("..\\resources\\Shaders\\Framebuffer.vs", "..\\resources\\Shaders\\Framebuffer.fs");
   Shader stencilShader("..\\resources\\Shaders\\outline.vs", "..\\resources\\Shaders\\outline.fs");
   Shader ReflectedShader("..\\resources\\Shaders\\ReflectedShader.vs", "..\\resources\\Shaders\\ReflectedShader.fs");

   Shader lightingShader("..\\resources\\Shaders\\color.vs", "..\\resources\\Shaders\\color.fs");
   Model ourModel((char*)"../resources/objects/backpack/backpack.obj");
   Shader skyboxShader("..\\resources\\Shaders\\Skybox.vs", "..\\resources\\Shaders\\Skybox.fs");

   skyboxShader.use();
   glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);
   unsigned int diffuseMap= loadTexture("..\\resources\\Textures\\1-5.png");
   unsigned int specularMap = loadTexture("..\\resources\\Textures\\3-4.png");
   unsigned int emmisionMap = loadTexture("..\\resources\\Textures\\7.jpg");
   
   
   float floorVertices[]{
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
       0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
       0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
   };

   float vertices[] = {
       // координаты        // нормали           // текстурные координаты
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // нижняя-левая
         0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // нижняя-правая    
         0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // верхняя-правая              
         0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // верхняя-правая
        -0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // верхняя-левая
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // нижняя-левая                
        // передняя грань
       -0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, // нижняя-левая
        0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // верхняя-правая
        0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 1.0f, 0.0f, // нижняя-правая        
        0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // верхняя-правая
       -0.5f, -0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, // нижняя-левая
       -0.5f,  0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f, // верхняя-левая        
       // грань слева
       -0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // верхняя-правая
       -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // нижняя-левая
       -0.5f,  0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // верхняя-левая       
       -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // нижняя-левая
       -0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // верхняя-правая
       -0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // нижняя-правая
      // грань справа
        0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-левая
        0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // верхняя-правая      
        0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-правая          
        0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-правая
        0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // нижняя-левая
        0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-левая
      // нижняя грань          
       -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // верхняя-правая
        0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // нижняя-левая
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // верхняя-левая        
        0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // нижняя-левая
       -0.5f, -0.5f, -0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // верхняя-правая
       -0.5f, -0.5f,  0.5f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // нижняя-правая
     // верхняя грань
      -0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // верхняя-левая
       0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // верхняя-правая
       0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // нижняя-правая                 
       0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // нижняя-правая
      -0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, // нижняя-левая  
      -0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f  // верхняя-левая       
   };
   float quadVertices[] = { // атрибуты вершин в нормализованных координатах устройства для прямоугольника, который имеет размеры экрана 
       // координаты // текстурные координаты
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
   };

   

 


   unsigned int VBO, cubeVAO;
   glGenVertexArrays(1, &cubeVAO);
   
   glGenBuffers(1, &VBO);

   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   glBindVertexArray(cubeVAO);

   // Координатные атрибуты
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);

   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
   glEnableVertexAttribArray(2);

   unsigned int quadVAO, quadVBO;
   glGenVertexArrays(1, &quadVAO);
   glGenBuffers(1, &quadVBO);
   glBindVertexArray(quadVAO);
   glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

   // 2. Настраиваем VAO света (VBO остается неизменным; вершины те же и для светового объекта, который также является 3D-кубом)
   unsigned int lightVAO;
   glGenVertexArrays(1, &lightVAO);
   glBindVertexArray(lightVAO);

   // Нам нужно только привязаться к VBO (чтобы связать его с glVertexAttribPointer), нам не нужно его заполнять; данные VBO уже содержат всё, что нам нужно (они уже привязаны, но мы делаем это снова в образовательных целях) 
   glBindBuffer(GL_ARRAY_BUFFER, VBO);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
 
   unsigned int floorVAO,floorVBO;
   glGenVertexArrays(1, &floorVAO);
   glGenBuffers(1, &floorVBO);
   glBindBuffer(GL_ARRAY_BUFFER, floorVBO);

   glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
   glBindVertexArray(floorVAO);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);

   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
   glEnableVertexAttribArray(2);

   


   glm::vec3 cubePositions[] = {
       glm::vec3(0.0f,  0.0f,  0.0f),
       glm::vec3(2.0f,  5.0f, -15.0f),
       glm::vec3(-1.5f, -2.2f, -2.5f),
       glm::vec3(-3.8f, -2.0f, -12.3f),
       glm::vec3(2.4f, -0.4f, -3.5f),
       glm::vec3(-1.7f,  3.0f, -7.5f),
       glm::vec3(1.3f, -2.0f, -2.5f),
       glm::vec3(1.5f,  2.0f, -2.5f),
       glm::vec3(1.5f,  0.2f, -1.5f),
       glm::vec3(-1.3f,  1.0f, -1.5f)
   };

   glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
   };


   float skyboxVertices[] =
   {
       //   Coordinates
       -1.0f, -1.0f,  1.0f,//        7--------6
        1.0f, -1.0f,  1.0f,//       /|       /|
        1.0f, -1.0f, -1.0f,//      4--------5 |
       -1.0f, -1.0f, -1.0f,//      | |      | |
       -1.0f,  1.0f,  1.0f,//      | 3------|-2
        1.0f,  1.0f,  1.0f,//      |/       |/
        1.0f,  1.0f, -1.0f,//      0--------1
       -1.0f,  1.0f, -1.0f
   };

   unsigned int skyboxIndices[] =
   {
       // Right
       1, 2, 6,
       6, 5, 1,
       // Left
       0, 4, 7,
       7, 3, 0,
       // Top
       4, 5, 6,
       6, 7, 4,
       // Bottom
       0, 3, 2,
       2, 1, 0,
       // Back
       0, 1, 5,
       5, 4, 0,
       // Front
       3, 7, 6,
       6, 2, 3
   };
   

   unsigned int skyboxTextureID,skyboxVAO, skyboxVBO, skyboxEBO;
   glGenVertexArrays(1, &skyboxVAO);
 
   glGenBuffers(1, &skyboxVBO);
   glGenBuffers(1, &skyboxEBO);
   glBindVertexArray(skyboxVAO);
   glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   std::string facesCubemap[6]{
       "../resources/SkyboxTextures/px.png",
       "../resources/SkyboxTextures/nx.png",
       "../resources/SkyboxTextures/py.png",
       "../resources/SkyboxTextures/ny.png",
       "../resources/SkyboxTextures/pz.png",
       "../resources/SkyboxTextures/nz.png",
   };

   glGenTextures(1, &skyboxTextureID);
   glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
   
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   // These are very important to prevent seams
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



   int width, height, nrChannels;
   unsigned char* data;
   for (unsigned int i = 0; i <6; i++)
   {
       stbi_set_flip_vertically_on_load(false);
       

       data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
       
  
       if(data){
            
       glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
       }
       else {
           std::cout << "\nFailed load skybox texture: "<<facesCubemap[i];
       }
       stbi_image_free(data);
   }



   unsigned int framebuffer;
   glGenFramebuffers(1, &framebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

   // Создание текстуры для прикрепляемого объекта цвета
   unsigned int textureColorbuffer;
   glGenTextures(1, &textureColorbuffer);
   glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

   // Создание объекта рендербуфера дла прикрепляемых объектов глубины и трафарета (сэмплирование мы не будет здесь проводить)
   unsigned int rbo;
   glGenRenderbuffers(1, &rbo);
   glBindRenderbuffer(GL_RENDERBUFFER, rbo);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // использование одного объекта рендербуфера для буферов глубины и трафарета
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // теперь прикрепляем это дело

   // Теперь, когда мы создали фреймбуфер и прикрепили все необходимые объекты, проверяем завершение формирования фреймбуфера
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
       std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


   

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   //Enable functionality
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);

   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
   
   glfwSwapInterval(1);
  
   //glEnable(GL_CULL_FACE);
   //glCullFace(GL_FRONT);
   //glFrontFace(GL_CW);
   //

   glClearColor(0.2f, 0.1f, 0.2f, 1.0f);
   double prevTime =0 ;
   double timeDiff;
   unsigned couter = 0;
   frameBufferShader.use();
   frameBufferShader.setInt("screenTexture", 0);

   frameBufferShader.use();
   ReflectedShader.setInt("skybox", 0);


   while (!glfwWindowShouldClose(window)) {
       
       // per-frame time logic
          // --------------------
       float currentFrame = static_cast<float>(glfwGetTime());
       deltaTime = currentFrame - lastFrame;
       lastFrame = currentFrame;

       ImGui_ImplOpenGL3_NewFrame();
       ImGui_ImplGlfw_NewFrame();
       ImGui::NewFrame();

       // input
       // -----
       imguiWindow();
       processInput(window);
        //FRS
        //------
       double timeDIFF = currentFrame - prevTime;
       couter++;
       if (timeDIFF >= 1.0 / 30) {
           FPS = (1.0 / timeDIFF) * couter;
           prevTime = currentFrame;
           couter = 0;
       }

       // render
       // ------
      //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
       
       
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
       glEnable(GL_DEPTH_TEST);
     
       glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);;
       glm::mat4 view =  camera.GetViewMatrix();
       glm::mat4 model;
       
       glStencilMask(0x00);
       
       

       
       



       
       // be sure to activate shader when setting uniforms/drawing objects
       lightingShader.use();
       lightingShader.setVec3("viewPos", camera.Position);

       lightingShader.setVec3("light.position", camera.Position);
       lightingShader.setVec3("light.direction", camera.Front);
       lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(innerFlashlightCutOff)));
       lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(outerFlashlightCutOff)));
       lightingShader.setInt("material.diffuse", 0);
       //lightingShader.setVec3("material.specular", materialSpecular);
       lightingShader.setInt("material.specular", 1);
       lightingShader.setInt("material.emmision", 2);
       lightingShader.setFloat("material.shininess", 32.0f);

       lightingShader.setVec3("dirLight.direction", glm::vec3(0.2, -1.0, 0.1));
       lightingShader.setVec3("dirLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
       lightingShader.setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
       lightingShader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

       lightingShader.setFloat("light.constant", 1.0f);
       lightingShader.setFloat("light.linear", 0.09f);
       lightingShader.setFloat("light.quadratic", 0.032f);

       lightingShader.setVec3("light.ambient", lightAmbientColor);
       lightingShader.setVec3("light.diffuse", lightDiffusion); // немного затемним рассеянный свет
       lightingShader.setVec3("light.specular", lightSpecular);



       // view/projection transformations


       lightingShader.setMat4("projection", projection);
       lightingShader.setMat4("view", view);

       // world transformation


       glActiveTexture(GL_TEXTURE0);
       glBindTexture(GL_TEXTURE_2D, diffuseMap);
       glActiveTexture(GL_TEXTURE1);
       glBindTexture(GL_TEXTURE_2D, specularMap);
       glActiveTexture(GL_TEXTURE2);
       glBindTexture(GL_TEXTURE_2D, emmisionMap);
       // render the cube
       glBindVertexArray(cubeVAO);

       lightingShader.setInt("toggleSunLight", toggleSunLight);
       //       lampShader.use();

       model = glm::mat4(1.0f);
       model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f)); // смещаем вниз чтобы быть в центре сцены
       model = glm::scale(model, glm::vec3(12.0f, 1.0f, 12.0f));	// объект слишком большой для нашей сцены, поэтому немного уменьшим его
       lightingShader.setMat4("model", model);
     
       glDrawArrays(GL_TRIANGLES, 0, 36);
       
       	// объект слишком большой для нашей сцены, поэтому немного уменьшим его
       
       
       if (!toggleSunLight) {
           glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
           
       }
       else {
           glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
       }


       

       //render external object
       ourShader.use();

       ourShader.setVec3("viewPos", camera.Position);

       ourShader.setVec3("light.position", camera.Position);
       ourShader.setVec3("light.direction", camera.Front);
       ourShader.setFloat("light.cutOff", glm::cos(glm::radians(innerFlashlightCutOff)));
       ourShader.setFloat("light.outerCutOff", glm::cos(glm::radians(outerFlashlightCutOff)));
       ourShader.setInt("material.diffuse", 0);
       //lightingShader.setVec3("material.specular", materialSpecular);
       ourShader.setInt("material.specular", 1);
       ourShader.setInt("material.emmision", 2);
       ourShader.setFloat("material.shininess", 32.0f);

       ourShader.setVec3("dirLight.direction", glm::vec3(0.2, -1.0, 0.1));
       ourShader.setVec3("dirLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
       ourShader.setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
       ourShader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

       ourShader.setFloat("light.constant", 1.0f);
       ourShader.setFloat("light.linear", 0.09f);
       ourShader.setFloat("light.quadratic", 0.032f);

       ourShader.setVec3("light.ambient", lightAmbientColor);
       ourShader.setVec3("light.diffuse", lightDiffusion); // немного затемним рассеянный свет
       ourShader.setVec3("light.specular", lightSpecular);
       ourShader.setInt("toggleSunLight", toggleSunLight);
       ourShader.setMat4("projection", projection);
       
       ourShader.setMat4("view", view);

       for (int i = 0; i < 4; i++) {
           std::string myStringMessage = "pointLights[" + std::to_string(i) + "].";
           const char* someMessage;
           someMessage = (myStringMessage + "constant").c_str();
           ourShader.setFloat(someMessage, 1.0f);


           someMessage = (myStringMessage + "linear").c_str();
           ourShader.setFloat(someMessage, 0.09f);

           someMessage = (myStringMessage + "quadratic").c_str();
           ourShader.setFloat(someMessage, 0.032f);


           someMessage = (myStringMessage + "position").c_str();
           ourShader.setVec3(someMessage, pointLightPositions[i]);


           someMessage = (myStringMessage + "ambient").c_str();
           ourShader.setVec3(someMessage, lightAmbientColor);


           someMessage = (myStringMessage + "diffuse").c_str();
           ourShader.setVec3(someMessage, lightDiffusion);


           someMessage = (myStringMessage + "specular").c_str();
           ourShader.setVec3(someMessage, lightSpecular);
       }

       model = glm::mat4(1.0f);
       model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // смещаем вниз чтобы быть в центре сцены
       model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// объект слишком большой для нашей сцены, поэтому немного уменьшим его
       ourShader.setMat4("model", model);


       


       glStencilFunc(GL_ALWAYS, 1, 0xFF);
       glStencilMask(0xFF);

       ourModel.Draw(ourShader);



       glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

       glStencilMask(0x00);
       glDisable(GL_DEPTH_TEST);
       stencilShader.use();
       stencilShader.setMat4("projection", projection);
       stencilShader.setMat4("view", view);
       model = glm::mat4(1.0f);
       model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // смещаем вниз чтобы быть в центре сцены
       stencilShader.setFloat("outlining", 0.05);
       stencilShader.setMat4("model", model);
       ourModel.Draw(ourShader);
       glStencilMask(0xFF);
       glStencilFunc(GL_ALWAYS, 1, 0xFF);
       glEnable(GL_DEPTH_TEST);

       
      


       lightingShader.use();
       for(int i=0;i<4;i++){
           std::string myStringMessage = "pointLights[" + std::to_string(i) + "].";
           const char* someMessage;
           someMessage = (myStringMessage + "constant").c_str();
       lightingShader.setFloat(someMessage, 1.0f);

        
       someMessage = (myStringMessage + "linear").c_str();
       lightingShader.setFloat(someMessage, 0.09f);

       someMessage = (myStringMessage + "quadratic").c_str();
       lightingShader.setFloat(someMessage, 0.032f);

       
       someMessage = (myStringMessage + "position").c_str();
       lightingShader.setVec3(someMessage, pointLightPositions[i]);

       
       someMessage = (myStringMessage + "ambient").c_str();
       lightingShader.setVec3(someMessage, lightAmbientColor);

       
       someMessage = (myStringMessage + "diffuse").c_str();
       lightingShader.setVec3(someMessage, lightDiffusion);

       
       someMessage = (myStringMessage + "specular").c_str();
       lightingShader.setVec3(someMessage, lightSpecular);
       }
       
       
       
       lampShader.use();
       lampShader.setMat4("projection", projection);
       lampShader.setMat4("view", view);
       glBindVertexArray(lightVAO);
       for (unsigned int i = 0; i < 4; i++)
       {
           
           glm::mat4 model = glm::mat4(1.0f);
           model = glm::translate(model, pointLightPositions[i]);
           model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
           lampShader.setMat4("model", model);

          glDrawArrays(GL_TRIANGLES, 0, 36);
       }

       ReflectedShader.use();
       model = glm::mat4(1.0f);
       model = glm::translate(model, glm::vec3((float)5.0 + glm::sin(glfwGetTime()), 1.0f, 0.0f)); // смещаем вниз чтобы быть в центре сцены
       model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

       ReflectedShader.setMat4("model", model);
       ReflectedShader.setMat4("view", view);
       ReflectedShader.setMat4("projection", projection);
       ReflectedShader.setVec3("cameraPos", camera.Position);

       // Кубы
       glBindVertexArray(cubeVAO);
       
       glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
       
       glDrawArrays(GL_TRIANGLES, 0, 36);
       
       glBindVertexArray(0);
       
       //// Теперь снова привязывемся к фреймбуферу, заданному по умолчанию и отрисовываем прямоугольник с прикрепленной цветовой текстурой фреймбуфера
       //glBindFramebuffer(GL_FRAMEBUFFER, 0);
       //{
       //    glEnable(GL_BLEND);
       //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
       //    glDisable(GL_DEPTH_TEST);
       //   
       //
       //   


       //    frameBufferShader.use();
       //    frameBufferShader.setFloat("offsetNumerator", offsetNumerator);
       //    glBindVertexArray(quadVAO);
       //    glBindTexture(GL_TEXTURE_2D, textureColorbuffer); // используем прикрепленную цветовую текстуру в качестве текстуры для прямоугольника
       //    glDrawArrays(GL_TRIANGLES, 0, 6);
       //}


       glDepthFunc(GL_LEQUAL);
       skyboxShader.use();
       glm::mat4 SkyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
       // We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
       // The last row and column affect the translation of the skybox (which we don't want to affect)
       //view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up)));
       //projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
       glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(SkyboxView));
       glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


       glBindVertexArray(skyboxVAO);
       glActiveTexture(GL_TEXTURE0);
       glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
       glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
       glBindVertexArray(0);
       glDepthFunc(GL_LESS);

       ImGui::Render();
       ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
       
      
       glfwSwapBuffers(window);
       glfwPollEvents();
       
   }
   




   glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        toggleCursor = !toggleCursor;
    }

   

 

}
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}