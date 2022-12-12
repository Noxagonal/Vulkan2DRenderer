#pragma once

#include <core/SourceCommon.hpp>

#include <vulkan/shaders/ShaderInterface.hpp>

#include <utility/Hasher.hpp>
#include <utility/EnumHelpers.hpp>



namespace vk2d {
namespace vulkan {



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderPassAttachmentCreateInfo
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline												RenderPassAttachmentCreateInfo() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline												RenderPassAttachmentCreateInfo(
		VkFormat										format,
		VkSampleCountFlagBits							samples,
		VkAttachmentLoadOp								load_op,
		VkAttachmentStoreOp								store_op,
		VkAttachmentLoadOp								stencil_load_op,
		VkAttachmentStoreOp								stencil_store_op,
		VkImageLayout									initial_layout,
		VkImageLayout									final_layout
	) :
		format( format ),
		samples( samples ),
		load_op( load_op ),
		store_op( store_op ),
		stencil_load_op( stencil_load_op ),
		stencil_store_op( stencil_store_op ),
		initial_layout( initial_layout ),
		final_layout( final_layout )
	{
		hash = CalculateHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline													RenderPassAttachmentCreateInfo(
		const RenderPassAttachmentCreateInfo			&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline RenderPassAttachmentCreateInfo				&	operator=(
		const RenderPassAttachmentCreateInfo			&	other
		) = default;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkFormat											GetFormat() const
	{
		return format;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkSampleCountFlagBits							GetSamples() const
	{
		return samples;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkAttachmentLoadOp								GetLoadOp() const
	{
		return load_op;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkAttachmentStoreOp								GetStoreOp() const
	{
		return store_op;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkAttachmentLoadOp								GetStencilLoadOp() const
	{
		return stencil_load_op;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkAttachmentStoreOp								GetStencilStoreOp() const
	{
		return stencil_store_op;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkImageLayout									GetInitialLayout() const
	{
		return initial_layout;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkImageLayout									GetFinalLayout() const
	{
		return final_layout;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t											GetHash() const
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t											CalculateHash() const
	{
		Hasher hasher;
		hasher.Hash( EnumToUnderlying( format ) );
		hasher.Hash( EnumToUnderlying( samples ) );
		hasher.Hash( EnumToUnderlying( load_op ) );
		hasher.Hash( EnumToUnderlying( store_op ) );
		hasher.Hash( EnumToUnderlying( stencil_load_op ) );
		hasher.Hash( EnumToUnderlying( stencil_store_op ) );
		hasher.Hash( EnumToUnderlying( initial_layout ) );
		hasher.Hash( EnumToUnderlying( final_layout ) );
		return hasher.GetHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkFormat												format = {};
	VkSampleCountFlagBits									samples = {};
	VkAttachmentLoadOp										load_op = {};
	VkAttachmentStoreOp										store_op = {};
	VkAttachmentLoadOp										stencil_load_op = {};
	VkAttachmentStoreOp										stencil_store_op = {};
	VkImageLayout											initial_layout = {};
	VkImageLayout											final_layout = {};

	size_t													hash = {};
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderPassSubpassAttachmentReference
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline													RenderPassSubpassAttachmentReference() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline													RenderPassSubpassAttachmentReference(
		uint32_t											attachment_index,
		VkImageLayout										layout,
		VkImageAspectFlags									aspect_mask
	) :
		attachment_index( attachment_index ),
		layout( layout ),
		aspect_mask( aspect_mask )
	{
		hash = CalculateHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline uint32_t											GetAttachmentIndex() const
	{
		return attachment_index;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkImageLayout									GetAttachmentLayout() const
	{
		return layout;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkImageAspectFlags								GetAttachmentAspectMask() const
	{
		return aspect_mask;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t											GetHash() const
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t											CalculateHash() const
	{
		Hasher hasher;
		hasher.Hash( attachment_index );
		hasher.Hash( layout );
		hasher.Hash( aspect_mask );
		return hasher.GetHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	uint32_t												attachment_index;
	VkImageLayout											layout;
	VkImageAspectFlags										aspect_mask;

	size_t													hash;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderPassSubpassCreateInfo
{
public:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline																RenderPassSubpassCreateInfo() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline																RenderPassSubpassCreateInfo(
		VkPipelineBindPoint												pipeline_bind_point,
		std::span<RenderPassSubpassAttachmentReference>					input_attachment_references,
		std::span<RenderPassSubpassAttachmentReference>					color_attachment_references,
		std::span<RenderPassSubpassAttachmentReference>					resolve_attachment_references,
		VkResolveModeFlags												resolve_depth_mode,
		VkResolveModeFlags												resolve_stencil_mode
	) :
		pipeline_bind_point( pipeline_bind_point ),
		input_attachment_references( input_attachment_references.begin(), input_attachment_references.end() ),
		color_attachment_references( color_attachment_references.begin(), color_attachment_references.end() ),
		resolve_attachment_references( resolve_attachment_references.begin(), resolve_attachment_references.end() ),
		resolve_depth_mode( resolve_depth_mode ),
		resolve_stencil_mode( resolve_stencil_mode )
	{
		hash = CalculateHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkPipelineBindPoint											GetPipelineBindPoint() const
	{
		return pipeline_bind_point;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const std::vector<RenderPassSubpassAttachmentReference>	&	GetInputAttachmentReferences() const
	{
		return input_attachment_references;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const std::vector<RenderPassSubpassAttachmentReference>	&	GetColorAttachmentReferences() const
	{
		return color_attachment_references;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const std::vector<RenderPassSubpassAttachmentReference>	&	GetResolveAttachmentReferences() const
	{
		return resolve_attachment_references;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkResolveModeFlags											GetResolveDepthMode() const
	{
		return resolve_depth_mode;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline VkResolveModeFlags											GetResolveStencilMode() const
	{
		return resolve_stencil_mode;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t														GetHash() const
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t														CalculateHash() const
	{
		Hasher hasher;
		hasher.Hash( pipeline_bind_point );
		for( auto & r : input_attachment_references ) hasher.Hash( r.GetHash() );
		for( auto & r : color_attachment_references ) hasher.Hash( r.GetHash() );
		for( auto & r : resolve_attachment_references ) hasher.Hash( r.GetHash() );
		hasher.Hash( resolve_depth_mode );
		hasher.Hash( resolve_stencil_mode );
		return hasher.GetHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VkPipelineBindPoint													pipeline_bind_point;
	std::vector<RenderPassSubpassAttachmentReference>					input_attachment_references;
	std::vector<RenderPassSubpassAttachmentReference>					color_attachment_references;
	std::vector<RenderPassSubpassAttachmentReference>					resolve_attachment_references;
	VkResolveModeFlags													resolve_depth_mode;
	VkResolveModeFlags													resolve_stencil_mode;

	size_t																hash;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderPassCreateInfo
{
public:

	using AttachmentList = std::vector<RenderPassAttachmentCreateInfo>;
	using SubpassList = std::vector<RenderPassSubpassCreateInfo>;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline 													RenderPassCreateInfo() = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline 													RenderPassCreateInfo(
		std::span<RenderPassAttachmentCreateInfo>			attachment_create_infos
	) :
		attachment_create_infos( attachment_create_infos.begin(), attachment_create_infos.end() )
	{
		hash = CalculateHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline 													RenderPassCreateInfo(
		const RenderPassCreateInfo						&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline RenderPassCreateInfo							&	operator=(
		const RenderPassCreateInfo						&	other
	) = default;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline bool												operator<(
		const RenderPassCreateInfo						&	other
	) const
	{
		return hash < other.GetHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const AttachmentList							&	GetAttachmentCreateInfos() const
	{
		return attachment_create_infos;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline const SubpassList							&	GetSubpassCreateInfos() const
	{
		return subpass_create_infos;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t											GetHash() const
	{
		return hash;
	}

private:

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline size_t											CalculateHash() const
	{
		Hasher hasher;
		for( auto & a : attachment_create_infos ) hasher.Hash( a.GetHash() );
		for( auto & a : subpass_create_infos ) hasher.Hash( a.GetHash() );

		return hasher.GetHash();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AttachmentList											attachment_create_infos;
	SubpassList												subpass_create_infos;

	size_t													hash;
};



} // vulkan
} // vk2d
