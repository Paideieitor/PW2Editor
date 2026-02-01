#include "System.h"

#include "Utils/NarcUtils.h"

#include "Data/Data.h"

bool Data::LoadBlock(Project* project, u32 idx)
{
	vector<int> data;
	vector<u32> concatIndices = { idx };

	data.reserve(DEFAULT_BLOCK_CAPACITY);
	for (u32 concatIdx = 0; concatIdx < (u32)concatIndices.size(); ++concatIdx)
	{
		u32 currentIdx = concatIndices[concatIdx];

		if (CheckFlag(Single_Block))
		{
			// Single block data is only compatible with single section data
			Section& section = sections.at(0);

			FileStream fileStream = LoadFileFromNarc(project->ctrMapProjectDir, project->romDir, section.storedDir, currentIdx);

			// Single block data allocates necesary data at load time
			if (fileStream.data && !section.load(data, concatIndices, fileStream))
			{
				Log(WARNING, "Failed loading block %d, section %d", currentIdx, 0);
				Log(WARNING, "    Couldn't load data to single block");
				return false;
			}
		
			// Single block data loads all the concats in 1 load call
			break;
		}

		for (u32 secIdx = 0; secIdx < sections.size(); ++secIdx)
		{
			Section& section = sections.at(secIdx);
		
			FileStream fileStream = LoadFileFromNarc(project->ctrMapProjectDir, project->romDir, section.storedDir, currentIdx);
		
			vector<int> sectionData;
			sectionData.resize(section.size);
			for (u32 idx = 0; idx < (u32)sectionData.size(); ++idx)
				sectionData.at(idx) = nullValue;
		
			if (!CheckFlag(Full_Access))
			{
				if (fileStream.data && !section.load(sectionData, concatIndices, fileStream))
				{
					Log(WARNING, "Failed loading block %d, section %d", currentIdx, secIdx);
					Log(WARNING, "    Couldn't load data to block");
					return false;
				}
		
				data.insert(data.end(), sectionData.begin(), sectionData.end());
			}
			else
			{
				data.insert(data.end(), sectionData.begin(), sectionData.end());
		
				// Full access data passes the all the data to the load function
				// since the sections are interdependent
				if (fileStream.data && !section.load(data, concatIndices, fileStream))
				{
					Log(WARNING, "Failed loading block %d, section %d", currentIdx, secIdx);
					Log(WARNING, "    Couldn't load data to block");
					return false;
				}
			}
		}
	}
	data.shrink_to_fit();

	blocks[idx] = { data, data, concatIndices };
	return true;
}

int Data::GetValue(Project* project, u32 idx, u32 field)
{
	unordered_map<u32, Block>::const_iterator itr = blocks.find(idx);
	if (itr == blocks.end())
	{
		if (!LoadBlock(project, idx))
		{
			Log(CRITICAL, "Couldn't load data block %d (%d)", idx, field);
			return nullValue;
		}
		itr = blocks.find(idx);
	}

	const Block& block = (*itr).second;
	if (field >= block.current.size())
	{
		Log(WARNING, "Trying to access out of reach index %d in block %d", field, idx);
		return nullValue;
	}
	return block.current.at(field);
}

u32 Data::SetValue(u32 idx, u32 field, int value, u32 actionIdx)
{
	unordered_map<u32, Block>::const_iterator itr = blocks.find(idx);
	if (itr == blocks.end())
	{
		Log(CRITICAL, "Trying to modify unloaded data %d (%d)", idx, field);
		return field;
	}

	vector<int>& data = blocks.at(idx).current;
	if (field >= blocks.at(idx).current.size())
	{
		Log(WARNING, "Trying to modify out of reach index %d in block %d", field, idx);
		return field;
	}
	int& currentValue = data.at(field);
	if (currentValue == value)
		return field;

	u32 concatIdx;
	u32 secIdx;
	Section* section = GetFieldData(field, concatIdx, secIdx);
	if (!section)
	{
		Log(CRITICAL, "Unable to find section for %d (%d)", idx, field);
		return field;
	}

	ReverseEvent& reverse = reverseEvents.emplace_back(actionIdx, idx, field, currentValue);
	currentValue = value;

	// The return value can get modified by this function pointer
	if (section->order && section->order(data, (concatIdx * blockSize) + section->start, section->size, field))
	{
		// Update the position of the change after ordering the section
		reverse.field = field;
	}
		
	InsertSaveEvent(idx, concatIdx, secIdx);
	return field;
}

