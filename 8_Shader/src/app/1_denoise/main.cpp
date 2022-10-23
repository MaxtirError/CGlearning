#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"

#include <iostream>
#include<set>
#include "ANN/ANN.h"

using namespace Ubpa;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
gl::Texture2D loadTexture(char const* path);
gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources);

// settings
unsigned int scr_width = 800;
unsigned int scr_height = 600;
float displacement_bias = 0.f;
float displacement_scale = 1.f;
float displacement_lambda = 0.2f;
bool have_denoise = false;

// camera
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW8 - denoise", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    gl::Enable(gl::Capability::DepthTest);

    // build and compile our shader zprogram
    // ------------------------------------
    gl::Shader vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3_denoise.vert");
    gl::Shader fs(gl::ShaderType::FragmentShader, "../data/shaders/light.frag");
    gl::Program program(&vs, &fs);
    rgbf ambient{ 0.2f,0.2f,0.2f };
    program.SetTex("albedo_texture", 0);
    program.SetTex("displacementmap", 1);
    program.SetVecf3("point_light_pos", { 0,5,0 });
    program.SetVecf3("point_light_radiance", { 100,100,100 });
    program.SetVecf3("ambient_irradiance", ambient);
    program.SetFloat("roughness", 0.5f );
    program.SetFloat("metalness", 0.f);

    // load model
    // ------------------------------------------------------------------
    auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj", true);
    // world space positions of our cubes
    pointf3 instancePositions[] = {
        pointf3(0.0f,  0.0f,  0.0f),
        pointf3(2.0f,  5.0f, -15.0f),
        pointf3(-1.5f, -2.2f, -2.5f),
        pointf3(-3.8f, -2.0f, -12.3f),
        pointf3(2.4f, -0.4f, -3.5f),
        pointf3(-1.7f,  3.0f, -7.5f),
        pointf3(1.3f, -2.0f, -2.5f),
        pointf3(1.5f,  2.0f, -2.5f),
        pointf3(1.5f,  0.2f, -1.5f),
        pointf3(-1.3f,  1.0f, -1.5f)
    };

    // load and create a texture 
    // -------------------------
    gl::Texture2D spot_albedo = loadTexture("../data/textures/spot_albedo.png");

    gl::Texture2D displacementmap = genDisplacementmap(spot);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        gl::ClearColor({ ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit); // also clear the depth buffer now!

        program.SetVecf3("camera_pos", camera.Position);

        // bind textures on corresponding texture units
        program.Active(0, &spot_albedo);
        program.Active(1, &displacementmap);

        // pass projection matrix to shader (note that in this case it could change every frame)
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
        program.SetMatf4("projection", projection);

        // camera/view transformation
        program.SetMatf4("view", camera.GetViewMatrix());
        program.SetFloat("displacement_bias", displacement_bias);
        program.SetFloat("displacement_scale", displacement_scale);
        program.SetFloat("displacement_lambda", displacement_lambda);
        program.SetBool("have_denoise", have_denoise);

        // render spots
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            program.SetMatf4("model", model);
            spot->va->Draw(&program);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    delete spot;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        have_denoise = !have_denoise;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    gl::Viewport({ 0, 0 }, width, height);
    scr_width = width;
    scr_height = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

gl::Texture2D loadTexture(char const* path)
{
    gl::Texture2D tex;
    tex.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat, gl::MinFilter::Linear, gl::MagFilter::Linear);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    gl::PixelDataFormat c2f[4] = {
        gl::PixelDataFormat::Red,
        gl::PixelDataFormat::Rg,
        gl::PixelDataFormat::Rgb,
        gl::PixelDataFormat::Rgba
    };
    gl::PixelDataInternalFormat c2if[4] = {
        gl::PixelDataInternalFormat::Red,
        gl::PixelDataInternalFormat::Rg,
        gl::PixelDataInternalFormat::Rgb,
        gl::PixelDataInternalFormat::Rgba
    };
    if (data)
    {
        tex.SetImage(0, c2if[nrChannels - 1], width, height, c2f[nrChannels - 1], gl::PixelDataType::UnsignedByte, data);
        tex.GenerateMipmap();
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return tex;
}

gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources) {
    float* displacementData = new float[1024 * 1024];
    // TODO: HW8 - 1_denoise | genDisplacementmap
    // 1. set displacementData with resources's positions, indices, normals, ...
    // 2. change global variable: displacement_bias, displacement_scale, displacement_lambda

    // ...
    auto& tri = resources->indices;
    auto& pos = resources->positions;
    auto& nor = resources->normals;
    auto& cord = resources->texcoords;
    std::set<int>* Edges = new std::set<int>[pos.size()]; //get edge
    for (int i = 0; i < tri.size(); i += 3)
        for (int k = 0; k < 3; ++k)
            Edges[tri[i + k]].insert(tri[i + (k + 1) % 3]);
    std::vector<int>* Mg_Points = new std::vector<int>[pos.size()];
    for (int i = 0; i < pos.size(); ++i) 
    {
        Mg_Points[i].push_back(i);
        for (int j = 0; j < i; ++j)
            if ((pos[i].cast_to<vecf3>() - pos[j].cast_to<vecf3>()).norm() < 1e-5)
            {
                Mg_Points[i].push_back(j);
                Mg_Points[j].push_back(i);
            }
    }

    float *raw_val = new float[pos.size()];
    for (int i = 0; i < pos.size(); ++i)
    {
        vecf3 delta(0, 0, 0);
        for (auto j : Edges[i])
            delta += pos[j].cast_to<vecf3>();
        delta = pos[i].cast_to<vecf3>() - delta / Edges[i].size();
        raw_val[i] = delta.dot(nor[i].cast_to<vecf3>());
    }

    ANNpointArray dataPts = annAllocPts(1024 * 1024, 2);
    std::vector<float>Vals;
    int nPts = 0;

    float mx = -1e9, mn = 1e9;
    for (int i = 0; i < pos.size(); ++i)
    {
        float val = 0;
        for (auto j : Mg_Points[i])
            val += raw_val[j];
        val /= Mg_Points[i].size();

        dataPts[nPts][0] = int(cord[i][0] * 1024);
        dataPts[nPts++][1] = int(cord[i][1] * 1024);
        Vals.push_back(val);
        mx = std::max(mx, val);
        mn = std::min(mn, val);
    }
    delete[] raw_val;

    displacement_scale = mx - mn;
    displacement_bias = mn;
    displacement_lambda = 0.75;

    const int K = 1;
    ANNidxArray index = new ANNidx[K];
    ANNdistArray dist = new ANNdist[K];
    ANNkd_tree* kdtree = new ANNkd_tree(dataPts, nPts, 2);
    for (int i = 0; i < 1024; i++)
        for (int j = 0; j < 1024; j++)
        {
            float vals = 0;
            ANNpoint pt = annAllocPt(2);
            pt[0] = i; pt[1] = j;
            kdtree->annkSearch(pt, K, index, dist, 0);

            for (int k = 0; k < K; k++)
            {
                int x = dataPts[index[k]][0];
                int y = dataPts[index[k]][1];
                vals += Vals[index[k]];
            }
            displacementData[1024 * j + i] = (vals / K - displacement_bias) / displacement_scale;
        }
    delete[] index;
    delete[] dist;
    delete kdtree;

    gl::Texture2D displacementmap;
    displacementmap.SetImage(0, gl::PixelDataInternalFormat::Red, 1024, 1024, gl::PixelDataFormat::Red, gl::PixelDataType::Float, displacementData);
    displacementmap.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
        gl::MinFilter::Linear, gl::MagFilter::Linear);
    stbi_uc* stbi_data = new stbi_uc[1024 * 1024];
    for (size_t i = 0; i < 1024 * 1024; i++)
        stbi_data[i] = static_cast<stbi_uc>(std::clamp(displacementData[i] * 255.f, 0.f, 255.f));
    stbi_write_png("../data/1_denoise_displacement_map.png", 1024, 1024, 1, stbi_data, 1024);
    delete[] stbi_data;
    delete[] displacementData;
    delete[] Edges;
    delete[] Mg_Points;
    return displacementmap;
}
