// PathSuiteDefaultPlugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PathSuiteHostVars.h"

//#include <boost\uuid\uuid.hpp>
//#include <boost\uuid\string_generator.hpp>
//#include <boost\uuid\random_generator.hpp>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\json_parser.hpp>


void DoActionCode(int code);
void OnIdleEvent();
void OnShutdown();


using namespace std;
using namespace std::tr2;
//using namespace boost;
using namespace SpotPluginApi;
using namespace HostInterop;

CallbackDispatcher dispatcher;


enum Functions
{
    FILE_ConvertSlashes_T1_T5               = 1,
    FILE_CreateDirectory_T1_B5              = 2,
    FILE_DeleteFile_T1_B5                   = 3,
    FILE_DoesDirectoryExist_T1_B5           = 4,
    FILE_DoesFileExist_T1_B5                = 5,
    FILE_MakeFileOrDirHidden_T1_B5          = 6,
    FILE_VerifyFileName_T1T2_B5             = 7,
    
    RenameCase_T1T2_B5T5                    = 100,
    GetAccessionPrefixDesciption_T1_T5      = 101,
    LockCase_T1                             = 102,
    UnlockCase_T1                           = 103,
    GetSpecimenImageList_T1T2_T5N5          = 104,
    CreateImageCatalog_T1_T5B5              = 200,
    OpenImageCatalog_T1_T5B5                = 201
};

void SetEventHandlers()
{
    //std::function<void(HostEvents::application_closing_t::arg_type)> onExit = [] (HostEvents::application_closing_t::arg_type)
    //{
            // Do shutdown stuff here.
    //};
    //HostEvents::ApplicationClosing().AddDelegate(make_event_delegate(onExit));
    //add_logger_to_event( HostInterop::HostEvents::ApplicationClosing(), "Application closing");
    //add_logger_to_event( HostInterop::HostEvents::CameraInit(), "Camera initialized");
    //add_logger_to_event( HostInterop::HostEvents::ImageDocChanged(), "Image document changed");
}

void UpdateCatalogFileNames(const sys::path& catalogPath)
{
    // Rename the image files to remove the accession and specimen prefix.
    // The file is already contained within a file structure that contains this information.
    string regexPostfix = "\\d+(-TN)?\\.([jJ][pP][gG2])$";
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
            regex imageFileNameRegEx(MakeRegExLiteral( filePrefix ) + regexPostfix);
            if (regex_match(fileName, imageFileNameRegEx))
            {
                fileName.erase(0, filePrefix.size());
                sys::path newName = path.parent_path();
                newName /= fileName;
                sys::rename(path, newName);
            }
        }
    }
}



std::string FileReadAllText(const std::string& fileName)
{
    std::ifstream file(fileName);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::vector<std::string> FileReadAllLines(const std::string& fileName)
{
    using std::getline;

    std::ifstream file(fileName);
    std::vector<std::string> lines;
    while (file)
    {
        lines.push_back(std::string());
        getline(file, lines.back());
    }
    return lines;
}

bool IsValidCatalog(const sys::path& catalogDir)
{
    if (!sys::exists(catalogDir) || !sys::is_directory(catalogDir))
        return false;
    if (sys::exists(catalogDir / sys::path(".config")))
        return true;
    for (auto dirIter = sys::directory_iterator(catalogDir); dirIter != sys::directory_iterator(); ++dirIter)
    {
        auto path = dirIter->path();
        if (sys::is_directory(path) && sys::exists( path / sys::path("case.var") ))
            return true;
    }
    return false;
}

void MakeDefaultCatalogConfigDir(const sys::path& catalogDir)
{
    sys::path catalogConfig = catalogDir;
    catalogConfig /= ".config";
    bool createdDir = sys::create_directory(catalogConfig); // create_directory returns true if directory was created. If the directory already existed it will return false.
    if (createdDir)
    {
#ifdef WIN32
{
        auto dir = catalogConfig.string();
        SetFileAttributes(dir.c_str(), GetFileAttributes(dir.c_str()) | FILE_ATTRIBUTE_HIDDEN);
}
#endif // WIN32
        UpdateCatalogFileNames(catalogDir);
        const std::string AccesssionPrefixFileName = "AccessionPrefixes.txt";
        sys::path appPrefsFolder = MGR::PrefsFilePath();
        sys::path originalFile = appPrefsFolder / sys::path(AccesssionPrefixFileName);
        if (sys::exists(originalFile))
        {
            sys::path newFile = catalogConfig / sys::path(AccesssionPrefixFileName);
            sys::copy_file(originalFile, newFile);
            newFile = originalFile;
            newFile.replace_extension(newFile.extension() + ".bak");
            sys::rename(originalFile, newFile);
        }
    }
}


void GetPTree()
{
    //using boost::property_tree::ptree;
    //using boost::uuids::uuid;

    //ptree pt;
    //read_json("details", pt);
    //pt.get("details.version", 0);
    vector<char> a, b;
    std::swap_ranges(a.begin(), a.end(), b.begin());
    //auto catalogId = uuids::string_generator()(pt.get("details.catalogId", ""));
    //if (catalogId.is_nil())
    //    catalogId = uuids::random_generator()();
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
    *userData = reinterpret_cast<uintptr_t>(&dispatcher);
    
    // Checks if a file at path _argT1 exists
    dispatcher.SetAction(FILE_DoesFileExist_T1_B5, []()
    {
        sys::path fileName = Args::Text(1);
        Returns::Bool(sys::exists(fileName) && sys::is_regular_file(fileName));
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
        }
        catch(const std::system_error& ex)
        {
            OutputDebugString(ex.what());
        }
        sort(fileNames.begin(), fileNames.end());
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
                    throw std::runtime_error("The destination folder must be an empty folder. The directory must ");
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


    
    dispatcher.SetAction(GetAccessionPrefixDesciption_T1_T5, []()
    {
        string casePrefix = Args::Text(1);
        sys::path accessionPrefixFile = MGR::MasterCatalogFolder();
        accessionPrefixFile /= "AccessionPrefixes.txt";
        auto lines = FileReadAllLines(accessionPrefixFile.string());
        for (auto &line : lines)
        {
            auto entry = Explode(line, ':');
            if (entry.size() > 1 && entry[0] == casePrefix)
            {
                Returns::Text(entry[1]);
                break;
            }
        }
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

