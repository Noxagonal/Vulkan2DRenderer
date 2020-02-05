#pragma once

#include "SourceCommon.h"

#include <memory>
#include <vector>
#include <array>

namespace vk2d {

namespace _internal {



class InstanceImpl;
class DescriptorPoolRequirements;
class DescriptorSetLayout;
class DescriptorAutoPool;
struct PoolDescriptorSet;

// Descriptor pool requirements is a way of detecting compatible descriptor pools for different descriptor set layouts.
// When we're allocating a descriptor set from the DescriptorAutoPool, it needs to create the Vulkan descriptor pools
// internally for different descriptor sets with different VkDescriptorType's.
// In short, we need to have information about what descriptor types are needed and how many bindings are needed in order
// to try and guess the best possible internal Vulkan descriptor pool to allocate from.
// In detail, if your first descriptor set layout uses VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
// DescriptorAutoPool internally creates a vulkan descriptor pool that only has VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
// available, this is to save on resources, the next descriptor set allocation might want
// both VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER and VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER types, the Vulkan descriptor pool
// created internally by the DescriptorAutoPool cannot provide this descriptor set, so the DescriptorAutoPool will
// have to create another internal Vulkan descriptor pool that has both descriptor types.
// After this if we want to allocate another descriptor set with only VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER type
// we could allocate this from either internal Vulkan descriptor pool, but we should try to allocate it from the one
// that would waste the least amount of resources, this problem becomes more severe when considering different amounts
// of different descriptor types, we need some heuristics for this and 100% resource utilization will be near impossible.
class DescriptorPoolRequirements {
	friend class vk2d::_internal::DescriptorSetLayout;

public:
	inline const std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE>	&	GetBindingAmounts() const
	{
		return bindingAmounts;
	};

	// Heuristic function, 0 = will not work, 1 = 100% compatible, 0-1 = will work but there might be a better option.
	 double															CheckCompatibilityWith(
		const DescriptorPoolRequirements						&	other ) const;

private:
	std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE>				bindingAmounts					= {};
	uint64_t														typeBits						= {};
};

struct PoolCategory {
	DescriptorPoolRequirements		originalPoolRequirements		= {};
	VkDescriptorPool				pool							= {};
	bool							isFull							= {};
	uint64_t						counter							= {};
};

class vk2d::_internal::DescriptorSetLayout {
	friend std::unique_ptr<vk2d::_internal::DescriptorSetLayout>	CreateDescriptorSetLayout(
		vk2d::_internal::InstanceImpl							*	instance,
		VkDevice													device,
		const VkDescriptorSetLayoutCreateInfo					*	pCreateInfo );

private:
																	DescriptorSetLayout(
		vk2d::_internal::InstanceImpl							*	instance,
		VkDevice													device,
		const VkDescriptorSetLayoutCreateInfo					*	pCreateInfo );
public:

																	~DescriptorSetLayout();

	 VkDescriptorSetLayout											GetVulkanDescriptorSetLayout() const;
	 const VkDescriptorSetLayoutCreateInfo						&	GetDescriptorSetLayoutCreateInfo() const;

	// Somewhat specialized and mostly used only by the VKRende lirar,
	// Read more above in the DescriptorPoolCategory struct defniton.
	 const vk2d::_internal::DescriptorPoolRequirements			&	GetDescriptorPoolRequirements() const;

																	operator VkDescriptorSetLayout() const;

private:
	vk2d::_internal::InstanceImpl								*	instance_parent							= {};

	VkDescriptorSetLayoutCreateInfo									createInfo								= {};
	VkDevice														refDevice								= {};
	VkDescriptorSetLayout											setLayout								= {};
	vk2d::_internal::DescriptorPoolRequirements						descriptorPoolRequirements				= {};

	bool															is_good									= {};
};

std::unique_ptr<vk2d::_internal::DescriptorSetLayout>				CreateDescriptorSetLayout(
	vk2d::_internal::InstanceImpl								*	instance,
	VkDevice														device,
	const VkDescriptorSetLayoutCreateInfo						*	pCreateInfo );





struct PoolDescriptorSet {
	friend class vk2d::_internal::DescriptorAutoPool;

	operator VkResult() const;

	VkDescriptorSet													descriptorSet							= {};
	VkResult														result									= {};
private:
	VkDescriptorPool												parentPool								= {};
	bool															allocated								= {};
};

class DescriptorAutoPool {
	friend std::unique_ptr<vk2d::_internal::DescriptorAutoPool>		CreateDescriptorAutoPool(
		vk2d::_internal::InstanceImpl							*	instance,
		VkDevice													device );

private:
																	DescriptorAutoPool(
		vk2d::_internal::InstanceImpl							*	instance,
		VkDevice													device );
public:
																	~DescriptorAutoPool();

	vk2d::_internal::PoolDescriptorSet								AllocateDescriptorSet(
		const vk2d::_internal::DescriptorSetLayout				&	rForDescriptorSetLaout);

	 void															FreeDescriptorSet(
		 vk2d::_internal::PoolDescriptorSet						&	pDescriptorSet );

private:
	vk2d::_internal::InstanceImpl								*	instance_parent			= {};

	VkDevice														refDevice				= {};
	std::vector<vk2d::_internal::PoolCategory>						poolCategories			= {};

	bool															is_good					= {};
};

std::unique_ptr<vk2d::_internal::DescriptorAutoPool>				CreateDescriptorAutoPool(
	vk2d::_internal::InstanceImpl								*	instance,
	VkDevice														device );



} // _internal

} // vk2d
