#include "GameRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb_image.h>

#include "Entity.h"
#include "EntityFoo.h"
#include "globals.h"
#include "FMath.h"
#include "Physics/PhysicsWorld.h"
#include "Pathfinder.h"
#include "Scene.h"
#include "Portal.h"
#include "StandardEntityComponents.h"

GameRenderer::GameRenderer()
{
	stbi_set_flip_vertically_on_load(true);
	texture = generateTexture("C:/Users/Sorenon/Crumble/src/main/resources/grass.png");
	textureArrow = generateTexture("C:/Users/Sorenon/Crumble/src/main/resources/Arrow.png");
}


GameRenderer::~GameRenderer()
{
	//TODO: clean up gl objects
}

void GameRenderer::doRender(float t)
{
	{
		auto& trans = registry.get<components::transform>(localplayer);
		auto& rb = registry.get<components::kinematic_ridgedbody>(localplayer);

		viewMatrix = glm::mat4(1.0f);
		viewMatrix = viewMatrix * glm::toMat4(FMath::createQuaternion(trans.rotation));
		viewMatrix = glm::translate(viewMatrix, -(trans.getInterpPos(t) + glm::vec3(0, rb.eyeHeight, 0)));

		projMatrix = glm::perspective(glm::radians(70.0f), (float)wWidth / (float)wHeight, 0.05f, renderDistance * 16 * (float)sqrt(2));
	}

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	texturedProgram.activate();

	renderScene(t);

	for (Portal& portal : scene.portals)
	{
		renderPortal(portal, t);
	}

	renderUI(t);
}

