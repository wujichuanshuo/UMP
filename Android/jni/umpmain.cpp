#include <iomanip>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <android/looper.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <jni.h>
#include <unistd.h>

#include "buffer.h"
#include "umpmemory.h"
#include "umpserver.h"
#include "umputils.h"


using namespace std;

Il2CppManagedMemorySnapshot* (*umpCaptureMemorySnapshot_)(void);
void (*umpFreeCapturedMemorySnapshot_)(Il2CppManagedMemorySnapshot*);

ALooper* mainThreadLooper_;
int messagePipe_[2];
//io::buffer buffer((size_t)0, 128 * 1024 * 1024);

int umpMainThreadLooperCallback(int fd, int, void*) {
    UMPLOGI("begin start");
    char msg;
    read(fd, &msg, 1);


    if (umpCaptureMemorySnapshot_ != nullptr && umpFreeCapturedMemorySnapshot_ != nullptr) {
        UMPLOGI("Start memory snapshot");
        auto snapshot = umpCaptureMemorySnapshot_();
        UMPLOGI("Snapshot! heaps: %i, stacks: %i, types: %i, gcHandles: %i, %u", 
            snapshot->heap.sectionCount, snapshot->stacks.stackCount, snapshot->metadata.typeCount, snapshot->gcHandles.trackedObjectCount, kSnapshotMagicBytes);
        //io::bufferwriter writer(buffer);
        int i=1;
        string name;
        for(i=1;i<6555555;i++){
            std::stringstream ssTemp;  
            ssTemp<<i;  
            string number=ssTemp.str(); 
            name = "/sdcard/"+number+".rawsnapshot";
            if(access( name.c_str(), F_OK ) == -1){
                break;
            }
        }


        ofstream f(name, ios::binary | ios::trunc);

        f.write((char *)&kSnapshotMagicBytes,sizeof(kSnapshotMagicBytes));
        f.write((char *)&kSnapshotFormatVersion,sizeof(kSnapshotFormatVersion));

        f.write((char *)&kSnapshotHeapMagicBytes,sizeof(kSnapshotHeapMagicBytes));
        f.write((char *)&snapshot->heap.sectionCount,sizeof(snapshot->heap.sectionCount));
        for (std::uint32_t i = 0; i < snapshot->heap.sectionCount; i++) {
            auto& section = snapshot->heap.sections[i];
            f.write((char *)&section.sectionStartAddress, sizeof(section.sectionStartAddress));
            f.write((char *)&section.sectionSize, sizeof(section.sectionSize));
            //writer.append(section.sectionBytes, section.sectionSize);
            f.write((char*)section.sectionBytes, section.sectionSize);
        }
        UMPLOGI("writed heap");

        f.write((char *)&kSnapshotStacksMagicBytes, sizeof(kSnapshotStacksMagicBytes));
        f.write((char *)&snapshot->stacks.stackCount, sizeof(snapshot->stacks.stackCount));
        for (std::uint32_t i = 0; i < snapshot->stacks.stackCount; i++) {
            auto& stack = snapshot->stacks.stacks[i];
            f.write((char *)&stack.sectionStartAddress, sizeof(stack.sectionStartAddress));
            f.write((char *)&stack.sectionSize, sizeof(stack.sectionSize));
            //writer.append(&stack.sectionBytes, stack.sectionSize);
            f.write((char*)stack.sectionBytes, stack.sectionSize);
        }
        UMPLOGI("writed stack");

        f.write((char *)&kSnapshotMetadataMagicBytes, sizeof(kSnapshotMetadataMagicBytes));
        f.write((char *)&snapshot->metadata.typeCount, sizeof(snapshot->metadata.typeCount));
        for (std::uint32_t i = 0; i < snapshot->metadata.typeCount; i++) {
            auto& type = snapshot->metadata.types[i];
            f.write((char *)&(type.flags), sizeof(type.flags));
            f.write((char *)&type.baseOrElementTypeIndex, sizeof(type.baseOrElementTypeIndex));

            //UMPLOGI("write metadata type, type.name=%s.%s, fieldCount=%d, staticsSize=%d", type.assemblyName, type.name, (int)type.fieldCount, (int)type.staticsSize);
            if ((type.flags & Il2CppMetadataTypeFlags::kArray) == 0) {
                f.write((char*)&type.fieldCount, sizeof(type.fieldCount));

                for (std::uint32_t j = 0; j < type.fieldCount; j++) {
                    auto& field = type.fields[j];
                    f.write((char *)&field.offset, sizeof(field.offset));
                    f.write((char *)&field.typeIndex, sizeof(field.typeIndex));

                    int str_len = strlen(field.name) + 1;
                    f.write((char*)&str_len, sizeof(uint32_t));
                    f.write((char *)field.name, strlen(field.name)); // TODO
                    f << '\0';

                    f.write((char *)&field.isStatic, sizeof(field.isStatic));
                }
                f.write((char*)&type.staticsSize, sizeof(type.staticsSize));
                //writer.append(type.statics, type.staticsSize);
                f.write((char*)type.statics, type.staticsSize);
            }

            int str_len = strlen(type.name) + 1;
            f.write((char*)&str_len, sizeof(uint32_t));
            f.write((char *)type.name, strlen(type.name)); // TODO
            f << '\0';

            str_len = strlen(type.assemblyName) + 1;
            f.write((char*)&str_len, sizeof(uint32_t));
            f.write((char *)type.assemblyName, strlen(type.assemblyName)); // TODO
            f << '\0';

            f.write((char *)&type.typeInfoAddress, sizeof(type.typeInfoAddress));
            f.write((char *)&type.size, sizeof(type.size));
            //UMPLOGI("end write metadata type, type.name=%s.%s", type.assemblyName, type.name);
        }
        UMPLOGI("writed metadata");

        f.write((char *)&kSnapshotGCHandlesMagicBytes, sizeof(kSnapshotGCHandlesMagicBytes));
        f.write((char *)&snapshot->gcHandles.trackedObjectCount, sizeof(snapshot->gcHandles.trackedObjectCount));

        for (std::uint32_t i = 0; i < snapshot->gcHandles.trackedObjectCount; i++) {
            f.write((char *)&snapshot->gcHandles.pointersToObjects[i], sizeof(snapshot->gcHandles.pointersToObjects[i]));
        }
        UMPLOGI("writed gc handlers");

        f.write((char *)&kSnapshotRuntimeInfoMagicBytes, sizeof(kSnapshotRuntimeInfoMagicBytes));
        f.write((char *)&snapshot->runtimeInformation.pointerSize, sizeof(snapshot->runtimeInformation.pointerSize));
        f.write((char *)&snapshot->runtimeInformation.objectHeaderSize, sizeof(snapshot->runtimeInformation.objectHeaderSize));
        f.write((char *)&snapshot->runtimeInformation.arrayHeaderSize, sizeof(snapshot->runtimeInformation.arrayHeaderSize));
        f.write((char *)&snapshot->runtimeInformation.arrayBoundsOffsetInHeader, sizeof(snapshot->runtimeInformation.arrayBoundsOffsetInHeader));
        f.write((char *)&snapshot->runtimeInformation.arraySizeOffsetInHeader, sizeof(snapshot->runtimeInformation.arraySizeOffsetInHeader));
        f.write((char *)&snapshot->runtimeInformation.allocationGranularity, sizeof(snapshot->runtimeInformation.allocationGranularity));
        f.write((char *)&kSnapshotTailMagicBytes, sizeof(kSnapshotTailMagicBytes));
        UMPLOGI("writed runtime info handlers");


        f.close();
        UMPLOGI("+++++++++++++++++++++");
        umpSend(1); // blocking
        //buffer.clear();
        umpFreeCapturedMemorySnapshot_(snapshot);
    }
    return 1; // continue listening for events
}

