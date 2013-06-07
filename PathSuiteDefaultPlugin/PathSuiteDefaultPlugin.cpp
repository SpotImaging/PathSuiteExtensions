// PathSuiteDefaultPlugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <chrono>
#include <ctime>
#include "PathSuiteHostVars.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>

const char* CONFIG_DIR_NAME                     = ".config";
const char* CATALOG_MAIN_CONFIG_FILENAME        = "HEAD";
const char* ACCESSION_PREFIX_FILENAME           = "AccessionPrefixes.txt";


using namespace std;
using namespace std::tr2;
using namespace boost;
using namespace SpotPluginApi;
using namespace HostInterop;

void MakeDefaultCatalogConfigDir(const sys::path& catalogDir);

enum Functions
{
    FILE_ConvertSlashes_T1_T5               = 1,
    FILE_CreateDirectory_T1_B5              = 2,
    FILE_DeleteFile_T1_B5                   = 3,
    FILE_DoesDirectoryExist_T1_B5           = 4,
    FILE_DoesFileExist_T1_B5                = 5,
    FILE_MakeFileOrDirHidden_T1_B5          = 6,
    FILE_VerifyFileName_T1_B5T5             = 7,
    FILE_EncodeForPath_T1_T5                = 8,
    FILE_DecodeFromPath_T1_T5B5             = 9,
    FILE_IsDirectroyEmptyOrMissing_T1_B5    = 10,

    TrimText_T1_T1                          = 20,
    SYS_GetDisplayResolution__N1N2          = 30,
    
    RenameCase_T1T2_B5T5                    = 100,
    GetAccessionPrefixDesciption_T1_T5      = 101,
    GetAccessionPrefixes__T5N5              = 102,
    AddAccessionPrefix_T1                   = 103,
    RemoveAccessionPrefix_T1                = 104,
    GetSpecimenImageList_T1T2_T5N5          = 105,
    GetSpecimenList_T1_T5N5                 = 106,
    CreateImageCatalog_T1_T5B5              = 200,
    OpenImageCatalog_T1_T5B5                = 201,
    IsValidCatalog_T1_B5                    = 202,
    LockCase_T1_T5B5                        = 203,
    UnlockCase_T1                           = 204
};

static std::vector<std::string> lockedCases;

sys::path CatalogConfigDirectory(const sys::path& catalogPath)
{
    return catalogPath / sys::path(CONFIG_DIR_NAME);
}

sys::path CatalogConfigDirectory()
{
    return CatalogConfigDirectory(sys::path(MGR::MasterCatalogFolder()));
}

sys::path CatalogMainConfigFile(const sys::path& catalogPath)
{
    return CatalogConfigDirectory(catalogPath) /= CATALOG_MAIN_CONFIG_FILENAME;
}

sys::path CatalogMainConfigFile()
{
    return CatalogConfigDirectory() /= CATALOG_MAIN_CONFIG_FILENAME;
}

sys::path GetCaseLockFilePath(const std::string& caseId)
{
    sys::path lockFile = MGR::MasterCatalogFolder();
    lockFile /= caseId;
    lockFile /= "case.lock";
    return lockFile;
}

boost::property_tree::ptree GetPropertyTree(const sys::path& fileName)
{
    using boost::property_tree::ptree;

    ptree pt;
    read_json(fileName, pt);
    return pt;
}

void SavePropertyTree(const sys::path& fileName, const boost::property_tree::ptree& pt)
{
    using boost::property_tree::write_json;
    write_json(fileName.string(), pt);

    //using boost::property_tree::write_ini;
    //sys::path iniFileName = fileName;
    //iniFileName.replace_extension("ini");
    //write_ini(iniFileName, pt);    
}


