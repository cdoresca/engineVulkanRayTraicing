#include "accelerationStructure.h"
#include <span>
#include "physicalDevicePropriete.h"
#include <glm/gtc/type_ptr.hpp>



acceleration::acceleration(vkRessource* v) : vk(v){
	createTopLevelAS();
}

acceleration::~acceleration() {
	cleanup();
}
void acceleration::primitiveToGeometry(VkAccelerationStructureBuildRangeInfoKHR& rangeInfo, VkAccelerationStructureGeometryKHR& geometry, mesh m,VkBuffer v, VkBuffer i) {
	//TODO:A revoir;
	
	uint32_t maxIndex = 0;
	for (uint32_t j = 0; j < m.indices.size(); j++)
		maxIndex = std::max(maxIndex, m.indices[j]);

	VkAccelerationStructureGeometryTrianglesDataKHR triangle{};
	triangle.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	triangle.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	triangle.vertexData.deviceAddress = vk->queryBufferAddress(v);
	triangle.indexType = VK_INDEX_TYPE_UINT32;
	triangle.vertexStride = sizeof(vertex); 
	triangle.indexData.deviceAddress = vk->queryBufferAddress(i);
	triangle.maxVertex =  maxIndex + 1;


	
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geometry.geometry.triangles = triangle;
	geometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR | VK_GEOMETRY_OPAQUE_BIT_KHR;

	rangeInfo.primitiveCount = m.indices.size() / 3;
	rangeInfo.primitiveOffset = 0;
	rangeInfo.firstVertex = 0;
	rangeInfo.transformOffset = 0;
}



