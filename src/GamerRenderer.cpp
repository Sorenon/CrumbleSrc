#include "GamerRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb_image.h>

#include "Player.h"
#include "globals.h"
#include "FMath.h"
#include "PhysicsWorld.h"

GamerRenderer::GamerRenderer() {
	stbi_set_flip_vertically_on_load(true);
	texture = generateTexture("C:/Users/Sorenon/Crumble/src/main/resources/grass.png");
}


GamerRenderer::~GamerRenderer() {
	//TODO clean up
}

void GamerRenderer::doRender(float t) {
	//int chunksUpdated = 0;
	for (auto pair : world.chunks) {//Remake VAOs
		//break;
		Chunk &chunk = *pair.second;
		chunkID id = pair.first;
		int x = id >> 32;
		int z = (int)id;

		if (chunk.needsUpdate) {
			//std::cout << "Updating Chunk " << ++chunksUpdated << std::endl;

			for (int i = 0; i < 16; i++) {
				SubChunk *subChunk = chunk.subChunks[i];
				if (subChunk != nullptr && subChunk->needsUpdate) {

					SubChunk &above = i + 1 >= 16 ? SubChunk::EMPTY : chunk.getSubChunkSafe(i + 1);
					SubChunk &below = i - 1 < 0 ? SubChunk::EMPTY : chunk.getSubChunkSafe(i - 1);

					SubChunk &right = world.getChunkSafe(x + 1, z).getSubChunkSafe(i);
					SubChunk &left = world.getChunkSafe(x - 1, z).getSubChunkSafe(i);

					SubChunk &front = world.getChunkSafe(x, z - 1).getSubChunkSafe(i);
					SubChunk &back = world.getChunkSafe(x, z + 1).getSubChunkSafe(i);

					t_VAO oldVAO = chunk.subChunkVAOs[i];
					if (oldVAO.id != 0) {
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

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	texturedProgram.activate();

	float renderDistance = 12;
	glm::mat4 projection = glm::perspective(glm::radians(40.0f), (float)wWidth / (float)wHeight, 0.05f, renderDistance * 16 * (float)sqrt(2));
	glUniformMatrix4fv(texturedProgram.projID, 1, GL_FALSE, glm::value_ptr(projection));
	//glUniformMatrix4fv(projID, 1, GL_FALSE, glm::value_ptr(glm::scale(projection, glm::vec3(0.5f, 0.5f, 1)))); //Interesting effect

	glm::mat4 view = glm::mat4(1.0f);
	view = glm::rotate(view, p_player->transform.rotation.x, Vectors::RIGHT);
	view = glm::rotate(view, p_player->transform.rotation.y, Vectors::UP);
	view = glm::translate(view, -p_player->getEyePos(t));

	glUniformMatrix4fv(texturedProgram.viewID, 1, GL_FALSE, glm::value_ptr(view));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	texturedProgram.activate();

	for (auto pair : world.chunks) { //Render world
		Chunk &chunk = *pair.second;
		chunkID id = pair.first;
		int x = id >> 32;
		int z = (int)id;

		for (int i = 0; i < 16; i++) {
			t_VAO &vao = chunk.subChunkVAOs[i];

			if (vao.id != 0) {
				glBindVertexArray(vao.id);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);

				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(x * 16, i * 16, z * 16));
				glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_TRIANGLES, 0, vao.vertices);
			}
		}
	}

	{//Render bullet object
		btTransform trans;
		p_physicsWorld->rbCube->getMotionState()->getWorldTransform(trans);

		btVector3 transOrigin = trans.getOrigin();

		glBindVertexArray(cubeVAO.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glm::mat4 model = glm::mat4(1.0f);
		trans.getOpenGLMatrix(glm::value_ptr(model));
		model = glm::translate(model, -glm::vec3(0.5f, 0.5f, 0.5f));//TODO: impove cube VBO 
		glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, cubeVAO.vertices);
	}

	//if (false) {//Render collisionBlocks
	//	for (auto &it : world.chunks) {
	//		Chunk *chunk = it.second;

	//		std::unordered_map<glm::ivec3, btCollisionObject*, HashFunc_ivec3, HashFunc_ivec3>::iterator it2 = chunk->storage.begin();
	//		auto end = chunk->storage.end();

	//		while (it2 != end) {
	//			auto pair = *it2;

	//			glBindVertexArray(cubeVAO.id);
	//			glActiveTexture(GL_TEXTURE0);
	//			glBindTexture(GL_TEXTURE_2D, 0);

	//			glm::mat4 model = glm::mat4(1.0f);
	//			model = glm::translate(model, glm::vec3(pair.first) + glm::vec3(0.1f, 0.1f, 0.1f));//TODO: impove cube VBO 
	//			glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

	//			glDrawArrays(GL_TRIANGLES, 0, cubeVAO.vertices);

	//			++it2;
	//		}
	//	}
	//}

	//{//Draw bullet debug
	//	dynamicsWorld->debugDrawWorld();
	//	std::vector<float> &vertices = debugDraw->vertices;
	//	if (!vertices.empty()) {
	//		GLuint VBO, VAO;
	//		glGenVertexArrays(1, &VAO);
	//		glGenBuffers(1, &VBO);

	//		glBindVertexArray(VAO);

	//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);


	//		// position attribute
	//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	//		glEnableVertexAttribArray(0);
	//		// texture coord attribute
	//		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//		glEnableVertexAttribArray(1);

	//		glm::mat4 model = glm::mat4(1.0f);
	//		glUniformMatrix4fv(texturedProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

	//		glDrawArrays(GL_LINES, 0, debugDraw->count);

	//		glDeleteVertexArrays(1, &VAO);
	//		glDeleteBuffers(1, &VBO);

	//		vertices.clear();
	//		debugDraw->count = 0;
	//	}
	//}

	RayTraceResult result = world.rayTrace(p_player->getEyePos(t), p_player->transform.getLook(t));

	texColourProgram.activate();

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUniformMatrix4fv(texColourProgram.viewID, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(alphaIDTexCol, 0.4f);

	if (result.hit) {
		glDisable(GL_CULL_FACE);
		glLineWidth(2.5f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(glm::scale(projection, glm::vec3(1, 1, 0.999f))));//Emulating a slightly lower FOV

		glBindVertexArray(blockLineVAO.id);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(result.hitPos));
		glUniformMatrix4fv(texColourProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_LINES, 0, blockLineVAO.vertices);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glUniformMatrix4fv(texColourProgram.projID, 1, GL_FALSE, glm::value_ptr(projection));

	{//Render gui
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glUniformMatrix4fv(texColourProgram.viewID, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		glBindVertexArray(plane.id);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-0.5, -0.5, -30));
		glUniformMatrix4fv(texColourProgram.modelID, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, plane.vertices);
	}

	glDisable(GL_BLEND);
}

t_VAO  GamerRenderer::createCubeVAO() {
	std::vector<float> vertices;

	//To do: add boilerplate
	float *side = createYFace(0, 1, 0, false);
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
	if (!vertices.empty()) {
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

t_VAO  GamerRenderer::createLineCubeVAO() {
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

t_VAO GamerRenderer::createGUIPlain() {
	float *side = createZFace(0, 0, 0, false);

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

t_VAO GamerRenderer::createSubChunkVAO(SubChunk & subChunk, SubChunk & above, SubChunk & below, SubChunk & right, SubChunk & left, SubChunk & front, SubChunk & back) {
	std::vector<float> vertices;

	int count = 0;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				if (subChunk.getBlock(x, y, z) != 0) {
					{//Yfaces
						if (y + 1 >= 16 ? above.getBlock(x, 0, z) == 0 : subChunk.getBlock(x, y + 1, z) == 0) {//Top side
							count++;

							float *side = createYFace(x, y + 1, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (y - 1 < 0 ? below.getBlock(x, 15, z) == 0 : subChunk.getBlock(x, y - 1, z) == 0) {//Bottom side
							count++;

							float *side = createYFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//Xfaces
						if (x + 1 >= 16 ? right.getBlock(0, y, z) == 0 : subChunk.getBlock(x + 1, y, z) == 0) {//Right side
							count++;

							float *side = createXFace(x + 1, y, z, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}

						if (x - 1 < 0 ? left.getBlock(15, y, z) == 0 : subChunk.getBlock(x - 1, y, z) == 0) {//Bottom side
							count++;

							float *side = createXFace(x, y, z, true);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
					}
					{//ZFaces
						if (z + 1 >= 16 ? back.getBlock(x, y, 0) == 0 : subChunk.getBlock(x, y, z + 1) == 0) {//Back side
							count++;

							float *side = createZFace(x, y, z + 1, false);

							vertices.insert(vertices.end(), side, &side[6 * 5]);

							delete[] side;
						}
						if (z - 1 < 0 ? front.getBlock(x, y, 15) == 0 : subChunk.getBlock(x, y, z - 1) == 0) {//Front side
							count++;

							float *side = createZFace(x, y, z, true);

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
	if (!vertices.empty()) {
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

float * GamerRenderer::createYFace(int x, int y, int z, bool flipped) {
	float *face;

	if (flipped) {
		face = new float[5 * 6]{
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		1.0f + x,  0.0f + y, 0.0f + z,  1.0f, 1.0f,
		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,

		1.0f + x,  0.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x,  0.0f + y, 1.0f + z,  0.0f, 0.0f,
		0.0f + x,  0.0f + y, 0.0f + z,  0.0f, 1.0f,
		};
	} else {
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

float * GamerRenderer::createXFace(int x, int y, int z, bool flipped) {
	float *face;

	if (flipped) {
		face = new float[5 * 6]{
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		0.0f + x, 1.0f + y, 0.0f + z,  1.0f, 1.0f,
		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,

		0.0f + x, 0.0f + y, 0.0f + z,  0.0f, 1.0f,
		0.0f + x, 0.0f + y, 1.0f + z,  0.0f, 0.0f,
		0.0f + x, 1.0f + y, 1.0f + z,  1.0f, 0.0f,
		};
	} else {
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

float * GamerRenderer::createZFace(int x, int y, int z, bool flipped) {
	float *face;

	if (flipped) {
		face = new float[5 * 6]{
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		1.0f + x, 0.0f + y,  0.0f + z,  1.0f, 0.0f,

		1.0f + x, 1.0f + y,  0.0f + z,  1.0f, 1.0f,
		0.0f + x, 0.0f + y,  0.0f + z,  0.0f, 0.0f,
		0.0f + x, 1.0f + y,  0.0f + z,  0.0f, 1.0f,
		};
	} else {
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

GLuint GamerRenderer::generateTexture(const char* path) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels;

	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return texture;
}
