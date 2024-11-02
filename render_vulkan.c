#define USE_VALIDATION_LAYERS 0
#define R_VULKAN_CHECK_RES 1

typedef struct R_VULKAN_State R_VULKAN_State;
struct R_VULKAN_State
{
	Arena *arena;
	VkInstance instance;
	VkPhysicalDevice phys_device;
	VkDevice device;
	u32 q_main_family;
	VkQueue q_main;
	VkSurfaceKHR surface;
};

pub R_VULKAN_State *r_vulkan_state;

fn void r_vulkan_check_res_impl(VkResult res)
{
	if(res != 0)
	{
		printf("VkResult is not great success; code: %d\n", res);
		INVALID_CODE_PATH();
	}
}

#if R_VULKAN_CHECK_RES
#define r_vulkan_check_res(res) r_vulkan_check_res_impl(res)
#elif
#define r_vulkan_check_res(res)
#endif

PFN_vkCreateInstance vkCreateInstance;
PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;

fn void r_vulkanLoadFunctions()
{
	OS_Handle vkdll = os_vulkan_loadLibrary();
	
	vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)os_loadFunction(vkdll, "vkEnumerateInstanceVersion");
	
	vkCreateInstance = (PFN_vkCreateInstance)os_loadFunction(vkdll, "vkCreateInstance");
	
	vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)os_loadFunction(vkdll, "vkEnumeratePhysicalDevices");
	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)os_loadFunction(vkdll, "vkGetPhysicalDeviceProperties");
	// NOTE(mizu): Is it ever sensible to return this fp?
	os_vulkan_loadSurfaceFunction(vkdll);
	
	vkCreateDevice = (PFN_vkCreateDevice)os_loadFunction(vkdll,"vkCreateDevice");
	vkGetDeviceQueue = (PFN_vkGetDeviceQueue)os_loadFunction(vkdll,"vkGetDeviceQueue");
	
	vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2 )os_loadFunction(vkdll, "vkGetPhysicalDeviceFeatures2");
}

