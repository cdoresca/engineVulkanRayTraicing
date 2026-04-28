#include "pipeline.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include "physicalDevicePropriete.h"




void  rasterizationPipeline::createPipeline() {

	vector<char> vertShaderCode;
	vector<char> fragShaderCode;


	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)display_ptr->getExtent().width;
	viewport.height = (float)display_ptr->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = display_ptr->getExtent();

	vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;


	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	VK_CHECK(vkCreatePipelineLayout(vk->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = display_ptr->getRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VK_CHECK(vkCreateGraphicsPipelines(vk->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));

	vkDestroyShaderModule(vk->getDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(vk->getDevice(), vertShaderModule, nullptr);
}

void raytracingPipeline::createPipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = vk->getDescriptorSetLayout().size();
	pipelineLayoutInfo.pSetLayouts = vk->getDescriptorSetLayout().data();
	pipelineLayoutInfo.pushConstantRangeCount = vk->getPushConstant().size();
	pipelineLayoutInfo.pPushConstantRanges = vk->getPushConstant().data();

	

	VK_CHECK(vkCreatePipelineLayout(vk->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout));


	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo{};
	rayTracingPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	rayTracingPipelineInfo.pStages = shaderStages.data();
	rayTracingPipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
	rayTracingPipelineInfo.pGroups = shaderGroups.data();
	rayTracingPipelineInfo.maxPipelineRayRecursionDepth = 3;
	rayTracingPipelineInfo.layout = pipelineLayout;

	vk->rt.vkCreateRayTracingPipelinesKHR(vk->getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineInfo, nullptr, &graphicsPipeline);
	createShaderBindingTable(rayTracingPipelineInfo);

	shader compute(vk, SHADER_DIR"frameAccumulation.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
	
	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.stage = compute.getPipelineShader();

	VK_CHECK(vkCreateComputePipelines(vk->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline));

}



void rasterizationPipeline::recordCommand(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex){
	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = display_ptr->getRenderPass();
	renderPassInfo.framebuffer = display_ptr->getFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = display_ptr->getExtent();

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)display_ptr->getExtent().width;
	viewport.height = (float)display_ptr->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = display_ptr->getExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));

}

void rasterizationPipeline::cleanup()
{
}

rasterizationPipeline::rasterizationPipeline(vkRessource*v, display*d):pipeline(v,d)
{
}

rasterizationPipeline::~rasterizationPipeline()
{
	cleanup();
}

void rasterizationPipeline::drawFrame()
{
}

void raytracingPipeline::addShader(shader* s)
{
	shaderStages.push_back(s->getPipelineShader());
	shaderGroups.push_back(s->getShaderGroup());
}

void raytracingPipeline::createShaderBindingTable(const VkRayTracingPipelineCreateInfoKHR& rtPipelineInfo){
	physicalDevicePropriete prop = queryphysicalDevicePropriete(vk->getPhysicalDevice());

	auto alignUp = [](uint32_t size, uint32_t alignment) { return (size + alignment - 1) & ~(alignment - 1); };

	uint32_t handlesize = prop.m_rtProperties.shaderGroupHandleSize;
	uint32_t handleAlignment = prop.m_rtProperties.shaderGroupHandleAlignment;
	uint32_t baseAlignment = prop.m_rtProperties.shaderGroupBaseAlignment;
	uint32_t groupCount = rtPipelineInfo.groupCount;

	size_t dataSize = handlesize * groupCount;
	shaderHandles.resize(dataSize);

	vk->rt.vkGetRayTracingShaderGroupHandlesKHR(vk->getDevice(), graphicsPipeline, 0, groupCount, dataSize, shaderHandles.data());

	uint32_t groupStride = alignUp(handlesize, handleAlignment);

	uint32_t raygenOffset = 0;
	uint32_t missOffset = alignUp(raygenOffset + groupStride, baseAlignment);
	uint32_t hitOffset = alignUp(missOffset + groupStride, baseAlignment);
	uint32_t callableOffset = alignUp(hitOffset + groupStride, baseAlignment);


	size_t bufferSize = callableOffset;
	
	const VkBufferUsageFlags bufferUsageFlags =
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	const VkMemoryPropertyFlags memoryUsageFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	shaderBindingTable = vk->createBuffer(bufferSize,bufferUsageFlags,memoryUsageFlags);
	

	shaderBindingTable->map();
	uint8_t* data = static_cast<uint8_t*>(shaderBindingTable->data());
	VkDeviceAddress baseAddress = vk->queryBufferAddress(shaderBindingTable->get());
	
	// Raygen
	memcpy(data + raygenOffset, shaderHandles.data() + 0 * handlesize, handlesize);
	raygenRegion.deviceAddress = baseAddress + raygenOffset;
	raygenRegion.stride = groupStride;
	raygenRegion.size = groupStride;

	// Miss
	memcpy(data + missOffset, shaderHandles.data() + 1 * handlesize, handlesize);
	memcpy(data + missOffset + groupStride, shaderHandles.data() + 2 * handlesize, handlesize);
	missRegion.deviceAddress = baseAddress + missOffset;
	missRegion.stride = groupStride;
	missRegion.size = groupStride * 2;

	// Hit
	memcpy(data + hitOffset, shaderHandles.data() + 3 * handlesize, handlesize);
	memcpy(data + hitOffset + groupStride, shaderHandles.data() + 4 * handlesize, handlesize);
	hitRegion.deviceAddress = baseAddress + hitOffset;
	hitRegion.stride = groupStride;
	hitRegion.size = groupStride * 2;


	// Pas de callable
	callableRegion.deviceAddress = 0;
	callableRegion.stride = 0;
	callableRegion.size = 0;
}


