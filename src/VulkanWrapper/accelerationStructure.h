#pragma once

#include "vkRessource.h"
#include <vulkan/vulkan.h>
#include <Shape/mesh.h>
#include <vector>




using namespace std;

/**
 * @brief 
 * La class accélération gère le top level accélération strucuture de Vulkan. 
 * Il crée des bottoms level acceleration structure à chaque fois qu'on ajoute un mesh et 
 * il  les garde en mémoire dans vector. De plus, il crée les buffers des mesh, puis ils gardent 
 * en mémoire.
 * 
 */
class acceleration {


	vector<unique_ptr<buffer>> bufferVertex;
	vector<unique_ptr<buffer>> bufferIndex;


	vkRessource* vk;

	VkAccelerationStructureKHR tlasAccel;
	unique_ptr<buffer> tlasBuffer;
	
	vector<mesh> scene;

	vector<VkAccelerationStructureKHR> m_blas;
	vector<unique_ptr<buffer>> m_blasBuffers;

	unique_ptr<buffer> tlasInstancesBuffer;  
	vector<VkAccelerationStructureInstanceKHR> tlasInstances;

	

	void createTopLevelAS();
	void primitiveToGeometry(VkAccelerationStructureBuildRangeInfoKHR& rangeInfo, VkAccelerationStructureGeometryKHR& geometry, mesh m,VkBuffer i, VkBuffer v);
	unique_ptr<buffer> createAccelerationStructure(VkAccelerationStructureTypeKHR asType,
		VkAccelerationStructureKHR& accelStruct,
		VkAccelerationStructureGeometryKHR& asGeometry,
		VkAccelerationStructureBuildRangeInfoKHR& asBuildRangeInfo,
		VkBuildAccelerationStructureFlagsKHR flags,
		VkBuildAccelerationStructureModeKHR mode
	);

	void buildAccelerationStructure(
		VkAccelerationStructureGeometryKHR& asGeometry,
		VkAccelerationStructureBuildRangeInfoKHR& asBuildRangeInfo,
		VkBuildAccelerationStructureFlagsKHR flags,
		VkBuildAccelerationStructureModeKHR mode
	);
	void cleanup();

	void updateDescriptorSetTlas();
	void updateDescriptorSetVertex(uint32_t index, buffer* vertexBuffer);
	void updateDescriptorSetIndex(uint32_t index, buffer* indexBuffer);

	public:
		acceleration(vkRessource* v);
		~acceleration();
		
		const VkAccelerationStructureKHR& getTop() const;

		acceleration(const acceleration&) = delete;
		acceleration& operator=(const acceleration&) = delete;
		
		void add(mesh m);
		void addInstance(node* n);
		void remove(node* n);
		void rebuildTlas(VkBuildAccelerationStructureModeKHR mode);
		void updateMesh(node*);

		vector<mesh> getScene();
};