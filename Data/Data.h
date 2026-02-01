#ifndef _DATA_H
#define _DATA_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Globals.h"

#include "Utils/FileUtils.h"

#include "Data/Project.h"

#define DEFAULT_BLOCK_CAPACITY 1024

class Data
{
public:
    virtual ~Data() {}

private:

	// Engine Interface
	bool LoadBlock(Project* project, u32 idx);

	int GetValue(Project* project, u32 idx, u32 field);
	u32 SetValue(u32 idx, u32 field, int value, u32 actionIdx);
	u32 GetCount(Project* project, u32 idx);
	u32 GetConcat(u32 idx, u32 concatIdx) const;

	u32 GetBlockCount();

	void ReverseAction(u32 actionIdx);
	void Save(Project* project);

	bool HasSaveEvents();

	// Structs
	struct Section
	{
		Section() : storedDir(string()), start(0), size(0), load(nullptr), save(nullptr), order(nullptr) {}
		Section(const string& storedDir, u32 start, u32 size, 
			bool (*load)(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream), 
			bool (*save)(const vector<int> data, FileStream& fileStream), 
			bool (*order)(vector<int>& data, u32 start, u32 size, u32& field) = nullptr) :
			storedDir(storedDir), start(start), size(size), load(load), save(save), order(order) {}
		Section(const Section& other) :
			storedDir(other.storedDir), start(other.start), size(other.size), load(other.load), 
			save(other.save), order(other.order) {}
		const Section& operator=(const Section& other)
		{
			this->storedDir = other.storedDir;
			this->start = other.start;
			this->size = other.size;
			this->load = other.load;
			this->save = other.save;
			this->order = other.order;
			return *this;
		}

		// Directory where the section data is stored
		string storedDir;

		// First index of the section in the block data
		u32 start;
		// Size of the section in the block data
		u32 size;

		// Functions that load/save data from/to files
		bool (*load)(vector<int>& data, vector<u32>& concatIndices, const FileStream& fileStream);
		bool (*save)(const vector<int> data, FileStream& fileStream);

		// Function to order the data
		bool (*order)(vector<int>& data, u32 start, u32 size, u32& field);
	};
	struct Block
	{
		Block(): original(vector<int>()), current(vector<int>()), concatIndices(vector<u32>()) {}
		Block(vector<int> original, vector<int> current, vector<u32> concatIndices) :
			original(original), current(current), concatIndices(concatIndices) {}
		Block(const Block& other) :
			original(other.original), current(other.current), concatIndices(other.concatIndices) {}
		const Block& operator=(const Block& other)
		{
			this->original = other.original;
			this->current = other.current;
			this->concatIndices = other.concatIndices;
			return *this;
		}

		// Original iteration of the block
		vector<int> original;
		// Current iteration of the block after modifications
		vector<int> current;

		// Indices of all the concatenated data (fist index matches the map key)
		vector<u32> concatIndices;
	};
	struct ReverseEvent
	{
		ReverseEvent() : actionIdx(0), idx(0), field(0), value(0) {}
		ReverseEvent(u32 actionIdx, u32 idx, u32 field, int value) : 
			actionIdx(actionIdx), idx(idx), field(field), value(value) {}
		ReverseEvent(const ReverseEvent& other) :
			actionIdx(other.actionIdx), idx(other.idx), field(other.field), value(other.value) {}
		const ReverseEvent& operator=(const ReverseEvent& other)
		{
			this->actionIdx = other.actionIdx;
			this->idx = other.idx;
			this->field = other.field;
			this->value = other.value;
			return *this;
		}

		u32 actionIdx;

		u32 idx;
		u32 field;
		int value;
	};
	struct SaveEvent
	{
		SaveEvent() : idx(0), concatIdx(0), sectionIdx(0) {}
		SaveEvent(u32 idx, u32 concatIdx, u32 sectionIdx) :
			idx(idx), concatIdx(concatIdx), sectionIdx(sectionIdx) {}
		SaveEvent(const SaveEvent& other) :
			idx(other.idx), concatIdx(other.concatIdx), sectionIdx(other.sectionIdx) {}
		const SaveEvent& operator=(const SaveEvent& other)
		{
			this->idx = other.idx;
			this->concatIdx = other.concatIdx;
			this->sectionIdx = other.sectionIdx;
			return *this;
		}

		u32 idx;
		u32 concatIdx;
		u32 sectionIdx;
	};

	Section* GetFieldData(u32 field, u32& concatIdx, u32& secIdx);
	bool IsNullData(const vector<int>& data);

	// Private Data
	unordered_map<u32, Block> blocks;
	vector<ReverseEvent> reverseEvents;
	vector<SaveEvent> saveEvents;

protected:

	bool InsertSaveEvent(u32 idx, u32 concatIdx, u32 secIdx);

	enum DataFlag
	{
		No_Flags = 0x0,
		// Enables full acces of the block data to the sections
		Full_Access = 0x1,
		// Data is stored in 1 block of undefined size
		Single_Block = 0x2,
	};
	bool CheckFlag(DataFlag flag);

	virtual void GenerateSections(const vector<string>& narcPaths) = 0;
	
	// Protected Data
	u32 flags;
	// Size of a single block (not concatenated)
	u32 blockSize;
	// Value that represents an empty data slot
	int nullValue;

	vector<Section> sections;

	friend class Engine;
};

#endif // _DATA_H