u32 Data::GetCount(Project* project, u32 idx)
{
	unordered_map<u32, Block>::const_iterator itr = blocks.find(idx);
	if (itr == blocks.end())
	{
		if (!LoadBlock(project, idx))
		{
			Log(CRITICAL, "Couldn't load data block %d (%d)", idx);
			return nullValue;
		}
		itr = blocks.find(idx);
	}
	const Block& block = (*itr).second;

	u32 count = (u32)block.concatIndices.size();
	if (count != block.current.size() / blockSize)
	{
		Log(CRITICAL, "Missmatched count with actual data in %d", idx);
		return 0;
	}

	return count;
}

u32 Data::GetConcat(u32 idx, u32 concatIdx) const
{
	unordered_map<u32, Block>::const_iterator itr = blocks.find(idx);
	if (itr == blocks.end())
	{
		Log(CRITICAL, "Trying to get count of unloaded data %d", idx);
		return 0;
	}
	const Block& block = (*itr).second;

	if (concatIdx >= (u32)block.concatIndices.size())
	{
		Log(CRITICAL, "Concat index out of scope in %d", idx);
		return 0;
	}

	return block.concatIndices.at(concatIdx);
}

u32 Data::GetBlockCount()
{
	return (u32)blocks.size();
}

void Data::ReverseAction(u32 actionIdx)
{
	int initialSize = (int)reverseEvents.size() - 1;
	
	int revIdx = initialSize;
	for (; revIdx >= 0; --revIdx)
	{
		ReverseEvent& reverseEvent = reverseEvents.at(revIdx);
		if (reverseEvent.actionIdx >= actionIdx)
		{
			u32 concatIdx;
			u32 secIdx;
			Section* section = GetFieldData(reverseEvent.field, concatIdx, secIdx);
			if (!section)
			{
				Log(CRITICAL, "Unable to find section for reverse event %d (%d)", reverseEvent.idx, reverseEvent.field);
				continue;
			}

			vector<int>& data = blocks.at(reverseEvent.idx).current;
			data.at(reverseEvent.field) = reverseEvent.value;

			if (section->order)
				section->order(data, (concatIdx * blockSize) + section->start, section->size, reverseEvent.field);

			InsertSaveEvent(reverseEvent.idx, concatIdx, secIdx);
		}
		else
			break;
	}
	
	if (revIdx != initialSize)
	{
		if (revIdx >= 0)
			reverseEvents.erase(reverseEvents.begin() + revIdx + 1, reverseEvents.end());
		else
			reverseEvents.clear();
	}
}