// Rename the image files within an existing catalog removing the accession and specimen prefixes.
// The file is already contained within a file structure that contains this information.
// Any alpha numbered files will be converted to the decimal equivalent.
void UpdateCatalog(const sys::path& catalogPath)
{
    string regexPostfix = "[\\w-]+\\.([jJ][pP][gG2])$";
    const char sectionDelimiters[] = {'-', '.'};
    for(auto dirIter = sys::recursive_directory_iterator(catalogPath); dirIter != sys::recursive_directory_iterator(); ++dirIter)
    {
        if(dirIter.level() > 2)
            dirIter.pop(); // no need to go deeper than two levels
        if (dirIter.level() == 2)
        {
            auto path = dirIter->path();
            if (!sys::is_regular_file(path))
                continue;
            auto pathIter = path.end();
            advance(pathIter, -3); // Backup two directory levels
            string filePrefix = *pathIter;
            filePrefix.append(".").append(*(++pathIter)).append(".");
            string fileName = *(++pathIter);
            regex imageFileNameRegEx("^" + MakeRegExLiteral( filePrefix ) + regexPostfix);
            if (regex_match(fileName, imageFileNameRegEx))
            {
                fileName.erase(0, filePrefix.size());
                if (isalpha(fileName.front()))
                {   // Alpha encoded number will be replaced with decimal number
                    auto postfixBegin = find_first_of(fileName.begin(), fileName.end(), begin(sectionDelimiters), end(sectionDelimiters));
                    string alphaNum = fileName.substr(0, distance(fileName.begin(), postfixBegin));
                    fileName.erase(fileName.begin(), postfixBegin); // remove alpha number
                    fileName.insert(0, to_string(AlphaToInt(alphaNum))); // insert decimal number
                }
                else
                { // Remove zero {0} padding from the front of the integer
                    fileName.erase(fileName.begin(), find_if(fileName.begin(), fileName.end(), [] (char c) { return c != '0';}));
                }
                sys::path newName = path.parent_path();
                newName /= fileName;
                sys::rename(path, newName);
            }
        }
    }
    if ( !sys::exists(CatalogConfigDirectory(catalogPath)) )
        MakeDefaultCatalogConfigDir(catalogPath);
}


bool IsValidCatalog(const sys::path& catalogDir)
{
    if ( !sys::exists(catalogDir) || !sys::is_directory(catalogDir) )
        return false;
    if ( sys::exists(CatalogConfigDirectory(catalogDir)) )
    {
        auto catalogDetailsFile = CatalogMainConfigFile(catalogDir);
        if (!sys::exists(catalogDetailsFile))
            return false;
        auto configData = GetPropertyTree(catalogDetailsFile);
        if (configData.get("details.version", 0) == 1)
            return true;
        return false;
    }
    for (auto dirIter = sys::directory_iterator(catalogDir); dirIter != sys::directory_iterator(); ++dirIter)
    {
        auto path = catalogDir / dirIter->path();
        if (sys::is_directory(path) && sys::exists( path/sys::path("case.var") ))
            return true;
    }
    return false;
}

bool CatalogNeedsToBeUpdated(const sys::path& catalogDir)
{
    return sys::exists(catalogDir) && !sys::exists(CatalogConfigDirectory(catalogDir));
}


void MakeDefaultCatalogConfigDir(const sys::path& catalogDir)
{
    using boost::property_tree::ptree;

    sys::path catalogConfig = CatalogConfigDirectory(catalogDir);
    bool createdDir = sys::create_directory(catalogConfig); // create_directory returns true if directory was created. If the directory already existed it will return false.
    if (createdDir)
    {
        MakeFileOrDirHidden(catalogConfig.string());
        sys::path appPrefsFolder = MGR::PrefsFilePath();
        sys::path originalFile = appPrefsFolder / sys::path(ACCESSION_PREFIX_FILENAME);
        if (sys::exists(originalFile))
        {
            sys::path newFile = catalogConfig / sys::path(ACCESSION_PREFIX_FILENAME);
            sys::copy_file(originalFile, newFile);
            newFile = originalFile;
            newFile.replace_extension(newFile.extension() + ".bak");
            sys::rename(originalFile, newFile);
        }
        ptree configData;
        configData.put("details.version", 1);
        configData.put("details.uid", to_string(uuids::random_generator()()));
        SavePropertyTree(CatalogMainConfigFile(catalogDir), configData);
    }
}


void SetEventHandlers()
{
    std::function<void(HostEvents::application_closing_t::arg_type)> onExit = [] (HostEvents::application_closing_t::arg_type)
    {
        // Do shutdown stuff here.
        for(auto& caseName : lockedCases)
        { // Remove any left over case lock files just in case someone didn't clean up after themselves.
            sys::remove(GetCaseLockFilePath(caseName));
        }
        lockedCases.clear();
    };
    HostEvents::ApplicationClosing().AddDelegate(make_event_delegate(onExit));
}

