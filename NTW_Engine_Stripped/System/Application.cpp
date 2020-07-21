#include <Pch/Pch.h>
#include "Application.h"
#include "MayaCommunicator/MayaViewer.h"
#include <System/MemoryUsage.h>

#define AUTOSTART false;
#define FULLSCREEN false;
#define DISABLE_INPUT false; 

float DeltaTime = 0.0f;
unsigned int Framerate = 0;

Application::Application() 
{
}

Application::~Application() 
{
	delete m_input;
	delete m_stateManager;
	//delete m_noLog;

	ShaderMap::getInstance()->destroy();
	Renderer::getInstance()->destroy();
	MaterialMap::getInstance()->destroy();
	MeshMap::getInstance()->destroy();
	AnimationMap::getInstance()->destroy();
	SkeletonMap::getInstance()->destroy();
	

	glfwTerminate();
}

bool Application::init() 
{
	
	bool statusOK = false;
	initialFrame = false;
	statusOK = glfwInit();

	if (!statusOK) {
		logError("Failed to initialize GLFW!");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);


#if FULLSCREEN
	m_window = glfwCreateWindow(1280, 720, "Maya Viewer Project", glfwGetPrimaryMonitor(), NULL);// !!! FULLSCREEN!!!
#else 
	m_window = glfwCreateWindow(1280, 720, "Maya Viewer Project", NULL, NULL);
#endif

	//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	if (m_window == nullptr) {
		glfwTerminate();
		logError("Failed to create GLFW window");
		return false;
	}

	centerWindowOnMonitor();

	// Opengl context
	glfwMakeContextCurrent(m_window);
	
	GLenum status = glewInit();

	if (status != GLEW_OK) {
		glfwTerminate();
		logError("Failed to initialize GLEW!");
	}
	
	glfwSwapInterval(0); // Turning this off will cause occasionally freezes, so don't!
	
#if DISABLE_INPUT
	//m_input = new Input();
#else
	m_input = new Input();

#endif
	initGraphics();

	m_stateManager = new StateManager();
	//m_stateManager->pushState(new PlayState());	
	m_stateManager->pushState(new MayaViewer());	

	unsigned int _time = unsigned int(time(NULL));
	srand(_time);

	MemoryUsage mu;
	mu.printBoth("End of application init:");

	return statusOK;
}

void Application::run()
{
	float timeNow = 0.0f;
	float timeThen = 0.0f;
	float updateFreq = 1.0f / 60.0f;
	float currentTime = 0.0f;
	logInfo("Running Application loop");

	while (!glfwWindowShouldClose(m_window)) 
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


		#if DISABLE_INPUT
		//m_input->clearKeys();
		#else
		m_input->clearKeys();

		#endif
		
		glfwPollEvents();

		// Quick way to close the app
		if (Input::isKeyReleased(GLFW_KEY_DELETE))
		{
			glfwSetWindowShouldClose(m_window, true);
		}

		if (Input::isKeyPressed(GLFW_KEY_F4)) 
		{
			ShaderMap::getInstance()->reload();
		}

		//Skip the first frame, this is because we 
		if (initialFrame == false) 
		{
			timeNow = static_cast<float>(glfwGetTime());
			timeThen = timeNow;
			initialFrame = true;
			glfwSwapBuffers(m_window);

			continue;
		}

		timeNow = static_cast<float>(glfwGetTime());

		//Deltatime
		DeltaTime = timeNow - timeThen;
		timeThen = timeNow;
		currentTime += DeltaTime;
		calcFPS(DeltaTime);

	
		m_stateManager->Update(DeltaTime);
		m_stateManager->render();
		

		glActiveTexture(GL_TEXTURE0);
		glfwSwapBuffers(m_window);
	}

}

void Application::initGraphics()
{
	//init renderer
	Renderer* m_renderer = Renderer::getInstance();
	if (!m_renderer) 
	{
		logError("Rendererer failed");
	}
	
	m_renderer->init(m_window);

	ShaderMap::getInstance();
}


void Application::centerWindowOnMonitor()
{
	auto monitor = glfwGetPrimaryMonitor();

	if (!monitor)
		return;

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (!mode)
		return;

	int monitorX, monitorY;
	glfwGetMonitorPos(monitor, &monitorX, &monitorY);

	int windowWidth, windowHeight;
	glfwGetWindowSize(m_window, &windowWidth, &windowHeight);

	glfwSetWindowPos(m_window,
		monitorX + (mode->width - windowWidth) / 2,
		monitorY + (mode->height - windowHeight) / 2);
}

void Application::calcFPS(const float& dt)
{
	static unsigned fps = 0;
	static float frameTimer = 1.0f;

	fps++;

	frameTimer -= dt;
	if (frameTimer <= 0.0f)
	{
		frameTimer = 1.0f;
		Framerate = fps;
		fps = 0;
	}


}