void raytracingPipeline::recordCommand(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t frameIndex){
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	vk->setImageLayout(
		commandBuffer,
		display_ptr->getStorage(CURRENT, frameIndex).image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR
	);

	VkDescriptorSet set[] = {
		vk->getSet(frameIndex),
		vk->getSet(frameIndex,1),
		vk->getSet(frameIndex,2)
	};

	

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, graphicsPipeline);

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		pipelineLayout,
		0,
		vk->getDescriptorSetLayout().size(),
		set,
		0,
		nullptr
	);
	cam->frame = frameIndex;
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0, sizeof(Camera), cam);
	

	vk->rt.vkCmdTraceRaysKHR(
		commandBuffer,
		&raygenRegion,
		&missRegion,
		&hitRegion,
		&callableRegion,
		display_ptr->getExtent().width,
		display_ptr->getExtent().height,
		1
	);
	// DEBUT COMPUTE SHADER
	vk->setImageLayout(
		commandBuffer,
		display_ptr->getStorage(HISTORY, frameIndex).image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
	);

	vk->setImageLayout(
		commandBuffer,
		display_ptr->getStorage(OUTPUT, frameIndex).image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
	);

	vk->setImageLayout(
		commandBuffer,
		display_ptr->getStorage(CURRENT,frameIndex).image,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
	);
	uint32_t groupCountX = (display_ptr->getExtent().width + localX - 1) / localX;
	uint32_t groupCountY = (display_ptr->getExtent().height + localY - 1) / localY;

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		pipelineLayout,
		0,
		vk->getDescriptorSetLayout().size(),
		set,
		0,
		nullptr
	);
	vkCmdDispatch(commandBuffer, groupCountX, groupCountY, 1);


	// FIN
	vk->setImageLayout(
		commandBuffer,
		display_ptr->getStorage(OUTPUT,frameIndex).image,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		VK_PIPELINE_STAGE_TRANSFER_BIT
	);

	VkImage swapchainImage = display_ptr->getImage(imageIndex);
	VkImageLayout oldLayout = display_ptr->getLayout(imageIndex);
	vk->setImageLayout(
		commandBuffer,
		swapchainImage,
		oldLayout,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT
	);
	display_ptr->setLayout(imageIndex,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkImageCopy region{};
	region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.extent = { display_ptr->getExtent().width, display_ptr->getExtent().height, 1 };

	vkCmdCopyImage(
		commandBuffer,
		display_ptr->getStorage(OUTPUT,frameIndex).image, 
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		swapchainImage, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, 
		&region
	);

	oldLayout = display_ptr->getLayout(imageIndex);
	vk->setImageLayout(
		commandBuffer,
		swapchainImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	);
	display_ptr->setLayout(imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = display_ptr->getRenderPass();
	renderPassInfo.framebuffer = display_ptr->getFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = display_ptr->getExtent();
	renderPassInfo.clearValueCount = 0;
	renderPassInfo.pClearValues = nullptr;

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = display_ptr->getExtent().width;
	viewport.height = display_ptr->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = display_ptr->getExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));
	display_ptr->setLayout(imageIndex, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void raytracingPipeline::cleanup()
{
	VkDevice device = vk->getDevice();

	// 1. Pipeline
	if (graphicsPipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		graphicsPipeline = VK_NULL_HANDLE;
	}
	if (computePipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(device, computePipeline, nullptr);
		graphicsPipeline = VK_NULL_HANDLE;
	}

	// 2. Pipeline layout
	if (pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}
	 
	shaderStages.clear();

	for (auto& semaphore : imageAvailableSemaphores)
		vkDestroySemaphore(device, semaphore, nullptr);
	for (auto& semaphore : renderFinishedSemaphores)
		vkDestroySemaphore(device, semaphore, nullptr);
	for (auto& fence : inFlightFences)
		vkDestroyFence(device, fence, nullptr);

	

	// 5. Shader Binding Table
	shaderBindingTable.reset(); // RAII


}



raytracingPipeline::raytracingPipeline(vkRessource* v, display* d):pipeline(v, d)
{
	createCommandBuffers();       
	createSyncObjects();         
}

raytracingPipeline::~raytracingPipeline()
{
	cleanup();
}

void raytracingPipeline::setCamera(Camera* cam)
{
	this->cam = cam;
}

void raytracingPipeline::drawFrame() {
	VK_CHECK(vkWaitForFences(vk->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX));
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		vk->getDevice(),
		display_ptr->getSwapChain(),
		UINT64_MAX,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		display_ptr->recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VK_CHECK(vkResetFences(vk->getDevice(), 1, &inFlightFences[currentFrame]));

	VK_CHECK(vkResetCommandBuffer(cmd[currentFrame], 0));

	recordCommand(cmd[currentFrame], imageIndex,currentFrame);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(vk->getGraphicQueue(), 1, &submitInfo, inFlightFences[currentFrame]));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { display_ptr->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(vk->getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || display_ptr->framebufferResized) {
		display_ptr->framebufferResized = false;
		display_ptr->recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return;
}

const VkPipelineLayout& pipeline::getPipelineLayout()
{
	return pipelineLayout;
}

void pipeline::createSyncObjects() {
	uint32_t imageCount = display_ptr->getImageCount();

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(imageCount);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK(vkCreateSemaphore(vk->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
		VK_CHECK(vkCreateFence(vk->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]));
	}
	for (size_t i = 0; i < imageCount; i++) {
		VK_CHECK(vkCreateSemaphore(vk->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
	}
}


void pipeline::createCommandBuffers() {
	cmd.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vk->getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)cmd.size();

	VK_CHECK(vkAllocateCommandBuffers(vk->getDevice(), &allocInfo, cmd.data()));
}

pipeline::pipeline(vkRessource*v, display*d):vk(v),display_ptr(d)
{
}

pipeline::~pipeline()
{
}