void GameRenderer::renderScene(float t)
{
	updateWorld(&scene.mainWorld);
	for (SubWorld& subWorld : scene.subWorlds)
	{
		updateWorld(&subWorld);
	}

	texturedProgram.activate();

	glUniformMatrix4fv(texturedProgram.projID, 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniformMatrix4fv(texturedProgram.viewID, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	renderWorld(scene.mainWorld);
	for (SubWorld& subWorld : scene.subWorlds)
	{
		renderWorld(subWorld);
	}
	renderEntities(t);

	debugDrawBulletDebug();
}

void GameRenderer::renderPortal(Portal& portal, float t)
{
	{//Render portal
		glEnable(GL_STENCIL_TEST);

		renderPortalStencil(portal);

		glm::mat4 oldViewMatrix = viewMatrix;

		viewMatrix = glm::translate(viewMatrix, -portal.getExit() + portal.getPosition());//Get the difference from the output of the portal and the stencil quad to find the proper perspective
		{
			texturedProgram.activate();

			glUniform4fv(texturedProgram.clipPlaneID, 1, glm::value_ptr(portal.getPlane().asVector()));
			//glUniform4fv(texturedProgram.clipPlaneID, 1, glm::value_ptr(glm::vec4(0.0f, -1.0f, 0.0f, 80.0f)));
			glEnable(GL_CLIP_DISTANCE0);

			renderScene(t);//TODO: possibly render this to a framebuffer to allow for aftereffects

			glDisable(GL_CLIP_DISTANCE0);
		}

		viewMatrix = oldViewMatrix;

		glDisable(GL_STENCIL_TEST);
	}
}

void GameRenderer::renderPortalStencil(Portal& portal)
{
	glEnable(GL_DEPTH_CLAMP);//Allows the camera to be right next to the plane and still it

	glStencilFunc(GL_ALWAYS, 0x01, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);

	texColourProgram.activate();
	glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(projMatrix));
	glUniformMatrix4fv(texColourProgram.viewID, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(glm::scale(projMatrix, glm::vec3(0.999f, 0.999f, 0.999f))));//Emulating a slightly lower FOV (this allows for the quad to be drawn over fragments it would usually zfight with)

	glUniform4f(colourIDTexCol, 0.2f, 0.3f, 0.3f, 1.0f);
	//glUniform4f(colourIDTexCol, 1, 1, 0.3f, 1.0f);

	{
		glClear(GL_STENCIL_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(portal.getVAO().id);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, portal.getPosition());
		model = model * glm::toMat4(portal.getFacing().angle);

		glUniformMatrix4fv(texColourProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, portal.getVAO().count);

		if (renderPortalDebugOutline)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform4f(colourIDTexCol, 0, 0, 0, 1);
			glLineWidth(5);

			glDrawArrays(GL_TRIANGLES, 0, portal.getVAO().count);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	glStencilFunc(GL_EQUAL, 0x01, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0x00);

	{//This has to be done after the stencil because glDepthFunc(GL_ALWAYS) would allow the quad to be seen through walls
		glDepthFunc(GL_ALWAYS);
		glDepthRange(1, 1); //Clear the depth buffer where the quad is drawn by making all the fragments be drawn at gl_fragDepth=1
		glColorMask(false, false, false, false);

		glDrawArrays(GL_TRIANGLES, 0, portal.getVAO().count);//Uniforms and texture is already set

		glDepthFunc(GL_LEQUAL);
		glDepthRange(0, 1);
		glColorMask(true, true, true, true);
	}

	glDisable(GL_DEPTH_CLAMP);

	glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(projMatrix));
}

void GameRenderer::updateWorld(World* world)
{//Remake VAOs
	for (auto pair : world->chunks)
	{
		Chunk& chunk = *pair.second;
		chunkID id = pair.first;
		int x = id >> 32;
		int z = (int)id;

		if (chunk.needsUpdate)
		{
			//std::cout << "Updating Chunk " << ++chunksUpdated << std::endl;

			for (int i = 0; i < 16; i++)
			{
				SubChunk* subChunk = chunk.subChunks[i];
				if (subChunk != nullptr && subChunk->needsUpdate)
				{

					SubChunk& above = i + 1 >= 16 ? SubChunk::EMPTY : chunk.getSubChunkSafe(i + 1);
					SubChunk& below = i - 1 < 0 ? SubChunk::EMPTY : chunk.getSubChunkSafe(i - 1);

					SubChunk& right = world->getChunkSafe(x + 1, z).getSubChunkSafe(i);
					SubChunk& left = world->getChunkSafe(x - 1, z).getSubChunkSafe(i);

					SubChunk& front = world->getChunkSafe(x, z - 1).getSubChunkSafe(i);
					SubChunk& back = world->getChunkSafe(x, z + 1).getSubChunkSafe(i);

					t_VAO oldVAO = chunk.subChunkVAOs[i];
					if (oldVAO.id != 0)
					{
						glDeleteVertexArrays(1, &oldVAO.id);
						glDeleteBuffers(1, &oldVAO.VBO);
					}

					chunk.subChunkVAOs[i] = createSubChunkVAO(*subChunk, above, below, right, left, front, back);

					subChunk->needsUpdate = false;
				}
			}
			chunk.needsUpdate = false;
		}
	}
}

void GameRenderer::renderWorld(World& world)
{
	for (auto pair : world.chunks)
	{
		Chunk& chunk = *pair.second;
		chunkID id = pair.first;
		int x = id >> 32;
		int z = (int)id;

		for (int i = 0; i < 16; i++)
		{
			t_VAO& vao = chunk.subChunkVAOs[i];

			if (vao.id != 0)
			{
				glBindVertexArray(vao.id);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);

				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(x * 16, i * 16, z * 16));

				glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_TRIANGLES, 0, vao.count);
			}
		}
	}
}

