#ifndef __INTEL_XE_METAL_PLUGIN_HPP__
#define __INTEL_XE_METAL_PLUGIN_HPP__

#include <IOKit/IOService.h>

struct MTLTextureDescriptor;
typedef uint32_t MTLResourceOptions;
struct IOGraphicsDevice;

class IntelXeMetalPlugin : public IOService {
    OSDeclareDefaultStructors(IntelXeMetalPlugin)
    
public:
    /* IOGraphicsAccelerator2 Overrides */
    virtual bool init(OSDictionary* properties) override;
    virtual bool start(IOService* provider) override;
    virtual void stop(IOService* provider) override;
    virtual void free() override;
    
    /* Metal Device Interface */
    virtual IOReturn createMetalDevice(IOGraphicsDevice* device, void** outDevice);
    virtual IOReturn destroyMetalDevice(void* device);
    
    /* Resource Management */
    virtual IOReturn allocateTexture(void* device, MTLTextureDescriptor* desc, void** outTexture);
    virtual IOReturn allocateBuffer(void* device, size_t length, MTLResourceOptions options, void** outBuffer);
    virtual IOReturn deallocateResource(void* device, void* resource);
    
    /* Command Submission */
    virtual IOReturn submitCommandBuffer(void* device, void* commandBuffer);
    virtual IOReturn waitUntilCompleted(void* device, void* commandBuffer);
    
    /* Query Capabilities */
    virtual IOReturn getDeviceCapabilities(void* device, void* capabilities);
    virtual IOReturn getFeatureSet(void* device, void* featureSet);
    
    /* Memory Management */
    virtual IOReturn createSharedMemory(size_t size, void** outMemory);
    virtual IOReturn destroySharedMemory(void* memory);
    
private:
    /* Parent accelerator */
    IOService* fParentAccelerator;
    
    /* Metal device handle */
    void* fMetalDevice;
    
    /* Device capabilities */
    struct {
        bool supportsMetal3 : 1;
        bool supportsRayTracing : 1;
        bool supportsMeshShaders : 1;
        bool supportsVariableRateShading : 1;
        bool supportsTileShaders : 1;
        UInt32 maxTextureSize;
        UInt32 maxBufferLength;
        UInt32 maxThreadsPerThreadgroup;
    } fCapabilities;
    
    /* Initialize capabilities */
    void initializeCapabilities();
    
    /* GPU memory pools */
    IOBufferMemoryDescriptor* fTexturePool;
    IOBufferMemoryDescriptor* fBufferPool;
    size_t fTexturePoolOffset; // bump allocator offset
    size_t fBufferPoolOffset;
    
    /* Statistics */
    UInt64 fAllocatedTextures;
    UInt64 fAllocatedBuffers;
    UInt64 fTotalMemoryAllocated;
};

#endif /* __INTEL_XE_METAL_PLUGIN_HPP__ */