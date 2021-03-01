#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include "Shader.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION    
#include "stb_image.h"

#include <vector>
#include <chrono>
#include <iostream>
#include <random>

class Physarum
{

public:

	void run()
	{
		initWindow();
		//initPoints();
		//renderPoints();
		initSSBO();
		renderPoints();
		renderLoop();
		clearResources();
	}

private:

	// Constants.
	const GLuint width = 800, height = 450;

	// Globals
	GLFWwindow* window;

	// User interaction.
	// OpenGL is right handed so the last component corresponds to move forward/backwards.
	const float SPEED = 5.0f;
	glm::vec3 camPos = glm::vec3(0.0f);
	glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	// Mouse rotation globals.
	bool firstMouse = true, clicked = false;
	float lastX = (float)(width) / 2.0F, lastY = (float)(height), yaw = -90.0f, pitch = 0.0f;

	// The current time since we started rendering.
	float time = 0.0f;

	// How much time between frames.
	float deltaTime = 0.0f, lastFrame = 0.0f;

	// Geometry.
	const GLuint dimX = 50u, dimY = dimX, dimZ = dimX, numberOfPoints = dimX * dimY * dimZ;

	// Yes I have masochistic tendencies. 
	//std::vector<glm::vec3> points;
	glm::vec4* positions;
	glm::vec4* velocities;

	// FBOs
	GLuint FBOA, FBOB;

	// Buffer objects
	GLuint VBO, VAO;

	// SSBOs.
	GLuint posSSBO, velSSBO;

	// Shaders.
	Shader* shaderA;
	Shader* shaderC;

	// GUI.
	float distance = 0.03f;
	bool GUIHovered;

	void initWindow()
	{

		// Initialize GLFW.
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// GLFW Window.
		window = glfwCreateWindow((int)width, (int)height, "Physarum", NULL, NULL);
		if (window == nullptr)
		{

			throw::std::exception("Failed to create the GLFW window.");
			glfwTerminate();

		}
		glfwSetWindowUserPointer(window, this);
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
		glfwSetMouseButtonCallback(window, mouseClickCallBack);
		glfwSetCursorPosCallback(window, mouseCallback);
		// https://stackoverflow.com/questions/4431637/hiding-mouse-cursor-with-glfw
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Load our OpenGL function pointers.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{

			throw::std::exception("Failed to initialize GLAD");

		}

		// Set dimensions of the OpenGL viewport.
		glViewport(0, 0, width, height);

		// Load ImGUI.
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 440");

		// Setup Style
		ImGui::StyleColorsDark();

	}

	/*void initFBO(GLuint FBO)
	{

		// Create a colour attachment texture.
		GLuint textureColourBuffer;
		glGenTextures(1, &textureColourBuffer);
		glBindTexture(GL_TEXTURE_2D, textureColourBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, 0, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourBuffer, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			throw::std::runtime_error("Framebuffer is not complete!");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}*/

