#include <map>

#include "System.h"

#include "Utils/Alle5Format.h"
#include "Utils/StringUtils.h"

map<wchar_t, wchar_t> specialCharacters = {
    {0x246D, L'♂'},
    {0x246E, L'♀'},
    {0xFFFE, L'\n'},
};

#define COMMAND_HEADER (wchar_t)0xF000
map<wchar_t, wstring> commands = {
    {0x0100, L"Trainer Name"},
    {0x0101, L"Pokémon Name"},
    {0x0102, L"Pokémon Nick"},
    {0x0103, L"Type"},
    {0x0105, L"Location"},
    {0x0106, L"Ability"},
    {0x0107, L"Move"},
    {0x0108, L"Item 1"},
    {0x0109, L"Item 2"},
    {0x010A, L"Trainer Bag"},
    {0x010B, L"Box"},
    {0x010D, L"EV Stat"},
    {0x0110, L"O Power"},
    {0x0127, L"Ribbon"},
    {0x0134, L"Mii Name"},
    {0x013E, L"Weather"},
    {0x0189, L"Trainer Nick"},
    {0x018A, L"1st char Trainer"},
    {0x018B, L"Shoutout"},
    {0x018E, L"Berry"},
    {0x018F, L"Rem Feel"},
    {0x0190, L"Rem Qual"},
    {0x0191, L"Website"},
    {0x019C, L"Choice Cos"},
    {0x01A1, L"GSync ID"},
    {0x0192, L"Private ID Say"},
    {0x0193, L"Battle Test"},
    {0x0195, L"Gender Lock"},
    {0x0199, L"Choice Food"},
    {0x019A, L"Hotel Item"},
    {0x019B, L"Taxi Stop"},
    {0x019F, L"Mais Title"},
    {0x1000, L"Item Plural 0"},
    {0x1001, L"Item Plural 1"},
    {0x1100, L"Gender"},
    {0x1101, L"Number Branch"},
    {0x1302, L"iColor 2"},
    {0x1303, L"iColor 3"},
    {0x0200, L"Number 1"},
    {0x0201, L"Number 2"},
    {0x0202, L"Number 3"},
    {0x0203, L"Number 4"},
    {0x0204, L"Number 5"},
    {0x0205, L"Number 6"},
    {0x0206, L"Number 7"},
    {0x0207, L"Number 8"},
    {0x0208, L"Number 9"},
    {0xBD01, L"Color Reset"},
    {0xBE00, L"Wait Scroll"},
    {0xBE01, L"Wait Clear"},
    {0xBE02, L"Wait Pause"},
    {0xFF00, L"Color"},
};

#define NINE_BIT_HEADER (wchar_t)0xF100
#define NINE_BIT_NAME L"9-Bit"

#define AVAILABLE_BITS sizeof(wchar_t) * 8
#define BIT_MASK 0x01FF
#define MAX_READ 9
wstring Process9Bit(const wstring& str)
{
    wstring output;

    wchar_t character = 0;
    u8 nextBit = 0;

    for (u32 i = 0; i < (u32)str.size(); ++i)
    {
        wchar_t data = str[i];
        u8 nextReadBit = 0;

        while (nextReadBit < AVAILABLE_BITS)
        {
            u8 bitsToRead = MAX_READ - nextBit;
            u8 readableBits = AVAILABLE_BITS - nextReadBit;
            if (bitsToRead > readableBits)
                bitsToRead = readableBits;

            wchar_t mask = BIT_MASK >> (MAX_READ - bitsToRead);
            mask = mask << nextReadBit;

            wchar_t bitsRead = (data & mask) >> nextReadBit;
            character |= bitsRead << nextBit;

            nextBit += bitsToRead;
            nextReadBit += bitsToRead;

            if (nextBit >= MAX_READ)
            {
                if (character == BIT_MASK)
                    return output;
                output.push_back(character);
                character = 0;
                nextBit = 0;
            }
        }
    }
    return output;
}