/// Summary:
///   Main export function that must be implemented by a plug-in.
///   This method will be called by the host application upon loading the library.
///   If the plug-in is allowed to load it will return a value of true.
///   Returning a value of false will cause the plug-in to unload
/// Parameters:
///   hostActionFunction (IN) -
///      A pointer to the host function to issue commands back to. 
///      Note that this function has usage limitations during the call to this function. See docs for more details.
///   handle (IN) - 
///      The handle for this plug-in. This value must be passed back to the host application.
///   info (IN) - 
///      Reserved for future use.
///   pluginCallbackFunc (OUT) -
///      A pointer to a value that will hold the callback function pointer. This value must be set be the function.
///      If a callback function is not needed then the pointer should be set to a value of NULL.
///   userData (OUT) -
///      A pointer to value that can be set by the function and the resulting value will be sent as an argument to the callback function. 
/// Returns:
///   true to continue loading the plug-in library, otherwise false.
bool SPOTPLUGINAPI SPOTPLUGIN_INIT_FUNC(host_action_func_t hostActionFunc, uintptr_t handle, uintptr_t info, callback_func_t *pluginCallbackFunc, uintptr_t *userData)
{
    // This following items must be initialized before anything else can be done. They are required for all plug-ins
    PluginHost::ActionFunc = hostActionFunc;
    PluginHost::pluginHandle = handle;

    //===============================
    // Setup callback handler
    *pluginCallbackFunc = CallbackDispatcher::master_callback_func;
    CallbackDispatcher& dispatcher = CallbackDispatcher::DefaultDispatcher();
    *userData = reinterpret_cast<uintptr_t>(&dispatcher);


    dispatcher.SetAction(FILE_ConvertSlashes_T1_T5, []()
    {
        string arg = Args::Text(1);
        replace(arg.begin(), arg.end(), '/', '\\');
        Returns::Text(5, arg);
    });
    
    // Checks if a file at path _argT1 exists
    dispatcher.SetAction(FILE_DoesFileExist_T1_B5, []()
    {
        sys::path fileName = Args::Text(1);
        Returns::Bool(sys::exists(fileName) && sys::is_regular_file(fileName));
    });

    // Checks if a file at path _argT1 exists
    dispatcher.SetAction(FILE_DoesDirectoryExist_T1_B5, []()
    {
        sys::path fileName = Args::Text(1);
        if (fileName.filename() == ".")
            fileName.remove_filename();
        Returns::Bool(sys::exists(fileName) && sys::is_directory(fileName));
    });


    dispatcher.SetAction(FILE_VerifyFileName_T1_B5T5, []()
    {
        bool isValidFilename = false;
        string fileName = TrimCopy(Args::Text(1)); // trim off all the whitespace on the ends
        if (fileName.empty())
        {
            Returns::Text("must not be an empty string.");
        }
        else if (find_first_of(fileName.begin(), fileName.end(), begin(InvalidFilePathChars), end(InvalidFilePathChars)) != fileName.end())
        {
            string message("must not contain any of the following characters:\n");
            interlace_with(begin(InvalidFilePathChars), end(InvalidFilePathChars), back_inserter(message), ' ');
            Returns::Text(message);
        }
        else if(find_if(fileName.begin(), fileName.end(), std::iscntrl) != fileName.end())
        {
            Returns::Text("must not contain a tab, newline, or any other non-displayable character.");
        }
        else
        {
            isValidFilename = true;
        }
        Returns::Bool(isValidFilename);
    });
    
    dispatcher.SetAction(FILE_MakeFileOrDirHidden_T1_B5, []()
    {
        Returns::Bool(MakeFileOrDirHidden(Args::Text(1)));
    });

    dispatcher.SetAction(TrimText_T1_T1, []()
    {
        Returns::Text(1, TrimCopy(Args::Text(1)));
    });

    dispatcher.SetAction(SYS_GetDisplayResolution__N1N2, []()
    {
        Returns::Num(1, GetSystemMetrics(SM_CXSCREEN));
        Returns::Num(2, GetSystemMetrics(SM_CYSCREEN));
    });
    
    
    dispatcher.SetAction(FILE_EncodeForPath_T1_T5, []()
    {
        const char escapeChar = '%';
        std::string original = TrimCopy(Args::Text(1));
        std::string encoded;
        encoded.reserve(original.size());
        for(auto ch : original)
        {
            if(find(begin(InvalidFilePathChars), end(InvalidFilePathChars), ch) != end(InvalidFilePathChars) || std::iscntrl(ch))
            {
                static const char digits[] = "0123456789ABCDEF";
                encoded.push_back(escapeChar);
                encoded.push_back(digits[(ch >> 4) & 0xF]);
                encoded.push_back(digits[ch & 0xF]);
            }
            else
            {
                if (escapeChar == ch)
                    encoded.push_back(escapeChar);
                encoded.push_back(ch);
            }
        }
        Returns::Text(5, encoded);
    });

    dispatcher.SetAction(FILE_DecodeFromPath_T1_T5B5, []()
    {
        bool success = true;
        const char escapeChar = '%';
        std::string encoded = Args::Text(1);
        std::string original;
        original.reserve(encoded.size());
        std::string::iterator cur = encoded.begin();
        while(cur != encoded.end())
        {
            std::string::iterator front = cur;
            cur = find(front, encoded.end(), escapeChar);
            copy(front, cur, back_inserter(original));
            if (cur != encoded.end())
            {
                static const char digits[] = "0123456789ABCDEF";
                size_t remaining = distance(++cur, encoded.end()); // move to the next char and get remaining length
                if (remaining >= 1 && *cur == escapeChar) 
                {   // a pair of escape chars "%%" equals a literal escape char '%'
                    original.push_back(escapeChar);
                }
                else if(remaining >= 2)
                {
                    auto upper = find(digits, digits+sizeof(digits), *cur);
                    auto lower = find(digits, digits+sizeof(digits), *++cur);
                    if (end(digits) == upper || end(digits) == lower)
                    {   // an escape char was followed by an invalid encoding value
                        success = false;
                        break;
                    }
                    original.push_back( static_cast<char>((distance(digits, upper)<<4) | distance(digits, lower)) );
                }
                else
                {   // the string was not long enough to contain the encoded value following the escape char
                    success = false;
                    break;
                }
            }
        }
        Returns::Bool(5, success);
        Returns::Text(5, original);
    });

    dispatcher.SetAction(FILE_IsDirectroyEmptyOrMissing_T1_B5, [] ()
    {
        sys::path dirToCheck = Args::Text(1);
        if (dirToCheck.filename() == ".")
            dirToCheck.remove_filename();
        bool dirExits = sys::exists(dirToCheck);
        Returns::Bool(!dirExits || (dirExits && sys::is_empty(dirToCheck)) );
    });

    // Get the list of specimens within a case.
    // Returns:
    // _argN5 contains the count of items in the list
    // _argT5 contains the name of each specimen separated by a newline char '\n'
    dispatcher.SetAction(GetSpecimenList_T1_T5N5, []()
    {
        vector<string> fileNames;
        try
        {
            sys::path directory = MGR::MasterCatalogFolder();
            directory /= Args::Text(1); // Case ID
            transform_if(sys::directory_iterator(directory), sys::directory_iterator(), back_inserter(fileNames),
                         [] (const sys::path& item) { return item.filename();},
                         [] (const sys::path& item) { return sys::is_directory(item); });
            sort(fileNames.begin(), fileNames.end());
        }
        catch(const std::system_error& ex)
        {
            OutputDebugString(ex.what());
        }
        Returns::Text(5, JoinWith(fileNames.begin(), fileNames.end(), "\n"));
        Returns::Num(5, fileNames.size()); 
    });


    // Get image file names in folder
    dispatcher.SetAction(GetSpecimenImageList_T1T2_T5N5, []()
    {
        vector<string> fileNames;
        try
        {
            sys::path directory = MGR::MasterCatalogFolder();
            directory /= Args::Text(1); // Case ID
            directory /= Args::Text(2); // Specimen number
            if (directory.filename() == ".")
                directory.remove_filename();
            string regString = "^\\d+\\.([jJ][pP][gG2])";
            regex imageFileNameRegEx(regString);
            transform_if(sys::directory_iterator(directory), sys::directory_iterator(), back_inserter(fileNames),
                         [] (const sys::path& item) { return item.filename();},
                         [&imageFileNameRegEx] (const sys::path& item) { return regex_match(item.filename(), imageFileNameRegEx); });
            sort(fileNames.begin(), fileNames.end());
        }
        catch(const std::system_error& ex)
        {
            OutputDebugString(ex.what());
        }
        Returns::Text(5, JoinWith(fileNames.begin(), fileNames.end(), "\n"));
        Returns::Num(5, fileNames.size()); 
    });

    dispatcher.SetAction(CreateImageCatalog_T1_T5B5, []()
    {
        bool success = false;
        try
        {
            sys::path catalogDir = Args::Text(1);
            if (catalogDir.filename() == ".")
                catalogDir.remove_filename();
            if (sys::exists(catalogDir) && sys::is_directory(catalogDir))
            {
                if(!sys::is_empty(catalogDir))
                    throw std::runtime_error("The destination folder must be an empty folder.");
            }
            else
                sys::create_directories(catalogDir);
            MakeDefaultCatalogConfigDir(catalogDir);
            success = true; 
        }
        catch(const std::exception& ex)
        {
            OutputDebugString(ex.what());
            Returns::Text(ex.what());
        }
        Returns::Bool(success);
    });


    // Updates a image catalog if needed to the current format.
    dispatcher.SetAction(OpenImageCatalog_T1_T5B5, []()
    {
        bool success = false;
        try
        {
            sys::path catalogDir = Args::Text(1);
            if (catalogDir.filename() == ".")
                catalogDir.remove_filename();
            if (!sys::exists(catalogDir))
                throw std::runtime_error("Unable to connect to the catalog. The catalog path does not exist on this system.");
            if(!IsValidCatalog(catalogDir))
                throw std::runtime_error("The image catalog is incompatible with this application version.");
            if (CatalogNeedsToBeUpdated(catalogDir))
                UpdateCatalog(catalogDir);
            success = true; 
        }
        catch(const std::exception& ex)
        {
            OutputDebugString(ex.what());
            success = false;
            Returns::Text(ex.what());
        }
        Returns::Bool(success);
    });

    dispatcher.SetAction(IsValidCatalog_T1_B5, []()
    {
        Returns::Bool(IsValidCatalog(Args::Text(1)));
    });

    dispatcher.SetAction(LockCase_T1_T5B5, []()
    {
        lockedCases.push_back(Args::Text(1));
        sys::path lockFileName = GetCaseLockFilePath(lockedCases.back());
        if(sys::exists(lockFileName))
        {
            Returns::Bool(false);
            Returns::Text(ReadFileToString(lockFileName));
        }
        else
        {
            using std::chrono::system_clock;
            using boost::uuids::uuid;

            ofstream lockFileStream(lockFileName);
            lockFileStream << "User: " << HostInterop::GetTextVariable("CurUserName") << std::endl;
            time_t tt = system_clock::to_time_t(system_clock::now());
            lockFileStream << "Locked On: " << ctime(&tt) << std::endl;
            lockFileStream << "Id:" << to_string(uuids::random_generator()());
            Returns::Bool(true);
        }
    });    

    dispatcher.SetAction(UnlockCase_T1, []()
    {
        string caseName = Args::Text(1);
        lockedCases.erase(find(lockedCases.begin(), lockedCases.end(), caseName));
        sys::remove(GetCaseLockFilePath(caseName));
    });    


    dispatcher.SetAction(GetAccessionPrefixes__T5N5, []()
    {
        sys::path accessionPrefixFile = CatalogConfigDirectory() /= ACCESSION_PREFIX_FILENAME;
        string result;
        int count = 0;
        if (sys::exists(accessionPrefixFile))
        {
            for (auto &line : ReadFileToStrings(accessionPrefixFile.string()))
            {
                line.erase(find(line.begin(), line.end(), '-'), line.end());
                Trim(line);
                result.append(line).append("\n");
                ++count;
            }
        }
        Returns::Num(count);
        Returns::Text(result);
    });

    dispatcher.SetAction(GetAccessionPrefixDesciption_T1_T5, []()
    {
        bool foundItem = false;
        string casePrefix = TrimRightCopy(Args::Text(1));
        sys::path accessionPrefixFile = CatalogConfigDirectory() /= ACCESSION_PREFIX_FILENAME;
        if (sys::exists(accessionPrefixFile))
        {
            for (auto &line : ReadFileToStrings(accessionPrefixFile.string()))
            {
                auto entry = Explode(line, '-');
                if (entry.size() > 1) 
                {
                    Trim(entry[0]);
                    if(entry[0] == casePrefix)
                    {
                        Returns::Text(TrimCopy(entry[1]));
                        foundItem = true;
                        break;
                    }
                }
            }
        }
        if (!foundItem)
            Returns::Text("");
    });
    

    /// Renames a case
    /// Args:
    ///     T1 - The current name of the case that is to be renamed
    ///     T2 - The new name of the case
    /// Returns:
    ///     B5 - A value of true if the case was renamed successfully
    ///     T5 - If the value of B5 is false this will contain a string describing why the rename failed.
    dispatcher.SetAction(RenameCase_T1T2_B5T5, []()
    {
        bool success = false;
        sys::path catalogPath = MGR::MasterCatalogFolder();
        sys::path oldpath = catalogPath/sys::path(Args::Text(1));
        sys::path newPath = catalogPath/sys::path(Args::Text(2));
        if (sys::exists( oldpath ))
        {
            if (sys::exists(newPath))
                Returns::Text("Cannot rename. The case " + Args::Text(2) + " already exists in the catalog.");
            else if(!(success = sys::rename(oldpath, newPath)))
                Returns::Text("Unable to update the image catalog. Check your system to ensure that you have privileges to write to the catalog location.");
        }
        else
            Returns::Text("The case "+ Args::Text(1) + " could not be found in the catalog.");
        Returns::Bool(success);
    });


    //===============================
    // Setup optional event bindings
    SetEventHandlers();

    return true; // Tell the host that we want to load
}

