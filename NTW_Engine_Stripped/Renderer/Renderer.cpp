#include <Pch/Pch.h>
#include "Renderer.h"

Renderer * Renderer::m_rendererInstance = 0;

Renderer::Renderer()
{
	m_gWindow = nullptr;
	m_camera = nullptr;
	glEnable(GL_MULTISAMPLE);
	int x = -10;
	int z = -40;

	//GL related calls
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	initShaders();
}

Renderer::~Renderer()
{

}


void Renderer::createDepthMap() 
{
	//Create a depth map texture for the rendering system
	glGenFramebuffers(1, &m_depthFBO);

	glGenTextures(1, &m_depthMap);
	glBindTexture(GL_TEXTURE_2D, m_depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1, 1, 1, 1 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, m_depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Renderer::initShaders() 
{
	ShaderMap* shaderMap = ShaderMap::getInstance();
	Shader* shader;
	shaderMap->createShader(DEPTH_MAP, "Depth.vert", "Depth.frag");
	shaderMap->createShader(BASIC_FORWARD, "VertexShader.vert", "FragShader.frag");
	shaderMap->createShader(TRANSPARENCY, "TransparentRender.vert", "TransparentRender.frag");
	shaderMap->createShader(SKYBOX, "Skybox.vs", "Skybox.fs");
	shaderMap->getShader(SKYBOX)->setInt("skyBox", 4);
}

void Renderer::bindMatrixes(const std::string& shaderName) 
{
	ShaderMap::getInstance()->getShader(shaderName)->setMat4("viewMatrix", m_camera->getViewMat());
	ShaderMap::getInstance()->getShader(shaderName)->setMat4("projMatrix", m_camera->getProjMat());
}

void Renderer::bindMatrixes(Shader* shader)
{
	shader->setMat4("viewMatrix", m_camera->getViewMat());
	shader->setMat4("projMatrix", m_camera->getProjMat());
}

Renderer* Renderer::getInstance()
{
	if (m_rendererInstance == 0) {
		m_rendererInstance = new Renderer();
	}
	return m_rendererInstance;
}

void Renderer::init(GLFWwindow* window)
{
	m_gWindow = window;
}

void Renderer::setupCamera(Camera* camera)
{
	if (camera == nullptr) {
		return;
	}
	m_camera = camera;
}

void Renderer::submit(GameObject* gameObject, RENDER_TYPE objType)
{
	if (objType == RENDER_TYPE::STATIC)
	{
		m_staticObjects.emplace_back(gameObject);
	}
	
	else if (objType == RENDER_TYPE::DYNAMIC) {
		m_dynamicObjects.emplace_back(gameObject);
	}
	else if (objType == RENDER_TYPE::ANIMATEDSTATIC) {
		m_anistaticObjects.emplace_back(gameObject);
	}
	else if (objType == RENDER_TYPE::ANIMATEDDYNAMIC) {
		m_anidynamicObjects.emplace_back(gameObject);
	}
	else if (objType == RENDER_TYPE::SKYOBJECTS) {
		m_skyObjects.emplace_back(gameObject);
	}
	else if (objType == RENDER_TYPE::POINTLIGHT_SOURCE) {
		/* Place the light in the lights list */
		Pointlight* lightRef = static_cast<Pointlight*>(gameObject);

		PLIGHT light;
		light.position = gameObject->getObjectTransform().position;
		light.color = lightRef->getColor();
		light.attenAndRadius = lightRef->getAttenuationAndRadius(); //First 3 dims are for the attenuation, final 4th is for radius
		light.strength = lightRef->getStrength();
		light.index = -2;

		m_lights.emplace_back(light);
	}
}

void Renderer::submitSkybox(SkyBox* skybox)
{
	m_skyBox = skybox;
}


void Renderer::clear() 
{
	m_staticObjects.clear();
	m_dynamicObjects.clear();
	m_anistaticObjects.clear();
	m_anidynamicObjects.clear();
	m_lights.clear();
	m_skyObjects.clear();
}

void Renderer::removeRenderObject(GameObject* gameObject, RENDER_TYPE objType)
{
	int index = -1;

	if (objType == RENDER_TYPE::DYNAMIC) { //Remove dynamic objet from the dynamic objet vector
		//Find the index of the object
		for (size_t i = 0; i < m_dynamicObjects.size(); i++)
		{
			if (m_dynamicObjects[i] == gameObject) 
			{
				index = i;
				break;
			}
		}
		if (index > -1) {
			m_dynamicObjects.erase(m_dynamicObjects.begin() + index);
		}
	}
	else if (objType == STATIC) 
	{
		//Find the index of the object
		for (size_t i = 0; i < m_staticObjects.size(); i++)
		{
			if (m_staticObjects[i] == gameObject) {
				index = i;
				break;
			}
		}
		if (index > -1) {
			m_staticObjects.erase(m_staticObjects.begin() + index);
		}
	}
	else if (objType == RENDER_TYPE::ANIMATEDSTATIC) { //remove PICKUP from the spell PICKUP!!
	   //Find the index of the object
		for (size_t i = 0; i < m_anistaticObjects.size(); i++)
		{
			if (m_anistaticObjects[i] == gameObject) 
			{
				index = i;
				break;
			}
		}
		if (index > -1) 
		{
			m_anistaticObjects.erase(m_anistaticObjects.begin() + index);
		}
	}
}

void Renderer::destroy()
{
	delete m_rendererInstance;
	m_rendererInstance = nullptr;
}

void Renderer::renderSkybox()
{
	glDisable(GL_CULL_FACE);

	//glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	auto* shader = ShaderMap::getInstance()->useByName("Skybox_Shader");
	shader->setMat4("modelMatrix", m_skyBox->getModelMatrix());
	shader->setMat4("viewMatrix", glm::mat4(glm::mat3(m_camera->getViewMat())));
	shader->setMat4("projMatrix", m_camera->getProjMat());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyBox->getCubeMapTexture());
	glBindVertexArray(m_skyBox->getVAO());
	glDrawArrays(GL_TRIANGLES, 0, 36); //Maybe index the skybox?
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, NULL);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

void Renderer::renderDepthmap() 
{
	Shader* shader = ShaderMap::getInstance()->useByName(DEPTH_MAP);
	glm::mat4 modelMatrix;
	Mesh* mesh = nullptr;
	//Bind and draw the objects to the depth-buffer
	bindMatrixes(shader);
	glBindFramebuffer(GL_FRAMEBUFFER, m_depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//Loop through all of the static Gameobjects
	for (GameObject* object : m_staticObjects)
	{
		if (object == nullptr) {
			continue;
		}

		if (!object->getShouldRender()) {
			continue;
		}

		//Then through all of the meshes
		for (int j = 0; j < object->getMeshesCount(); j++)
		{
			glEnableVertexAttribArray(0);
			modelMatrix = glm::mat4(1.0f);
			//Fetch the current mesh and its transform
			mesh = object->getMesh(j);

			modelMatrix = object->getMatrix(j);

			glBindVertexArray(mesh->getBuffers().vao);

			//Bind the modelmatrix
			shader->setMat4("modelMatrix", modelMatrix);

			glDrawElements(GL_TRIANGLES, mesh->getBuffers().nrOfFaces * 3, GL_UNSIGNED_INT, NULL);

			glBindVertexArray(0);
			glDisableVertexAttribArray(0);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_renderedDepthmap = true;
}

void Renderer::render() {
	Mesh* mesh = nullptr;
	Transform transform;
	glm::mat4 modelMatrix;
	Shader* shader = nullptr;
	MeshMap* meshMap = MeshMap::getInstance();
	ShaderMap* shaderMap = ShaderMap::getInstance();
	Material* material = nullptr;
	//We always assume that we haven't rendered a depthmap
	m_renderedDepthmap = false; //This is set to true in renderDepthmap()


#if FORWARDPLUS	
	if (m_lights.size() > 0) {
		renderDepthmap();
	}
#endif

#if SSAO
	if (!m_renderedDepthmap) {
		renderDepthmap();
	}
#endif

#if FORWARDPLUS
	//Light Culling from the compute shader
	if (m_lights.size() > 0) {
		shader = shaderMap->useByName(LIGHT_CULL);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightIndexSSBO);
		bindMatrixes(shader);

		glm::vec2 screenSize = glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
		shader->setVec2("screenSize", screenSize);
		shader->setInt("lightCount", m_lights.size());//Set the number of active pointlights in the scene

		//Bind the depthmap
		glActiveTexture(GL_TEXTURE0);
		shader->setInt("depthMap", 0);
		glBindTexture(GL_TEXTURE_2D, m_depthMap);

		//Send all of the light data into the compute shader
		for (size_t i = 0; i < m_lights.size(); i++) {
			shader->setVec3("lights[" + std::to_string(i) + "].position", m_lights[i].position);
			shader->setFloat("lights[" + std::to_string(i) + "].radius", m_lights[i].attenAndRadius.w);
		}

		glDispatchCompute(workGroups.x, workGroups.y, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		//Unbind the depth & noise
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
#endif

#if SSAO
	//Here we dispatch the SSAO compute shader
	shader = shaderMap->useByName(SSAO_RAW);
	glActiveTexture(GL_TEXTURE0);
	//bindMatrixes(shader);	//Bind view and projection matrix
	glBindTexture(GL_TEXTURE_2D, m_depthMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTexture);

	for (size_t i = 0; i < m_SSAOKernels.size(); i++)
	{
		//Assign the kernels
		shader->setVec3("kernels[" + std::to_string(i) + "]", m_SSAOKernels[i]);
	}

	glBindImageTexture(0, m_SSAOColourBuffer, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); //Bind the image unit

	glDispatchCompute(workGroups.x, workGroups.y, 1);
	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
	glBindTexture(GL_TEXTURE_2D, 0);
#if N_BLUR //Naive blurring
	shader = shaderMap->useByName(NAIVE_BLUR);
	glBindImageTexture(0, m_SSAOColourBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glDispatchCompute(workGroups.x, workGroups.y, 1);
	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
#endif
#if BLUR //More efficiant blurring
	//Blur Horizontally
	shader = shaderMap->useByName(H_BLUR);
	glBindImageTexture(0, m_SSAOColourBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); //Bind the image unit
	glDispatchCompute(2, 1, 1); //Dispatch 2 - xAxis workgroups
	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Blur Vertically
	shader = shaderMap->useByName(V_BLUR);
	glBindImageTexture(0, m_SSAOColourBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); //Bind the image unit
	glDispatchCompute(1, 1, 1); //Because of our resolution we can comfortably dipatch one group of threads
	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
	glBindTexture(GL_TEXTURE_2D, 0);
#endif

#endif

#pragma region Color_Render
	shader = shaderMap->useByName(BASIC_FORWARD);
	shader->clearBinding();

	//Bind view- and projection matrix
	bindMatrixes(shader);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightIndexSSBO);
	shader->setVec3("CameraPosition", m_camera->getCamPos());
	//Add a step where we insert lights into the scene

	shader->setInt("LightCount", m_lights.size());
	shader->setInt("SSAO", 0);

	if (m_lights.size() > 0) 
	{
		std::string iConv = "";
		for (size_t i = 0; i < m_lights.size(); i++) {

			iConv = std::to_string(i);

			shader->setVec3("pLights[" + std::to_string(i) + "].position", m_lights[i].position);
			shader->setVec3("pLights[" + iConv + "].color", m_lights[i].color);
			shader->setVec4("pLights[" + iConv + "].attenAndRadius", m_lights[i].attenAndRadius);
			shader->setFloat("pLights[" + iConv + "].strength", m_lights[i].strength);
		}
	}

#if SSAO 
	shader->setInt("SSAO", 1);
	glBindImageTexture(0, m_SSAOColourBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
#endif

	//Render Static objects
	for (GameObject* object : m_staticObjects)
	{
		if (object == nullptr) 
		{
			continue;
		}

		if (!object->getShouldRender()) 
		{
			continue;
		}
	
		//Then through all of the meshes
		for (int j = 0; j < object->getMeshesCount(); j++)
		{
			//Fetch the current mesh and its transform
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			mesh = object->getMesh(j);			

			//Bind the material
			object->bindMaterialToShader(shader, j);

			//Bind the modelmatrix
			modelMatrix = object->getMatrix(j);
			shader->setMat4("modelMatrix", modelMatrix);

			glBindVertexArray(mesh->getBuffers().vao);
			glDrawElements(GL_TRIANGLES, mesh->getBuffers().nrOfFaces * 3, GL_UNSIGNED_INT, NULL);

			glBindVertexArray(0);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);			
		}
	}

	shader->setInt("SSAO", 0);
	shader->clearBinding();
	//Dynamic objects
	if (m_dynamicObjects.size() > 0) {
		for (GameObject* object : m_dynamicObjects)
		{
			if (object == nullptr) {
				continue;
			}

			if (!object->getShouldRender()) {
				continue;
			}

			//Then through all of the meshes
			for (int j = 0; j < object->getMeshesCount(); j++)
			{
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				glEnableVertexAttribArray(3);
				mesh = object->getMesh(j);
				//Bind the material
				object->bindMaterialToShader(shader, j);

				//Bind the modelmatrix
				modelMatrix = object->getMatrix(j);
				shader->setMat4("modelMatrix", modelMatrix);

				glBindVertexArray(mesh->getBuffers().vao);
				glDrawElements(GL_TRIANGLES, mesh->getBuffers().nrOfFaces * 3, GL_UNSIGNED_INT, NULL);

				glBindVertexArray(0);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
				glDisableVertexAttribArray(3);
			}
		}
	}
	shader->clearBinding();


	if (m_skyObjects.size() > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shader = shaderMap->useByName(TRANSPARENCY);
		bindMatrixes(shader);
		//Render objects 
		float rotValue = 0.0f;

		for (GameObject* object : m_skyObjects)
		{
			if (object == nullptr)
				continue;
			if (!object->getShouldRender())
				continue;

			rotValue -= 0.3f;

			for (int j = 0; j < object->getMeshesCount(); j++)
			{
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				mesh = object->getMesh(j);

				material = object->getMaterial(j);
				object->bindMaterialToShader(shader, j);

				modelMatrix = glm::mat4(1.0f); //<--- Change this line to apply rotation
				modelMatrix = object->getMatrix(j);


				modelMatrix = glm::rotate(modelMatrix, (float)glfwGetTime() * rotValue, glm::vec3(0.0f, 1.0f, 0.0f));
				shader->setMat4("modelMatrix", modelMatrix);

				glBindVertexArray(mesh->getBuffers().vao);
				glDrawElements(GL_TRIANGLES, mesh->getBuffers().nrOfFaces * 3, GL_UNSIGNED_INT, NULL);

				glBindVertexArray(0);
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
			}
		}
		shader->clearBinding();
	}
#pragma endregion


	//renderSkybox();
	// Spell Rendering

#ifdef DEBUG_WIREFRAME
	// DEBUG (MOSTLY FOR DSTR)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
	
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

Camera* Renderer::getMainCamera() const
{
	return m_camera;
}