wchar_t EncryptCharacter(wchar_t character, u16& key)
{
    character = key ^ character;
    key = ((key << 3) | (key >> 13)) & 0xFFFF;
    return character;
}
wchar_t DecryptCharacter(wchar_t character, u16& key)
{
    character = character ^ key;
    key = ((key << 3) | (key >> 13)) & 0xFFFF;
    return character;
}

bool LoadAlle5Data(const FileStream& fileStream, vector<string>& lines)
{
    u16 textSections = FileStreamRead<u16>(fileStream, 0);
    u16 lineCount = FileStreamRead<u16>(fileStream, 2);
    if (lineCount == 0)
    {
        Log(WARNING, "Empty text file");
        return false;
    }

    u32 totalLength = FileStreamRead<u32>(fileStream, 4);
    u32 initialKey = FileStreamRead<u32>(fileStream, 8);
    u32 sectionData = FileStreamRead<u32>(fileStream, 12);

    // Some sanity checking to prevent errors.
    if (initialKey != 0)
    {
        Log(WARNING, "Invalid initial key in text file");
        return false;
    }
    if (sectionData + totalLength != fileStream.length || textSections != 1)
    {
        Log(INFO, "Invalid text file, common with CTRMap text files processing either way");
    }

    u32 sectionLength = FileStreamRead<u32>(fileStream, sectionData);
    if (sectionLength != totalLength)
    {
        Log(WARNING, "Section size and overall size do not match in text file");
        return false;
    }

    lines.clear();
    lines.reserve(lineCount);

    // Start key
    u16 key = 0x7C89;

    for (u16 lineIdx = 0; lineIdx < lineCount; ++lineIdx)
    {
        wstring line;

        u16 lineKey = key;

        u32 offset = FileStreamRead<u32>(fileStream, (lineIdx * 8) + sectionData + 4) + sectionData;
        u32 length = FileStreamRead<u32>(fileStream, (lineIdx * 8) + sectionData + 8);

        bool is9Bit = false;
        wstring line9Bit;

        line.reserve(length);

        u32 start = offset;
        while (offset < start + length * 2) // loop through the entire text line
        {
            wchar_t character = DecryptCharacter(FileStreamReadUpdate<wchar_t>(fileStream, offset), lineKey);
            if (character == 0xFFFF)
                break;

            if (character == NINE_BIT_HEADER)
            {
                is9Bit = true;
                line9Bit.reserve(length);

                //line += L'[';
                //line += NINE_BIT_NAME;
                //line += L']';
                continue;
            }
            if (is9Bit)
            {
                line9Bit.push_back(character);
                continue;
            }

            map<wchar_t, wchar_t>::iterator itr = specialCharacters.find(character);
            if (itr != specialCharacters.end())
            {
                line += itr->second;
                continue;
            }

            if (character == COMMAND_HEADER)
            {
                line += L'[';

                u16 type = (u16)DecryptCharacter(FileStreamReadUpdate<wchar_t>(fileStream, offset), lineKey);
                u16 paramCount = (u16)DecryptCharacter(FileStreamReadUpdate<wchar_t>(fileStream, offset), lineKey);

                map<wchar_t, wstring>::iterator cmdItr = commands.find(type);
                if (cmdItr != commands.end())
                {
                    line += cmdItr->second;
                    for (u8 paramIdx = 0; paramIdx < paramCount; ++paramIdx)
                    {
                        u16 param = (u16)DecryptCharacter(FileStreamReadUpdate<wchar_t>(fileStream, offset), lineKey);
                        line += L'(' + to_wstring(param) + L')';
                    }
                }

                line += L']';
                continue;
            }

            line += character;
        }
        if (is9Bit)
            line += Process9Bit(line9Bit);
        lines.emplace_back(WideToUtf8(line));

        // Set key for next line
        key += 0x2983;
    }

    return true;
}

bool LoadAlle5File(const string& path, vector<string>& lines)
{
    FileStream fileStream;
    if (!LoadFileStream(fileStream, path))
        return false;

    bool output = LoadAlle5Data(fileStream, lines);

    ReleaseFileStream(fileStream);
    return output;
}

wstring MakeWideLine(string line)
{
    wstring output = Utf8ToWide(line);
    output.push_back((wchar_t)0xFFFF);
    return output;
}

