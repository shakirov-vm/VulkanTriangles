#include <iostream>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#ifndef ANALYZE
#define ANALYZE 1
#endif

auto Cleanup = [](GLFWwindow *window) {
  if (window) {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
};

constexpr int SZX = 600;
constexpr int SZY = 600;

// TODO:
#define VK_CHECK_RESULT(function)                                                \
    {                                                                            \
        vk::Result Res = (function);                                             \
    	if (Res != vk::Result::eSuccess) {                                                   \
      		std::ostringstream out;                                              \
     		out << "Vulkan error at " << __FILE__ << ":" << __LINE__ << "\n";    \
    	}                                                                        \
  	}

     		/*throw vulkan_error(Res, out.str());*/							      /*TODO*/

#define dbgs                                                                     \
    if (!ANALYZE) {                                                              \
    } else                                                                       \
    	std::cout

struct VKApp {

	//std::unique_ptr<GLFWwindow, decltype(Cleanup)> window;
	//GLFWwindow* window; // TODO

	vk::Instance Instance;
	//VkPhysicalDevice PhysDevice;
	//VkDevice Device;
	vk::SurfaceKHR Surface;
	/*VkQueue GraphicsQueue;
	VkQueue PresentQueue;
	VkSwapchainKHR SwapChain;
	VkExtent2D Extent;
	VkSurfaceFormatKHR SurfaceFormat;
	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;
	VkRenderPass RenderPass;
	VkPipelineLayout PipelineLayout;
	VkPipeline GraphicsPipeline;
	VkCommandPool CommandPool;
	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;
	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;
	std::vector<VkCommandBuffer> CommandBuffers;
	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> InFlightFences;
	std::vector<VkFence> ImagesInFlight;
	size_t CurrentFrame = 0;
	VkShaderModule StoredVertexID, StoredFragmentID;

	unsigned PresentFamily = -1u;
	unsigned GraphicsFamily = -1u;

	*/

	void initialize_window();
	void create_instance();
	/*void peek_device();
	void find_queues();
	void create_logical_device();

	VkShaderModule installShader(std::vector<char> ShaderCode);

	void create_swap_chain();
	void create_image_views();
	void create_render_pass();
	void create_descset_layout();
	void create_pipeline(VkShaderModule VertexID, VkShaderModule FragmentID);
	void create_frame_buffer();
	void create_command_pool();
	void create_buffers();
	void create_command_buffers();
	void create_sync_objs();

	void render_frame();
	void run();

	unsigned findMemoryType(unsigned typeFilter,
	                      VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	                VkMemoryPropertyFlags properties, VkBuffer &buffer,
	                VkDeviceMemory &bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	bool FramebufferResized;

	VkApp() : Wnd(nullptr, Cleanup) {}

	void update_swap_chain(); // really create from scratch

	void cleanup_swap_chain() {
	for (auto framebuffer : SwapChainFramebuffers)
	    vkDestroyFramebuffer(Device, framebuffer, nullptr);

	vkFreeCommandBuffers(Device, CommandPool, CommandBuffers.size(),
	                     CommandBuffers.data());

	vkDestroyPipeline(Device, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	vkDestroyRenderPass(Device, RenderPass, nullptr);

	for (auto imageView : SwapChainImageViews)
	    vkDestroyImageView(Device, imageView, nullptr);

		vkDestroySwapchainKHR(Device, SwapChain, nullptr);
	}

	~VkApp() {
		cleanup_swap_chain();

		// TODO: to "unique pointers"
		vkDestroyDevice(Device, nullptr);
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		vkDestroyInstance(Instance, nullptr);
	}*/
};

// custom error handler class
struct glfw_error : public std::runtime_error {
    glfw_error(const char *s) : std::runtime_error(s) {}
};

// vulkan-specific error (knows error code)
struct vulkan_error : public std::runtime_error {
    VkResult Res;
    vulkan_error(VkResult R, std::string S) : std::runtime_error(S), Res(R) {}
};

// throw on errors
void error_callback(int, const char *err_str) { throw glfw_error(err_str); }
/*
// make sure the viewport matches the new window dimensions
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    auto App = reinterpret_cast<VKApp *>(glfwGetWindowUserPointer(window));
    //App->FramebufferResized = true; // TODO
}*/

void VKApp::initialize_window() {

	glfwInit();
	glfwSetErrorCallback(error_callback);

	// this is interesting:
	// GLFW_NO_API required to NOT create OpenGL context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto *Window = glfwCreateWindow(SZX, SZY, "Hello, Vulkan", NULL, NULL);
	assert(Window); // error callback shall throw otherwise
	// so no need to call glfwMakeContextCurrent
	glfwSetWindowUserPointer(Window, this);
	//glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback); TODO
	//window->reset(Window); TODO
}

void VKApp::create_instance() {
							// Is it correct?
	vk::ApplicationInfo appInfo ("Triangles", VK_MAKE_VERSION(1, 0, 0), "No Engine",
								 VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	unsigned glfwExtensionCount = 0;
	
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	dbgs << "Enumerated: " << glfwExtensionCount << " glfw required extensions\n";

												//No validation layers
	vk::InstanceCreateInfo createInfo({}, &appInfo, 0, nullptr, glfwExtensionCount, glfwExtensions);

	//vk::createInstance(&createInfo, nullptr, &Instance);
	//TODO:
	/*VK_CHECK_RESULT();*/
	//glfwCreateWindowSurface(Instance, window.get(), nullptr, &Surface);
	/*VK_CHECK_RESULT();*/
}

int main() {

	VKApp App;
	App.create_instance();
}