unique_ptr<buffer> acceleration::createAccelerationStructure(VkAccelerationStructureTypeKHR asType,
	VkAccelerationStructureKHR& accelStruct,
	VkAccelerationStructureGeometryKHR& asGeometry,
	VkAccelerationStructureBuildRangeInfoKHR& asBuildRangeInfo,
	VkBuildAccelerationStructureFlagsKHR flags,
	VkBuildAccelerationStructureModeKHR mode
) {
	auto alignUp = [](auto value, size_t alignment) noexcept { return ((value + alignment - 1) & ~(alignment - 1)); };

	VkAccelerationStructureBuildGeometryInfoKHR asBuildInfo{};
	asBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	asBuildInfo.type = asType;
	asBuildInfo.flags = flags;
	asBuildInfo.mode = mode;
	asBuildInfo.geometryCount = 1;
	asBuildInfo.pGeometries = &asGeometry;

	std::vector<uint32_t> maxPrimCount(1);
	maxPrimCount[0] = asBuildRangeInfo.primitiveCount;

	VkAccelerationStructureBuildSizesInfoKHR asBuildSize{};
	asBuildSize.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;


	vk->rt.vkGetAccelerationStructureBuildSizesKHR(vk->getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asBuildInfo,
		maxPrimCount.data(), &asBuildSize);

	physicalDevicePropriete prop = queryphysicalDevicePropriete(vk->getPhysicalDevice());


	VkDeviceSize scratchSize = alignUp(asBuildSize.buildScratchSize, prop.m_asProperties.minAccelerationStructureScratchOffsetAlignment);
	VkBufferUsageFlags usage= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

	unique_ptr<buffer> scratchBuffer = vk->createBuffer(scratchSize,
		usage, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	VkBufferUsageFlags asUsage =
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	auto asBuffer = vk->createBuffer(
		asBuildSize.accelerationStructureSize,
		asUsage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.type = asType;
	createInfo.size = asBuildSize.accelerationStructureSize;
	createInfo.buffer = asBuffer->get();  
	createInfo.offset = 0;
	vk->rt.vkCreateAccelerationStructureKHR(vk->getDevice(), &createInfo, nullptr, &accelStruct);

	VkCommandBuffer tmp = vk->createTmpCmdBuffer();
	asBuildInfo.dstAccelerationStructure = accelStruct;
	asBuildInfo.scratchData.deviceAddress = vk->queryBufferAddress(scratchBuffer->get());
	
	VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &asBuildRangeInfo;
	vk->rt.vkCmdBuildAccelerationStructuresKHR(tmp, 1, &asBuildInfo, &pBuildRangeInfo);


	vk->flushCommandBuffer(tmp);
	return asBuffer;
}

void acceleration::createTopLevelAS(){
	
	
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	VkDeviceSize size = (VkDeviceSize)sizeof(VkAccelerationStructureInstanceKHR) * MAX_MESHES;;

	tlasInstancesBuffer = vk->createBuffer(size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	
	VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{ .primitiveCount = MAX_MESHES };

	VkAccelerationStructureGeometryInstancesDataKHR geometryInstances{ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
												.data = {.deviceAddress = vk->queryBufferAddress(tlasInstancesBuffer->get()) }
	};
	
	VkAccelerationStructureGeometryKHR       asGeometry{ .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
						  .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
						  .geometry = {.instances = geometryInstances} 
	};

	tlasBuffer = createAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, tlasAccel, asGeometry,
		asBuildRangeInfo, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR, VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

	updateDescriptorSetTlas();
}



void acceleration::add(mesh m)
{
	if (m.vertices.empty()) return;
	scene.push_back(m);
	bufferVertex.push_back(vk->createVertexBuffer(m.vertices));
	bufferIndex.push_back(vk->createIndexBuffer(m.indices));

	updateDescriptorSetVertex(bufferVertex.size() - 1, bufferVertex.back().get());
	updateDescriptorSetIndex(bufferIndex.size() - 1, bufferIndex.back().get());

	VkAccelerationStructureKHR blas{};
	VkAccelerationStructureGeometryKHR       geometry{};
	VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};

	primitiveToGeometry(asBuildRangeInfo, geometry, m,bufferVertex.back()->get(), bufferIndex.back()->get());
	m_blasBuffers.push_back(createAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		blas, geometry,
		asBuildRangeInfo, 
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, 
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR
	));
	
	m_blas.push_back(blas);
	auto toTransformMatrixKHR = [](const glm::mat4& m) {
		VkTransformMatrixKHR t;
		memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
		return t;
	};

	
	VkAccelerationStructureInstanceKHR asInstance{};
	asInstance.transform = toTransformMatrixKHR(m.him->model);
	asInstance.instanceCustomIndex = scene.size() - 1;
	asInstance.accelerationStructureReference = vk->queryASAddress(blas);
	asInstance.instanceShaderBindingTableRecordOffset = 0;
	asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	asInstance.mask = 0xFF;
	tlasInstances.emplace_back(asInstance);
	
}

void acceleration::addInstance(node* n)
{
	auto toTransformMatrixKHR = [](const glm::mat4& m) {
		VkTransformMatrixKHR t;
		memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
		return t;
	};

	
	for (int i = 0; i < scene.size(); i++) {
		if (n == scene[i].him || n == scene[i].him->parent) {

			VkAccelerationStructureInstanceKHR asInstance{};
			asInstance.transform = toTransformMatrixKHR(scene[i].him->model);
			asInstance.instanceCustomIndex = i;
			asInstance.accelerationStructureReference = vk->queryASAddress(m_blas[i]);
			asInstance.instanceShaderBindingTableRecordOffset = 0;
			asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			asInstance.mask = 0xFF;
			tlasInstances.emplace_back(asInstance);
			scene[i].hide = false;
		}
	}
	
}

void acceleration::remove(node* n)
{
	tlasInstances.clear();
	auto toTransformMatrixKHR = [](const glm::mat4& m) {
		VkTransformMatrixKHR t;
		memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
		return t;
		};
	for (int i = 0; i < scene.size(); i++) {
		if (n == scene[i].him || n == scene[i].him->parent) { 
			scene[i].hide = true;
			continue; 
		}
		
		if(scene[i].hide) continue;


		VkAccelerationStructureInstanceKHR asInstance{};
		asInstance.transform = toTransformMatrixKHR(scene[i].him->model);
		asInstance.instanceCustomIndex = i;
		asInstance.accelerationStructureReference = vk->queryASAddress(m_blas[i]);
		asInstance.instanceShaderBindingTableRecordOffset = 0;
		asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		asInstance.mask = 0xFF;
		tlasInstances.emplace_back(asInstance);

	}
}

void acceleration::rebuildTlas(VkBuildAccelerationStructureModeKHR mode)
{

	VkDeviceSize size = std::span<VkAccelerationStructureInstanceKHR const>(tlasInstances).size_bytes();

	if (size > 0) {
		vk->staggingBuffer(size,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			tlasInstancesBuffer->get(),
			tlasInstances.data()
		);
	}
	

	VkAccelerationStructureGeometryInstancesDataKHR geometryInstances{
	   .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
	   .data = {.deviceAddress = vk->queryBufferAddress(tlasInstancesBuffer->get()) }
	};

	VkAccelerationStructureGeometryKHR asGeometry{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = {.instances = geometryInstances }
	};

	VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{
		.primitiveCount = static_cast<uint32_t>(tlasInstances.size())
	};

	buildAccelerationStructure(
		asGeometry, asBuildRangeInfo,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR |
		VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
		mode
	);
}

void acceleration::updateMesh(node* n)
{
	auto toTransformMatrixKHR = [](const glm::mat4& m) {
		VkTransformMatrixKHR t;
		memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
		return t;
	};
	for (int i = 0; i < tlasInstances.size(); i++) {
		if(n == scene[i].him)
			tlasInstances[i].transform = toTransformMatrixKHR(scene[i].him->model);
		else if(n == scene[i].him->parent)
			tlasInstances[i].transform = toTransformMatrixKHR(n->model * scene[i].him->model);
	}

}

void acceleration::updateDescriptorSetTlas()
{
	for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		VkDescriptorSet set0 = vk->getSet(i);

		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &tlasAccel;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = set0;
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		vkUpdateDescriptorSets(vk->getDevice(), 1, &accelerationStructureWrite, 0, VK_NULL_HANDLE);
	}
}