bool SaveAlle5File(const string& path, const vector<string>& lines)
{
    FileStream fileStream;
    if (!LoadEmptyFileStream(fileStream))
        return false;

    // Set the header
    FileStreamPutBack<u16>(fileStream, 1);
    FileStreamPutBack<u16>(fileStream, (u16)lines.size());
    FileStreamPutBack<u32>(fileStream, ~0); // temporary value
    FileStreamPutBack<u32>(fileStream, 0);
    FileStreamPutBack<u32>(fileStream, 16);
    // Set the section data
    FileStreamPutBack<u32>(fileStream, ~0); // temporary value

    // Set the line data
    for (u16 lineIdx = 0; lineIdx < lines.size(); ++lineIdx)
    {
        // temporary values
        FileStreamPutBack<u32>(fileStream, ~0); // offset
        FileStreamPutBack<u32>(fileStream, ~0); // length
    }

    u16 key = 0x7C89;
    u32 offset = fileStream.length - 16;

    // Set the character data
    for (u16 lineIdx = 0; lineIdx < lines.size(); ++lineIdx)
    {
        // Update the line data with the actual offset
        FileStreamReplace<u32>(fileStream, (lineIdx * 8) + 16 + 4, offset);

        // TODO: Turn back into 9 bit when necessary

        wstring line = MakeWideLine(lines[lineIdx]);
        u16 lineKey = key;

        u32 length = 0;
        for (u32 characterIdx = 0; characterIdx < line.length(); ++characterIdx)
        {
            wchar_t character = line[characterIdx];

            // Check if there is a command
            if (character == L'[')
            {
                wstring command;
                vector<u16> params;
                // Find the characters inside the command
                for (++characterIdx; characterIdx < line.length(); ++characterIdx)
                {
                    // Stop the loop if the command ended
                    if (line[characterIdx] == L']')
                        break;

                    // Check if there is a command parameter
                    if (line[characterIdx] == L'(')
                    {
                        wstring param;
                        for (++characterIdx; characterIdx < line.length(); ++characterIdx)
                        {
                            // Stop the loop if the command parameter ended
                            if (line[characterIdx] == L')')
                            {
                                // Advance by 1 so that the parenthesis does 
                                // not get added to the command string
                                ++characterIdx;
                                break;
                            }
                                
                            param += line[characterIdx];
                        }

                        params.push_back((u16)stoi(param));
                        break;
                    }

                    command += line[characterIdx];
                }

                for (map<wchar_t, wstring>::iterator itr = commands.begin();
                    itr != commands.end(); ++itr)
                {
                    if (itr->second == command)
                    {
                        FileStreamPutBack<u16>(fileStream, EncryptCharacter(COMMAND_HEADER, lineKey));
                        FileStreamPutBack<u16>(fileStream, EncryptCharacter(itr->first, lineKey));
                        FileStreamPutBack<u16>(fileStream, EncryptCharacter((wchar_t)params.size(), lineKey));

                        offset += 6;
                        length += 3;

                        for (u8 paramIdx = 0; paramIdx < params.size(); ++paramIdx)
                        {
                            FileStreamPutBack<u16>(fileStream, EncryptCharacter(params[paramIdx], lineKey));

                            offset += 2;
                            ++length;
                        }
                        break;
                    }
                }

                continue;
            }

            for (map<wchar_t, wchar_t>::iterator itr = specialCharacters.begin();
                itr != specialCharacters.end(); ++itr)
            {
                if (itr->second == character)
                {
                    character = itr->first;
                    break;
                }
            }

            FileStreamPutBack<u16>(fileStream, EncryptCharacter(character, lineKey));

            offset += 2;
            ++length;
        }

        // Update the line data with the actual length
        FileStreamReplace<u32>(fileStream, (lineIdx * 8) + 16 + 8, length);

        // Increment the key for the next line
        key += 0x2983;
    }

    // Update the header with the actual section lenght
    FileStreamReplace<u32>(fileStream, 4, fileStream.length - 16);
    FileStreamReplace<u32>(fileStream, 16, fileStream.length - 16);

    SaveFileStream(fileStream, path);

    ReleaseFileStream(fileStream);
    return true;
}