void Data::Save(Project* project)
{
	for (u32 idx = 0; idx < saveEvents.size(); ++idx)
	{
		SaveEvent& save = saveEvents.at(idx);

		unordered_map<u32, Block>::const_iterator itr = blocks.find(save.idx);
		if (itr == blocks.end())
		{
			Log(CRITICAL, "Trying to save unloaded data %d (%d)", save.idx, save.concatIdx);
			continue;
		}
		const Block& block = (*itr).second;
		if (block.current == block.original)
			continue;

		if (save.concatIdx >= (u32)block.concatIndices.size())
		{
			Log(CRITICAL, "Trying to save an out of scope concatentation %d", save.concatIdx);
			continue;
		}
		const u32 saveIdx = block.concatIndices.at(save.concatIdx);

		if (!CheckFlag(Full_Access))
		{
			if (save.sectionIdx >= (u32)sections.size())
			{
				Log(CRITICAL, "Trying to save using out of scope section %d in %d", save.sectionIdx, saveIdx);
				continue;
			}

			Section& section = sections.at(save.sectionIdx);
			vector<int> data(block.current.begin() + section.start, block.current.begin() + section.start + section.size);
			if (IsNullData(data))
			{
				Log(WARNING, "Trying to save null data at %d, section %d", saveIdx, save.sectionIdx);
				continue;
			}

			FileStream fileStream;
			if (!LoadEmptyFileStream(fileStream))
			{
				Log(CRITICAL, "Failed to create file stream while saving %d, section %d", saveIdx, save.sectionIdx);
				continue;
			}

			if (!section.save(data, fileStream))
			{
				ReleaseFileStream(fileStream);
				Log(CRITICAL, "Failed to save %d, section %d", saveIdx, save.sectionIdx);
				continue;
			}

			string savePath = PathConcat(PathConcat(project->ctrMapProjectDir, CTRMAP_FILESYSTEM_DIR), PathConcat(section.storedDir, to_string(saveIdx)));
			SaveFileStream(fileStream, savePath);
			ReleaseFileStream(fileStream);
		}
		else
		{
			// Full access data saves every section a change
			// since changes in one section could affect the others
			for (u32 secIdx = 0; secIdx < (u32)sections.size(); ++secIdx)
			{
				Section& section = sections.at(secIdx);
				
				FileStream fileStream;
				if (!LoadEmptyFileStream(fileStream))
				{
					Log(CRITICAL, "Failed to create file stream while saving %d, section %d", saveIdx, save.sectionIdx);
					continue;
				}

				if (!section.save(block.current, fileStream))
				{
					ReleaseFileStream(fileStream);
					Log(CRITICAL, "Failed to save %d, section %d", saveIdx, save.sectionIdx);
					continue;
				}

				string savePath = PathConcat(PathConcat(project->ctrMapProjectDir, CTRMAP_FILESYSTEM_DIR), PathConcat(section.storedDir, to_string(saveIdx)));
				SaveFileStream(fileStream, savePath);
				ReleaseFileStream(fileStream);
			}
		}
	}

	saveEvents.clear();
}

bool Data::HasSaveEvents()
{
	return !saveEvents.empty();
}

Data::Section* Data::GetFieldData(u32 field, u32& concatIdx, u32& secIdx)
{
	concatIdx = field / blockSize;
	u32 sectionField = field % blockSize;
	for (secIdx = 0; secIdx < (u32)sections.size(); ++secIdx)
	{
		Section& section = sections.at(secIdx);
		if (sectionField >= section.start && sectionField <= (section.start + section.size - 1))
			return &section;
	}
	return nullptr;
}

bool Data::IsNullData(const vector<int>& data)
{
	for (u32 idx = 0; idx < (u32)data.size(); ++idx)
		if (data.at(idx) != nullValue)
			return false;
	return true;
}

bool Data::InsertSaveEvent(u32 idx, u32 concatIdx, u32 secIdx)
{
	// Discard save event if it has already been scheaduled
	for (u32 saveIdx = 0; saveIdx < (u32)saveEvents.size(); ++saveIdx)
	{
		const SaveEvent& saveEvent = saveEvents.at(saveIdx);
		if (saveEvent.idx == idx &&
			saveEvent.concatIdx == concatIdx)
		{
			// Full access data doesn't discriminate save events by section
			// since any change triggers a save of all the sections
			if (CheckFlag(Full_Access) ||
				saveEvent.sectionIdx == secIdx)
				return false;
		}
	}

	saveEvents.emplace_back(idx, concatIdx, secIdx);
	return true;
}

bool Data::CheckFlag(DataFlag flag)
{
	return (flags & flag) != 0;
}
