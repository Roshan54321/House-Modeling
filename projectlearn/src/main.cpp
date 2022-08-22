#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <shader.h>
#include <camera.h>
#include <model.h>
#include <Animator.h>


#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.f, 60.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


//paths
const char * vertexShaderPath = 
"/home/baral/Downloads/tests/House_Modeling" //can be changed
"/projectlearn/res/shaders/1.model_loading.vs";
const char * lightingShadervPath = 
"/home/baral/Downloads/tests/House_Modeling"
"/projectlearn/res/shaders/anim.vs";
const char * lightingShaderfPath = 
"/home/baral/Downloads/tests/House_Modeling"
"/projectlearn/res/shaders/anim.fs";
const char * fragmentShaderPath = 
"/home/baral/Downloads/tests/House_Modeling"
"/projectlearn/res/shaders/1.model_loading.fs";
const char * objFilePath = 
"/home/baral/Downloads/tests/House_Modeling"
"/projectlearn/res/models/untitled.dae";



int main()
{
    // initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "House Modeling", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Making the context of our window the main context of the current thread
    glfwMakeContextCurrent(window);

    // configure resize callback function to framebuffer_size_callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // load the address of the OpenGL function pointers which is OS-Specific
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    //configures blend function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // Shader ourShader( vertexShaderPath,fragmentShaderPath );
    Shader lightingShader( lightingShadervPath,lightingShaderfPath );

    // load models
	Model ourModel( objFilePath );
    Animation danceAnimation(objFilePath,&ourModel);
	Animator animator(&danceAnimation);
    
    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glm::vec3 lightPos(0.0f, 200.0f, 100.0f);
    // light properties
    glm::vec3 lightColor;
    lightColor.x = static_cast<float>(1.0f);
    lightColor.y = static_cast<float>(1.0f);
    lightColor.z = static_cast<float>(1.0f);


    //imgui
    const char *glsl_version = "#version 130";
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // main/render loop
    glfwSwapInterval(3);

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);
       


        //-----------------------------

        //imgui
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
         animator.UpdateAnimation(deltaTime);

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable shader before setting uniforms
        // ourShader.use();
        lightingShader.use();
        // lightingShader.setVec3("light.position", lightPos);
        // lightingShader.setVec3("viewPos", camera.Position);

        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.7f); // decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.4f); // low influence
        // lightingShader.setVec3("light.ambient", ambientColor);
        // lightingShader.setVec3("light.diffuse", diffuseColor);
        // lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        // // material properties
        // lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        // lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        // lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
        // lightingShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			lightingShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // ourShader.setMat4("projection", projection);
        // ourShader.setMat4("view", view);
       

      

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.1f, -0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
        // ourShader.setMat4("model", model);
        lightingShader.setMat4("model", model);
        // ourModel.Draw(ourShader);
        ourModel.Draw(lightingShader);

        //imgui
        {
            ImGui::SliderFloat3("LightPos", &lightPos.x, -400.f, 400.f);
            ImGui::SliderFloat3("LightColor", &lightColor.x, 0.0f, 1.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        //-----------------------------

        //swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //imgui
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }

    //terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(CRIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(CLEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