void acceleration::updateDescriptorSetVertex(uint32_t index, buffer* vertexBuffer)
{

	for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {


		VkDescriptorSet set1 = vk->getSet(i,1);

		VkDescriptorBufferInfo verterDescriptor{};
		verterDescriptor.buffer = vertexBuffer->get();
		verterDescriptor.offset = 0;
		verterDescriptor.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet vertexWrite{};
		vertexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vertexWrite.pNext = nullptr;
		vertexWrite.dstSet = set1;
		vertexWrite.dstBinding = 0;
		vertexWrite.dstArrayElement = index;
		vertexWrite.descriptorCount = 1;
		vertexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertexWrite.pBufferInfo = &verterDescriptor;

		vkUpdateDescriptorSets(vk->getDevice(), 1, &vertexWrite, 0, VK_NULL_HANDLE);
	}
}

void acceleration::updateDescriptorSetIndex(uint32_t index, buffer* indexBuffer)
{

	for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {


		VkDescriptorSet set1 = vk->getSet(i, 1);

		VkDescriptorBufferInfo indexDescriptor{};
		indexDescriptor.buffer = indexBuffer->get();
		indexDescriptor.offset = 0;
		indexDescriptor.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet indexWrite{};
		indexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		indexWrite.pNext = nullptr;
		indexWrite.dstSet = set1;
		indexWrite.dstBinding = 1;
		indexWrite.dstArrayElement = index;
		indexWrite.descriptorCount = 1;
		indexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indexWrite.pBufferInfo = &indexDescriptor;

		vkUpdateDescriptorSets(vk->getDevice(), 1, &indexWrite, 0, VK_NULL_HANDLE);
	}
}

void acceleration::buildAccelerationStructure(VkAccelerationStructureGeometryKHR& asGeometry, VkAccelerationStructureBuildRangeInfoKHR& asBuildRangeInfo, VkBuildAccelerationStructureFlagsKHR flags, VkBuildAccelerationStructureModeKHR mode)
{
	auto alignUp = [](auto value, size_t alignment) noexcept {
		return ((value + alignment - 1) & ~(alignment - 1));
		};

	VkAccelerationStructureBuildGeometryInfoKHR asBuildInfo{};
	asBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	asBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	asBuildInfo.flags = flags;
	asBuildInfo.mode = mode;
	asBuildInfo.geometryCount = 1;
	asBuildInfo.pGeometries = &asGeometry;
	asBuildInfo.dstAccelerationStructure = tlasAccel;  // ← réutilise l'existant
	asBuildInfo.srcAccelerationStructure = tlasAccel;

	// Query scratch size
	std::vector<uint32_t> maxPrimCount = { asBuildRangeInfo.primitiveCount };

	VkAccelerationStructureBuildSizesInfoKHR asBuildSize{};
	asBuildSize.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vk->rt.vkGetAccelerationStructureBuildSizesKHR(vk->getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&asBuildInfo, maxPrimCount.data(), &asBuildSize);

	physicalDevicePropriete prop = queryphysicalDevicePropriete(vk->getPhysicalDevice());
	VkDeviceSize scratchSize = alignUp(asBuildSize.buildScratchSize,
		prop.m_asProperties.minAccelerationStructureScratchOffsetAlignment);

	// Scratch buffer temporaire
	unique_ptr<buffer> scratchBuffer = vk->createBuffer(scratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	asBuildInfo.scratchData.deviceAddress = vk->queryBufferAddress(scratchBuffer->get());

	VkCommandBuffer tmp = vk->createTmpCmdBuffer();
	VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &asBuildRangeInfo;
	vk->rt.vkCmdBuildAccelerationStructuresKHR(tmp, 1, &asBuildInfo, &pBuildRangeInfo);
	vk->flushCommandBuffer(tmp);
}

void acceleration::cleanup() {
	for (size_t i = 0; i < m_blas.size(); i++)
	{
		vk->rt.vkDestroyAccelerationStructureKHR(vk->getDevice(), m_blas[i], nullptr);
	}
	vk->rt.vkDestroyAccelerationStructureKHR(vk->getDevice(), tlasAccel, nullptr);

	vk = nullptr;
}

const  VkAccelerationStructureKHR& acceleration::getTop() const  { return tlasAccel; }

vector<mesh> acceleration::getScene(){ return scene;}