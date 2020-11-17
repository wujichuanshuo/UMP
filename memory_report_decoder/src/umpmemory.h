#ifndef UMPMEMORY_H
#define UMPMEMORY_H

//#include <QtEndian>
#include <cstdint>
#include <cstring>

const uint32_t kSnapshotFormatVersion = 4;
const uint32_t kSnapshotMagicBytes = 0xFABCED01;
const uint32_t kSnapshotHeapMagicBytes = 0x9111DAAA;
const uint32_t kSnapshotStacksMagicBytes = 0x147358AA;
const uint32_t kSnapshotMetadataMagicBytes = 0x4891AEFD;
const uint32_t kSnapshotGCHandlesMagicBytes = 0x3456132C;
// const uint32_t kSnapshotNativeTypesMagicBytes = 0x78514753;
// const uint32_t kSnapshotNativeObjectsMagicBytes = 0x6173FAFE;
const uint32_t kSnapshotRuntimeInfoMagicBytes = 0x0183EFAC;
const uint32_t kSnapshotTailMagicBytes = 0x865EEAAF;



struct Il2CppMetadataField
{
    uint32_t offset;
    uint32_t typeIndex;
    char* name;
    bool isStatic;
};

enum Il2CppMetadataTypeFlags
{
    kNone = 0,
    kValueType = 1 << 0,
    kArray = 1 << 1,
    kArrayRankMask = 0xFFFF0000
};

struct Il2CppMetadataType
{
    Il2CppMetadataTypeFlags flags;  // If it's an array, rank is encoded in the upper 2 bytes
    Il2CppMetadataField* fields;
    uint32_t fieldCount;
    uint32_t staticsSize;
    uint8_t* statics;
    uint32_t baseOrElementTypeIndex;
    char* name;
    char* assemblyName;
    uint64_t typeInfoAddress;
    uint32_t size;
    uint32_t typeIndex;
};

struct Il2CppMetadataSnapshot
{
    uint32_t typeCount;
    Il2CppMetadataType* types;
};

struct Il2CppManagedMemorySection
{
    uint64_t sectionStartAddress;
    uint32_t sectionSize;
    uint8_t* sectionBytes;
};

struct Il2CppManagedHeap
{
    uint32_t sectionCount;
    Il2CppManagedMemorySection* sections;
};

struct Il2CppStacks
{
    uint32_t stackCount;
    Il2CppManagedMemorySection* stacks;
};

struct NativeObject
{
    uint32_t gcHandleIndex;
    uint32_t size;
    uint32_t instanceId;
    uint32_t classId;
    uint32_t referencedNativeObjectIndicesCount;
    uint32_t* referencedNativeObjectIndices;
};

struct Il2CppGCHandles
{
    uint32_t trackedObjectCount;
    uint64_t* pointersToObjects;
};

struct Il2CppRuntimeInformation
{
    uint32_t pointerSize;
    uint32_t objectHeaderSize;
    uint32_t arrayHeaderSize;
    uint32_t arrayBoundsOffsetInHeader;
    uint32_t arraySizeOffsetInHeader;
    uint32_t allocationGranularity;
};

struct Il2CppManagedMemorySnapshot
{
    Il2CppManagedHeap heap;
    Il2CppStacks stacks;
    Il2CppMetadataSnapshot metadata;
    Il2CppGCHandles gcHandles;
    Il2CppRuntimeInformation runtimeInformation;
    void* additionalUserInformation;
};

// read big-endian data to little-endian
class bufferreader {
public:
    using size_type = size_t;

    bufferreader(const char* data, size_type size, bool isBigEndian);
	void EndianSwap(std::uint8_t *pData, int startIndex, int length);
    bool read(char* data, size_type size);
    bool atEnd() const;

    bufferreader& operator>> (std::uint32_t& value);
    bufferreader& operator>> (std::uint64_t& value);
    bufferreader& operator>> (char*& value);
    bufferreader& operator>> (bool& value);


private:
    const char* data_;
    size_type size_;
    size_type index_ = 0;
	bool isBigEndian_ = true;
};

inline void bufferreader::EndianSwap(std::uint8_t *pData, int startIndex, int length)
{
	int i, cnt, end, start;
	cnt = length / 2;
	start = startIndex;
	end = startIndex + length - 1;
	uint8_t tmp;
	for (i = 0; i < cnt; i++)
	{
		tmp = pData[start + i];
		pData[start + i] = pData[end - i];
		pData[end - i] = tmp;
	}
}

inline bufferreader::bufferreader(const char* data, size_type size, bool isBigEndian) : data_(data), size_(size), isBigEndian_(isBigEndian) {}

inline bool bufferreader::atEnd() const {
    return index_ >= size_;
}

inline bool bufferreader::read(char* data, size_type size) {
    if (index_ + size > size_)
        return false;
    memcpy(data, &data_[index_], size);
    index_ += size;
    return true;
}

inline bufferreader& bufferreader::operator>> (std::uint32_t& value) {
    if (index_ + 4 <= size_) {
        memcpy(reinterpret_cast<char*>(&value), &data_[index_], 4);
		if (isBigEndian_) {
			int len = sizeof(value);
			std::uint8_t* pData = (std::uint8_t*)(&value);
			for (int i = 0; i < len; i += 4)
			{
				EndianSwap(pData, i, 4);
			}
		}
        index_ += 4;
    }
    return *this;
}

inline bufferreader& bufferreader::operator>> (std::uint64_t& value) {
    if (index_ + 8 <= size_) {
        memcpy(reinterpret_cast<char*>(&value), &data_[index_], 8);
		if (isBigEndian_) {
			int len = sizeof(value);
			std::uint8_t* pData = (std::uint8_t*)(&value);
			for (int i = 0; i < len; i += 4)
			{
				EndianSwap(pData, i, 4);
			}
		}
        index_ += 8;
    }
    return *this;
}

inline bufferreader& bufferreader::operator>> (char*& value) {
    std::uint32_t len;
    *this >> len;
    if (index_ + len <= size_ && len > 0) {
        value = new char[len];
        memcpy(value, &data_[index_], len);
        index_ += len;
    }
    return *this;
}

inline bufferreader& bufferreader::operator>> (bool& value) {
    if (index_ + 1 <= size_) {
        value = data_[index_];
        index_++;
    }
    return *this;
}

#endif