fn void r_vulkanInnit(OS_Handle handle)
{
	Arena *arena = arenaAlloc();
	r_vulkan_state = push_struct(arena, R_VULKAN_State);
	r_vulkan_state->arena = arena;
	
	VkResult res;
	
	// instance creation
	{
		u32 version = 0;
		
		vkEnumerateInstanceVersion(&version);
		
		printf("\nVulkan Version: %d.%d.%d\n"
									,VK_VERSION_MAJOR(version)
									,VK_VERSION_MINOR(version)
									,VK_VERSION_PATCH(version)
									);
		
		char *extentions[10] = {0};
		u32 extention_num = 0;
		
		char *validation_layers[1] = {0};
		u32 validation_layers_num = 0;
		
		//extentions and validation layers
		extentions[extention_num++] = VK_KHR_SURFACE_EXTENSION_NAME;
		extentions[extention_num++] = os_vulkan_surfaceExtentionName();
		
#if USE_VALIDATION_LAYERS
		
		validation_layers[validation_layers_num++] = (char*){
			"VK_LAYER_KHRONOS_validation"
		};
		
		extentions[extention_num++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		
#endif
		
		VkApplicationInfo app_info = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = 0,
			.pApplicationName = "Alfia",
			.applicationVersion = 1,
			.pEngineName = "Saoirse",
			.engineVersion = 1,
			.apiVersion = VK_API_VERSION_1_2
		};
		
		VkInstanceCreateInfo inst_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = 0,
			.flags = 0, // reserved for future use
			.pApplicationInfo = &app_info,
			.enabledLayerCount = validation_layers_num,
			.ppEnabledLayerNames = validation_layers,
			.enabledExtensionCount = extention_num,
			.ppEnabledExtensionNames = extentions
		};
		
		res = vkCreateInstance(&inst_info, 0, &r_vulkan_state->instance);
		r_vulkan_check_res(res);
	}
	
	// device selection
	{
		u32 count = 0;
		res = vkEnumeratePhysicalDevices(r_vulkan_state->instance, &count, 0);
		r_vulkan_check_res(res);
		printf("Device Count %d\n", count);
		
		VkPhysicalDevice *phys_devices = push_array(arena, VkPhysicalDevice, count);
		
		res = vkEnumeratePhysicalDevices(r_vulkan_state->instance, &count, phys_devices);
		
		r_vulkan_check_res(res);
		
		typedef struct good_gpu
		{
			b32 discrete;
			b32 good;
		}good_gpu;
		
		good_gpu *gpus = push_array(arena, good_gpu, count);
		
		for(u32 i = 0; i < count; i ++)
		{
			// we care for
			//dyn rendering, sync2, bda and descr indexing
			
			VkPhysicalDeviceProperties props = {0};
			vkGetPhysicalDeviceProperties(phys_devices[i], &props);
			
			VkPhysicalDeviceDynamicRenderingFeaturesKHR dyn_ren = 
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			};
			
			VkPhysicalDeviceSynchronization2FeaturesKHR sync2 = 
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
				.pNext = &dyn_ren
			};
			
			VkPhysicalDeviceVulkan12Features vk12_feat = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				.pNext = &sync2
			};
			
			VkPhysicalDeviceFeatures2 feet =
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
				.pNext = &vk12_feat
			};
			
			vkGetPhysicalDeviceFeatures2(phys_devices[i], &feet);
			
			if (dyn_ren.dynamicRendering && sync2.synchronization2 && vk12_feat.bufferDeviceAddress && vk12_feat.descriptorIndexing)
			{
				printf("dyn rendering, sync2, bda and descr indexing present ");
				gpus[i].good = 1;
				//compulsory_features[i] = true;
			}
			
			switch(props.deviceType)
			{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				{
					printf("discrete gpu\n");
					gpus[i].discrete = 1;
				}break;
				
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				{
					printf("integrated gpu\n");
				}break;
				default:
				{
					printf("other\n");
				}
			}
		}
		
		b32 found_gpu = 0;
		
		for(u32 i = 0; i < count; i ++)
		{
			printf("=========\n");
			printf("discrete: %d\n", gpus[i].discrete);
			printf("index: %d\n", i);
			printf("good: %d\n", gpus[i].good);
			printf("========\n");
			
			if(gpus[i].discrete && gpus[i].good)
			{
				found_gpu = 1;
				r_vulkan_state->phys_device = phys_devices[i];
				break;
			}
		}
		
		if(!found_gpu)
		{
			for(u32 i = 0; i < count; i ++)
			{
				if(gpus[i].good)
				{
					found_gpu = 1;
					r_vulkan_state->phys_device = phys_devices[i];
					break;
				}
			}
		}
		
		if(!found_gpu)
		{
			printf("Quitting. No good gpu\n");
			INVALID_CODE_PATH();
		}
	}
	
	res = os_vulkan_createSurface(handle, r_vulkan_state->instance, &r_vulkan_state->surface);
	r_vulkan_check_res(res);
	
	// logical device
	{
		const char* device_extention_names[2] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };
		
		f32 q_priorities[1] = {1.f};
		
		VkDeviceQueueCreateInfo q_info = 
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = r_vulkan_state->q_main_family,
			.queueCount = 1,
			.pQueuePriorities = q_priorities,
		};
		
		VkDeviceCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &q_info,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = 0,
			.enabledExtensionCount = 2,
			.ppEnabledExtensionNames = device_extention_names,
			.pEnabledFeatures = 0
		};
		
		res = vkCreateDevice(r_vulkan_state->phys_device, &info, 0, &r_vulkan_state->device);
		r_vulkan_check_res(res);
	}
	
	
	// queues
	{
		r_vulkan_state->q_main_family = 0;
		vkGetDeviceQueue(r_vulkan_state->device, r_vulkan_state->q_main_family, 0, &r_vulkan_state->q_main);
	}
	
	
	//arena_temp_end(&scratch);
}
