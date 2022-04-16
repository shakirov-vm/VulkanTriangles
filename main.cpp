#include "App.h"

#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#ifndef ANALYZE
#define ANALYZE 1
#endif

// This is not definition?
auto Cleanup = [](GLFWwindow *window) {
    if (window) {
   		glfwDestroyWindow(window);
    }
    glfwTerminate();
};

constexpr int SZX = 600;
constexpr int SZY = 600;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// extensions list to query
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#define VK_CHECK_RESULT(function)                                                \
    {                                                                            \
        vk::Result Res = (vk::Result) (function);                                \
    	if (Res != vk::Result::eSuccess) {                                       \
      		std::ostringstream out;                                              \
     		out << "Vulkan error at " << __FILE__ << ":" << __LINE__ << "\n";    \
     		throw vulkan_error(Res, out.str());								     \
    	}                                                                        \
  	}


#define dbgs                                                                     \
    if (!ANALYZE) {                                                              \
    } else                                                                       \
    	std::cout


struct VKApp {

	std::unique_ptr<GLFWwindow, decltype(Cleanup)> window;

	vk::Instance Instance;
	vk::SurfaceKHR Surface;
    vk::PhysicalDevice PhysDevice;
    vk::Device Device;
    vk::Queue GraphicsQueue;
    vk::Queue PresentQueue;

	unsigned GraphicsFamily = -1u;
	unsigned PresentFamily = -1u;

	VKApp() : window(nullptr, Cleanup) {}

	void initialize_window();
	void create_instance();
	void peek_device();
    void find_queues();
    void create_logical_device();

	bool FramebufferResized;

  	~VKApp() {
		
		Instance.destroySurfaceKHR(Surface);
		Instance.destroy();
	}
};

struct glfw_error : public std::runtime_error {
    glfw_error(const char *str) : std::runtime_error(str) {}
};

struct vulkan_error : public std::runtime_error {
    vk::Result Res;
    vulkan_error(vk::Result res, std::string str) : std::runtime_error(str), Res(res) {}
};

void error_callback(int, const char *err_str) { throw glfw_error(err_str); }

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    auto App = reinterpret_cast<VKApp *>(glfwGetWindowUserPointer(window));
    App->FramebufferResized = true;
}

void VKApp::initialize_window() {

	glfwInit();
	glfwSetErrorCallback(error_callback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	auto *Window = glfwCreateWindow(SZX, SZY, "Triangles", NULL, NULL);
	assert(Window);

	glfwSetWindowUserPointer(Window, this);
	glfwSetFramebufferSizeCallback(Window, framebuffer_size_callback); 
	window.reset(Window);
}

void VKApp::create_instance() {


	unsigned glfwExtensionCount = 0;
	
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	dbgs << "Enumerated: " << glfwExtensionCount << " glfw required extensions\n";

	vk::ApplicationInfo appInfo ("Triangles", 1, "No Engine", 1, VK_API_VERSION_1_0);
	vk::InstanceCreateInfo createInfo({}, &appInfo, 0, nullptr, glfwExtensionCount, glfwExtensions);
	Instance = vk::createInstance(createInfo); //No validation layers

	VK_CHECK_RESULT(glfwCreateWindowSurface(Instance, window.get(), nullptr, (VkSurfaceKHR*) &Surface));
}

void VKApp::peek_device() {
	
	unsigned deviceCount = 0;
	
	// list of devices
	PhysDevice = Instance.enumeratePhysicalDevices().front();

	//Instance.enumeratePhysicalDevices();

	/*VK_CHECK_RESULT(vk::EnumeratePhysicalDevices(Instance, &deviceCount, nullptr));
	if (deviceCount != 1) // =)
	    throw std::runtime_error("Multiple Vulkan devices not supported yet");

	dbgs << deviceCount << " devices enumerated\n";
	VK_CHECK_RESULT(vk::EnumeratePhysicalDevices(Instance, &deviceCount, &PhysDevice));*/
}

void VKApp::find_queues() {
	
	std::vector<vk::QueueFamilyProperties> queueFamilies = PhysDevice.getQueueFamilyProperties();
	dbgs << queueFamilies.size() << " queue families found\n";

	unsigned i = 0;

	for(const auto &queueFamily : queueFamilies) { // maybe this is algorithm?

		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
		    GraphicsFamily = i;
		    dbgs << "Graphics queue family: " << i << std::endl;
		}

		if (PhysDevice.getSurfaceSupportKHR(i, Surface)) {
		    PresentFamily = i;
		    dbgs << "Present queue family: " << i << std::endl;
		}
		
		if (PresentFamily != -1u && GraphicsFamily != -1u) {
		    break;
		}
		
		i += 1;
	}

	if (PresentFamily == -1u || GraphicsFamily == -1u)
		throw std::runtime_error("Present and Graphics not found");
}

void VKApp::create_logical_device() { // ??
	
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	float queuePriority = 1.0f;

	vk::DeviceQueueCreateInfo queueCreateInfoPresent(vk::DeviceQueueCreateFlags(), PresentFamily, 1, &queuePriority);
	vk::DeviceQueueCreateInfo queueCreateInfoGraphics(vk::DeviceQueueCreateFlags(), GraphicsFamily, 1, &queuePriority);
	queueCreateInfos.push_back(queueCreateInfoPresent);
	queueCreateInfos.push_back(queueCreateInfoGraphics);	

	// note: we are querying no device features ??
	vk::PhysicalDeviceFeatures deviceFeatures{}; // ?
	dbgs << deviceExtensions.size() << " device extensions to enable\n";
	
	//TODO: ARRAY_PROXY
	vk::DeviceCreateInfo createInfo{};
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	Device = PhysDevice.createDevice(createInfo);

	Device.getQueue(GraphicsFamily, 0, &GraphicsQueue);
	Device.getQueue(PresentFamily, 0, &PresentQueue);
}

int main() {

	VKApp App;
    App.initialize_window();
	App.create_instance();
	App.peek_device();
    App.find_queues();
    App.create_logical_device();

	sleep(3);
	std::cout << "Success exit" << std::endl;
}