void umpOnRecvMessage(unsigned int type, const char* data, unsigned int size) {
    (void)data;
    (void)size;
    if (type == UMPMessageType::CAPTURE_SNAPSHOT) {
        char empty = 255;
        write(messagePipe_[1], &empty, 1);
    }
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    UMPLOGI("JNI_OnLoad");
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }
    mainThreadLooper_ = ALooper_forThread(); // get looper for this thread
    ALooper_acquire(mainThreadLooper_); // add reference to keep object alive
    pipe(messagePipe_); //create send-receive pipe
    // listen for pipe read end, if there is something to read
    // - notify via provided callback on main thread
    ALooper_addFd(mainThreadLooper_, messagePipe_[0],
                  0, ALOOPER_EVENT_INPUT, umpMainThreadLooperCallback, nullptr);
    std::thread([]() {
        while (true) {
            std::string il2cppPath;
            if (umpIsLibraryLoaded("libil2cpp", il2cppPath)) {
                char *error;
                void *handle = dlopen(il2cppPath.c_str(), RTLD_LAZY);
                if (handle) {
                    dlerror();
                    *(void **) (&umpCaptureMemorySnapshot_) = dlsym(handle, "il2cpp_capture_memory_snapshot");
                    if ((error = dlerror()) != NULL)  {
                        UMPLOGE("Error dlsym il2cpp_capture_memory_snapshot: %s", error);
                    }
                    UMPLOGI("il2cpp_capture_memory_snapshot addr=%p", umpCaptureMemorySnapshot_);

                    dlerror();
                    *(void **) (&umpFreeCapturedMemorySnapshot_) = dlsym(handle, "il2cpp_free_captured_memory_snapshot");
                    if ((error = dlerror()) != NULL)  {
                        UMPLOGE("Error dlsym il2cpp_free_captured_memory_snapshot: %s", error);
                    }
                    UMPLOGI("il2cpp_free_captured_memory_snapshot addr=%p", umpFreeCapturedMemorySnapshot_);
                } else {
                    UMPLOGE("Error dlopen: %s", dlerror());
                }
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }).detach();
    auto ecode = umpServerStart(7100);
    umpRecv(umpOnRecvMessage);
    UMPLOGI("UMP server start status %i", ecode);
    return JNI_VERSION_1_6;
}

#ifdef __cplusplus
}
#endif // __cplusplus