void GameRenderer::renderWorld(SubWorld& world)
{
	for (auto pair : world.chunks)
	{
		Chunk& chunk = *pair.second;
		chunkID id = pair.first;
		int x = id >> 32;
		int z = (int)id;

		for (int i = 0; i < 16; i++)
		{
			t_VAO& vao = chunk.subChunkVAOs[i];

			if (vao.id != 0)
			{
				glBindVertexArray(vao.id);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);

				glm::mat4 model = glm::mat4(1.0f);
				{
					model = glm::translate(model, world.offset);
					model = model * glm::toMat4(glm::quat(world.rotation));
					model = glm::translate(model, glm::vec3(x * 16, i * 16, z * 16));
					model = glm::translate(model, -world.centerOfMassOffset);
				}

				glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_TRIANGLES, 0, vao.count);
			}
		}
	}
}

void GameRenderer::renderEntities(float t)
{
	{//Render bullet object
		btTransform trans;
		p_physicsWorld->m_rbCube->getMotionState()->getWorldTransform(trans);

		btVector3 transOrigin = trans.getOrigin();

		glBindVertexArray(cubeVAO.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glm::mat4 model = glm::mat4(1.0f);
		trans.getOpenGLMatrix(glm::value_ptr(model));
		model = glm::translate(model, -glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

			//glDrawArrays(GL_TRIANGLES, 0, cubeVAO.count);
	}

	auto view = registry.view<components::transform, components::renderable>();
	for (auto entity : view)
	{
		auto& trans = view.get<components::transform>(entity);

		glBindVertexArray(cubeVAO.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, trans.getInterpPos(t));

		model = glm::rotate(model, trans.rotation.x, Vectors::RIGHT);
		model = glm::rotate(model, trans.rotation.y, Vectors::UP);
		model = glm::translate(model, -glm::vec3(0.5f, 0, 0.5f));

		glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, cubeVAO.count);
	}
}

void GameRenderer::renderUI(float t)
{
	texColourProgram.activate();

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	auto& trans = registry.get<components::transform>(localplayer);
	auto& rb = registry.get<components::kinematic_ridgedbody>(localplayer);

	glm::mat4 view = glm::mat4(1.0f);
	view = view * glm::toMat4(FMath::createQuaternion(trans.rotation));
	view = glm::translate(view, -(trans.getInterpPos(t) + glm::vec3(0, rb.eyeHeight, 0)));
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), (float)wWidth / (float)wHeight, 0.05f, renderDistance * 16 * (float)sqrt(2));

	glUniformMatrix4fv(texColourProgram.viewID, 1, GL_FALSE, glm::value_ptr(view));
	glUniform4f(colourIDTexCol, 0, 0, 0, 0.4f);

	if (renderPortalDebugOutline)
	{
		RayTraceResult result = scene.RayTraceAllWorlds(t);
		if (result.hasHit)
		{//Draw block selection box
			glDisable(GL_CULL_FACE);
			glLineWidth(2.5f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(glm::scale(projection, glm::vec3(1, 1, 0.999f))));//Emulating a slightly lower FOV

			glBindVertexArray(blockLineVAO.id);

			glm::mat4 model = glm::mat4(1.0f);
			if (result.world->isSubWorld)
			{
				SubWorld& subWorld = *(SubWorld*)result.world;
				model = glm::translate(model, subWorld.offset);
				model = model * glm::toMat4(glm::quat(subWorld.rotation));
				model = glm::translate(model, glm::vec3(result.hitPos));
				model = glm::translate(model, -subWorld.centerOfMassOffset);
			}
			else
			{
				model = glm::translate(model, glm::vec3(result.hitPos));
			}

			glUniformMatrix4fv(texColourProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_LINES, 0, blockLineVAO.count);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(projection));

	{//Render crosshair
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glUniformMatrix4fv(texColourProgram.viewID, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		glBindVertexArray(planeVAO.id);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0, 0, -30));
		glUniformMatrix4fv(texColourProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, planeVAO.count);
	}

	glDisable(GL_BLEND);
}