	void initSSBO()
	{

		/* Positions ShaderStorageBufferObject Start. */

		glGenBuffers(1, &posSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfPoints * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);

		GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

		positions = (glm::vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numberOfPoints * sizeof(glm::vec4), bufMask);

		// Not sure if this does it the SIMD way but I am way too used to write shaders.
		glm::vec3 dim = glm::vec3(dimX, dimY, dimZ) * .5f;
		glm::vec3 negDim = -dim;

		int idx = 0;

		for (int16_t z = (int16_t)negDim.z; z < (int16_t)dim.z; ++z)
		{

			for (int16_t y = (int16_t)negDim.y; y < (int16_t)dim.y; ++y)
			{

				for (int16_t x = (int16_t)negDim.x; x < (int16_t)dim.x; ++x)
				{

					positions[idx] = glm::vec4(x, y, z, 0.0f) * 0.03f;
					//std::cout << "x: " + std::to_string(x) << std::endl;

					idx++;

				}

			}

		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		/* Positions ShaderStorageBufferObject End. */

		/* Velocities ShaderStorageBufferObject Start. */

		glGenBuffers(1, &velSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfPoints * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);

		velocities = (glm::vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numberOfPoints * sizeof(glm::vec4), bufMask);

		/*idx = 0;

		for (int16_t z = (int16_t)negDim.z; z < (int16_t)dim.z; ++z)
		{

			for (int16_t y = (int16_t)negDim.y; y < (int16_t)dim.y; ++y)
			{

				for (int16_t x = (int16_t)negDim.x; x < (int16_t)dim.x; ++x)
				{

					velocities[idx] = glm::ballRand(glm::vec3(1.0f));// glm::vec3(x, y, z) * 0.03f;
					//std::cout << "x: " + std::to_string(x) << std::endl;

					idx++;

				}

			}

		}*/

		// Let's flatten this:

		for (int16_t i = 0; i < numberOfPoints; ++i)
		{
			velocities[i] = glm::vec4(glm::vec3(Rand()), 1.0f);// glm::vec3(x, y, z) * 0.03f;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		/* Velocities ShaderStorageBufferObject End. */

	}

	void renderLoop()
	{

		shaderA = &Shader("Vert.vert", "Frag.frag", nullptr, false);
		shaderA->use();

		// No need to compute this every frame as the FOV stays always the same.
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height,
			0.1f, 100.0f
		);
		shaderA->setMat4("projection", projection);

		glm::mat4 model = glm::mat4(1.0f);
		shaderA->setMat4("model", model);

		shaderC = &Shader(nullptr, nullptr, "Comp.comp", true);
		/*shaderC->bindBufferBase(0, posSSBO);
		shaderC->bindBufferBase(1, velSSBO);
		*/

		while (!glfwWindowShouldClose(window))
		{

			// Calculate the time between frames.
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			deltaTime = time - lastFrame;
			lastFrame = time;

			shaderC->bindBufferBase(0, posSSBO);
			shaderC->bindBufferBase(1, velSSBO);
			//shaderC->setFloat("iTimeDelta", time);
			shaderC->compute(numberOfPoints / 1000, 1, 1);

			shaderA->use();
			shaderA->setFloat("iTime", time);

			glClearColor(0.2f, 0.3f, distance, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);


			glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
			glBindVertexArray(VAO);
			glDrawArrays(GL_POINTS, 0, numberOfPoints);

			// Create the camera (eye).
			glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
			shaderA->setMat4("view", view);

			// User interaction.
			processInput(window);

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("GUI");

			ImGui::SliderFloat("Size of Mouse Painter", &distance, 0.0f, 1.0f);

			// Don't receive for anything different than the UI when inside the UI.
			GUIHovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered();

			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
			glfwPollEvents();

			std::cout << "Delta time: " + std::to_string(deltaTime * 1000.0f) << std::endl;

		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwTerminate();

	}

	void initPoints()
	{

		//points.resize(numberOfPoints);
		positions = new glm::vec4[numberOfPoints];

		// Not sure if this does it the SIMD way but I am way too used to write shaders.
		glm::vec3 dim = glm::vec3(dimX, dimY, dimZ) * .5f;
		glm::vec3 negDim = -dim;

		int idx = 0;

		for (int16_t z = (int16_t)negDim.z; z < (int16_t)dim.z; ++z)
		{

			for (int16_t y = (int16_t)negDim.y; y < (int16_t)dim.y; ++y)
			{

				for (int16_t x = (int16_t)negDim.x; x < (int16_t)dim.x; ++x)
				{

					positions[idx] = glm::vec4(x, y, z, 0.0f) * 0.03f;
					//std::cout << "x: " + std::to_string(x) << std::endl;

					idx++;

				}

			}

		}

		/* // Show the interval of the points.
		std::cout << "X = [" + std::to_string(negDim.x) + ", " + std::to_string(dim.x) + "]" << std::endl;
		std::cout << "Y = [" + std::to_string(negDim.y) + ", " + std::to_string(dim.y) + "]" << std::endl;
		std::cout << "Z = [" + std::to_string(negDim.z) + ", " + std::to_string(dim.z) + "]" << std::endl;
		*/
	}

	void workGroupSize()
	{



	}

	void renderPoints()
	{

		if (positions != nullptr)//(!points.empty())
		{

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
			glBufferData(GL_ARRAY_BUFFER, /*points.size()*/ numberOfPoints * sizeof(glm::vec4), /*&points.front()*/ positions, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

		}

		else
		{

			//std::cout << "NO" << std::endl;
			throw::std::runtime_error("The point matrix has not been initialized!");

		}

	}

	void clearResources()
	{

		//freePositions();
		//freeVelocities();
		
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	void freePositions()
	{

		delete positions;

	}

	void freeVelocities()
	{

		delete velocities;

	}

	/* Callbacks Start. */

	// https://discourse.glfw.org/t/what-is-a-possible-use-of-glfwgetwindowuserpointer/1294
	
	// Get and resize according to user input.
	static void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
	{

		// Get a handle of the instance of this class so that GLFW is able to do 
		// callbacks to the appropiate GLFW window.
		void* app = reinterpret_cast<Physarum*>(glfwGetWindowUserPointer(window));

		glViewport(0, 0, width, height);

	}

	void processInput(GLFWwindow* window)
	{

		float camSpeed = deltaTime * SPEED;

		// Close the app when pressing the ESC key.
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{

			glfwSetWindowShouldClose(window, true);

		}

		// To keep everything frame rate independent the "tick" is used.

		// Move forward. Simple vector addition every scalar in camPos added to every scalar in camFront
		// "weighted" by the deltaTime to abstract the frame dependency.
		//                 * B
		//                 ^^
		//                 | \  camFront
		//         camPos  |  \
		//                 |   ^ 
		//                 |  /
		//                 | /  camPosOld 
		//                 |/
		//                 * A
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{

			camPos += camSpeed * camFront;

		}

		// Same as forward but doing the opposite operation. This 
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{

			camPos -= camSpeed * camFront;

		}

		//                 ^
		//          camUp  |  ^
		//               --| / camFront
		//               | |/
		//     side  <------
		// The cross product gives a vector orthogonal to to other vectors, so the cross product of camFront
		// and camUp gives us a vector pointing to the "side" of the player, multiplying by deltaTime to make
		// sure we don't rely on the frame rate. If we apply the same knowledge of vector addition and
		// subtraction we are able to use this vector to move in that direction.
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{

			camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;

		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{

			camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;

		}

	}

	static void mouseCallback(GLFWwindow* window, double xPos, double yPos)
	{

		auto app = reinterpret_cast<Physarum*>(glfwGetWindowUserPointer(window));

		if (app->GUIHovered != true)
		{

			// Shameless copy from learnopengl.com
			// TODO:
			// Add quaternion support!
			if (app->firstMouse || !app->clicked)
			{
				app->lastX = xPos;
				app->lastY = yPos;
				app->firstMouse = false;
			}

			float xoffset = xPos - app->lastX;
			float yoffset = app->lastY - yPos; // reversed since y-coordinates go from bottom to top
			app->lastX = xPos;
			app->lastY = yPos;

			float sensitivity = 0.1f; // Who doesn't like magic values?
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			app->yaw += xoffset;
			app->pitch += yoffset;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (app->pitch > 89.0f)
				app->pitch = 89.0f;
			if (app->pitch < -89.0f)
				app->pitch = -89.0f;

			// Kill me math wizards, or lock me then Gimbal...
			glm::vec3 front;
			front.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
			front.y = sin(glm::radians(app->pitch));
			front.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
			app->camFront = glm::normalize(front);

		}

	}

	static void mouseClickCallBack(GLFWwindow* window, int button, int action, int mods)
	{

		auto app = reinterpret_cast<Physarum*>(glfwGetWindowUserPointer(window));

		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{

			if (action == GLFW_PRESS && !(app->clicked))
			{

				app->clicked = true;

			}

			else if (action == GLFW_RELEASE && (app->clicked))
			{

				app->clicked = false;

			}

		}

	}

	/* Callbacks End. */

	/* Start of Utilities. */

	// https://stackoverflow.com/questions/686353/random-float-number-generation
	float Rand()
	{
		static std::default_random_engine e;
		static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
		return dis(e);
	}

	/* End of Utilities */

};

int main()
{

	Physarum physarum;

	try
	{

		physarum.run();

	}

	catch (const std::exception& e)
	{

		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;

}