void GameRenderer::debugDrawPath()
{
	for (PathNode* node : p_pathfinder->path)
	{
		//for (PathNode* node : p_pathfinder->closedSet) {
		glBindVertexArray(planeVAO.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureArrow);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(node->pos) + glm::vec3(0.5f, 0.01f, 0.5f));

		if (node->inPath)
		{
			model = glm::translate(model, glm::vec3(0, 0.05f, 0));
		}

		if (node->face != nullptr)
		{
			model = model * glm::toMat4(node->face->angle);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		}
		else
		{
			model = glm::rotate(model, glm::radians(-80.0f), glm::vec3(1, 0, 0));
		}

		glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, planeVAO.count);
	}
}

void GameRenderer::debugDrawColliders()
{
	//for (auto& it : world.chunks) {//Render collisionBlocks
	//	Chunk* chunk = it.second;

	//	std::unordered_map<glm::ivec3, btCollisionObject*, HashFunc_ivec3, HashFunc_ivec3>::iterator it2 = chunk->storage.begin();
	//	auto end = chunk->storage.end();

	//	while (it2 != end) {
	//		auto pair = *it2;

	//		glBindVertexArray(planeVAO.id);
	//		glActiveTexture(GL_TEXTURE0);
	//		glBindTexture(GL_TEXTURE_2D, textureArrow);

	//		glm::mat4 model = glm::mat4(1.0f);
	//		model = glm::translate(model, glm::vec3(pair.first) + glm::vec3(0.5f, 1.01, 0.5f));
	//		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
	//		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	//		glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

	//		glDrawArrays(GL_TRIANGLES, 0, planeVAO.vertices);

	//		++it2;
	//	}
	//}
}

void GameRenderer::debugDrawBulletDebug()
{
	{//Draw bullet debug
		p_physicsWorld->m_dynamicsWorld->debugDrawWorld();
		std::vector<float>& vertices = p_physicsWorld->m_debugDrawer->vertices;
		if (!vertices.empty())
		{
			GLuint VBO, VAO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);


			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// texture coord attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glm::mat4 model = glm::mat4(1.0f);
			glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

			glLineWidth(5.0f);
			glDrawArrays(GL_LINES, 0, p_physicsWorld->m_debugDrawer->count);

			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);

			vertices.clear();
			p_physicsWorld->m_debugDrawer->count = 0;
		}
	}
}

t_VAO GameRenderer::createCubeVAO()
{
	std::vector<float> vertices;

	//To do: add boilerplate
	float* side = createYFace(0, 1, 0, false);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createYFace(0, 0, 0, true);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createXFace(1, 0, 0, false);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createXFace(0, 0, 0, true);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createZFace(0, 0, 1, false);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	side = createZFace(0, 0, 0, true);
	vertices.insert(vertices.end(), side, &side[6 * 5]);
	delete[] side;

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	if (!vertices.empty())
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	}

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, 6 * 6 };
}

t_VAO  GameRenderer::createLineCubeVAO()
{
	float vertices[] = {
		0.0f, 0.0f, 0.0f,  0.0f, 0.0f,//Front 8
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		0.0f, 0.0f, 1.0f,  0.0f, 0.0f,//Back 8
		1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
		1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

		0.0f, 1.0f, 1.0f,  1.0f, 0.0f,//Left 4
		0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

		1.0f, 1.0f, 1.0f,  1.0f, 0.0f,//Right 4
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,  0.0f, 0.0f,
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, 8 * 2 + 4 * 2 };
}

t_VAO GameRenderer::createQuad()
{
	float* side = createZFace(-0.5f, -0.5f, 0, false);

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float[5 * 6]), &side[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	delete[] side;

	return { VAO, VBO, 6 * 6 };
}

t_VAO GameRenderer::createQuad(float x, float y, float z, float height, float width)
{
	const float face[] = {
			0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
			width + x, 0.0f + y,  0.0f + z,  1.0f, 0.0f,
			width + x, height + y,  0.0f + z,  1.0f, 1.0f,

			width + x, height + y,  0.0f + z,  1.0f, 1.0f,
			0.0f + x, height + y,  0.0f + z,  0.0f, 1.0f,
			0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float[5 * 6]), &face[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, 6 * 6 };
}

t_VAO GameRenderer::createSubChunkVAO(SubChunk& subChunk, SubChunk& above, SubChunk& below, SubChunk& right, SubChunk& left, SubChunk& front, SubChunk& back)
{
	std::vector<float> vertices;

	int count = 0;
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				if (subChunk.getBlock(x, y, z) != 0)
				{
					{//Yfaces
						if (y + 1 >= 16 ? above.getBlock(x, 0, z) == 0 : subChunk.getBlock(x, y + 1, z) == 0)
						{//Top side
							count++;

							float* side = createYFace(x, y + 1, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (y - 1 < 0 ? below.getBlock(x, 15, z) == 0 : subChunk.getBlock(x, y - 1, z) == 0)
						{//Bottom side
							count++;

							float* side = createYFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//Xfaces
						if (x + 1 >= 16 ? right.getBlock(0, y, z) == 0 : subChunk.getBlock(x + 1, y, z) == 0)
						{//Right side
							count++;

							float* side = createXFace(x + 1, y, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}

						if (x - 1 < 0 ? left.getBlock(15, y, z) == 0 : subChunk.getBlock(x - 1, y, z) == 0)
						{//Bottom side
							count++;

							float* side = createXFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//ZFaces
						if (z + 1 >= 16 ? back.getBlock(x, y, 0) == 0 : subChunk.getBlock(x, y, z + 1) == 0)
						{//Back side
							count++;

							float* side = createZFace(x, y, z + 1, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (z - 1 < 0 ? front.getBlock(x, y, 15) == 0 : subChunk.getBlock(x, y, z - 1) == 0)
						{//Front side
							count++;

							float* side = createZFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
				}
			}
		}
	}

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	if (!vertices.empty())
	{
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	}

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return { VAO, VBO, count * 6 };
}

float* GameRenderer::createYFace(float x, float y, float z, bool flipped)
{
	float* face;

	if (flipped)
	{
		face = new float[5 * 6]{
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		1.0f + x,  0.0f + y, 0.0f + z,  1.0f, 1.0f,
		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,

		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x,  0.0f + y, 1.0f + z,  0.0f, 0.0f,
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		};
	}
	else
	{
		face = new float[5 * 6]{
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		1.0f + x,  0.0f + y, 0.0f + z,  1.0f, 1.0f,

		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x,  0.0f + y, 1.0f + z,  0.0f, 0.0f,
		};
	}

	return face;
}

float* GameRenderer::createXFace(float x, float y, float z, bool flipped)
{
	float* face;

	if (flipped)
	{
		face = new float[5 * 6]{
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 1.0f + y, 0.0f + z,  1.0f, 1.0f,
		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,

		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 0.0f + y, 1.0f + z,  0.0f, 0.0f,
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		};
	}
	else
	{
		face = new float[5 * 6]{
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 1.0f + y, 0.0f + z,  1.0f, 1.0f,

		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 0.0f + y, 1.0f + z,  0.0f, 0.0f,
		};
	}

	return face;
}

float* GameRenderer::createZFace(float x, float y, float z, bool flipped)
{
	float* face;

	if (flipped)
	{
		face = new float[5 * 6]{
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		1.0f + x, 0.0f + y,  0.0f + z,  1.0f, 0.0f,

		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		0.0f + x, 1.0f + y,  0.0f + z,  0.0f, 1.0f,
		};
	}
	else
	{
		face = new float[5 * 6]{
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		1.0f + x, 0.0f + y,  0.0f + z,  1.0f, 0.0f,
		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,

		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		0.0f + x, 1.0f + y,  0.0f + z,  0.0f, 1.0f,
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		};
	}

	return face;
}

GLuint GameRenderer::generateTexture(const char* path)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;

	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return texture;
}
