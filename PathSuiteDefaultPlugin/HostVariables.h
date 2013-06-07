#pragma once
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <type_traits>
#include <cassert>
#include "SpotPlugin.h"
#include "PluginHost.h"
#include "Utilities.h"


namespace HostInterop
{

    enum class VariableType
    {
        Bool,
        Text,
        Numeric,
        Integer 
    };

    enum class ScopeFlags
    {
        Unknown             = 0x00,
        ImageMetaData       = 0x01,
        CameraSetting       = 0x02,
        UserSetting         = 0x04,
        CaseInformation     = 0x08,
        FilePath            = 0x10,
        Measurment          = 0x20,
        Reporting           = 0x40,
        ApplicationState    = 0x80
    };

    inline ScopeFlags operator | (ScopeFlags a, ScopeFlags b)
    { return static_cast<ScopeFlags>(static_cast<std::underlying_type<ScopeFlags>::type>(a) | static_cast<std::underlying_type<ScopeFlags>::type>(b));}

    inline ScopeFlags operator & (ScopeFlags a, ScopeFlags b)
    { return static_cast<ScopeFlags>(static_cast<std::underlying_type<ScopeFlags>::type>(a) & static_cast<std::underlying_type<ScopeFlags>::type>(b));}


namespace internal // implementation specific namespace not for general usage
{
    template<size_t MaxReadLength>
    static inline std::string _GetTextVariable(const char* name)
    {
        static_assert(MaxReadLength < 65536u, "Warning - potential stack overflow detected. Consider using a different method to retrieve a variable of this size.");
        char szTextBuffer[MaxReadLength + 1];
        SpotPluginApi::msg_get_set_variable_t getVarMsg;
        getVarMsg.DataType = SpotPluginApi::msg_get_set_variable_t::Text;
        getVarMsg.VariableName = name;
        getVarMsg.TextValue = SpotPluginApi::make_text_variable(szTextBuffer, MaxReadLength);
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::GetVariable, 0, &getVarMsg))
            throw std::runtime_error(std::string("Error getting text macro variable named ") + name);
        getVarMsg.TextValue.UpdateLength();
        return std::string(getVarMsg.TextValue.c_str());
    }
} // end namespace internal

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /// Summary:
    ///     Sets a global text variable with a matching name to a new value.
    /// Arguments:
    ///     name  - A null terminated string of the name of the target variable
    ///     value - The value to set the variable to.
    /// Returns:
    ///     void
    /// Throws:
    ///     runtime_error if unable to set the variable value
    static void SetTextVariable(const char* name, const std::string& value)
    {
        SpotPluginApi::msg_get_set_variable_t setVarMsg;
        setVarMsg.DataType = SpotPluginApi::msg_get_set_variable_t::Text;
        setVarMsg.VariableName = name;
        setVarMsg.TextValue = SpotPluginApi::make_text_variable(value);
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::SetVariable, 0, &setVarMsg))
            throw std::runtime_error(std::string("Error setting text macro variable named ") + name);
    }

    /// Summary:
    ///     Returns a string with the current value of a global text variable with a matching name.
    /// Arguments:
    ///     name - A null terminated string of the name of the target variable
    /// Template Arguments:
    ///     MaxReadLength - The maximum length of the variable string to return.
    ///                     Make sure there is enough stack space available to allocate a char buffer of this size.
    /// Returns:
    ///     A std::string that is set to the current value of the global variable.
    /// Throws:
    ///     runtime_error if unable to get the variable value
    template<size_t MaxReadLength>
    static std::string GetTextVariable(const char* name)
    { return internal::_GetTextVariable<MaxReadLength>(name); }

    
    /// Summary:
    ///     Returns a string with the current value of a global text variable with a matching name.
    /// Arguments:
    ///     name - A null terminated string of the name of the target variable
    /// Returns:
    ///     A std::string that is set to the current value of the global variable up to the first 1024 characters.
    /// Throws:
    ///     runtime_error if unable to get the variable value
    static inline std::string GetTextVariable(const char* name)
    { return internal::_GetTextVariable<1024>(name); }

    
    /// Summary:
    ///     Sets a global numeric variable with a matching name to a new value.
    /// Arguments:
    ///     name  - A null terminated string of the name of the target variable
    ///     value - The value to set the variable to.
    /// Returns:
    ///     void
    /// Throws:
    ///     runtime_error if unable to set the variable value
    static inline void SetNumericVariable(const char* name, double value)
    {
        SpotPluginApi::msg_get_set_variable_t setVarMsg;
        setVarMsg.DataType = SpotPluginApi::msg_get_set_variable_t::Numeric;
        setVarMsg.VariableName = name;
        setVarMsg.NumericValue = value;
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::SetVariable, 0, &setVarMsg))
            throw std::runtime_error(std::string("Error setting numeric macro variable named ") + name);
    }
    
    /// Summary:
    ///     Returns the current value of a global numeric variable with a matching name.
    /// Arguments:
    ///     name - A null terminated string of the name of the target variable
    /// Returns:
    ///     A double precision float set to the current value of the global variable
    /// Throws:
    ///     runtime_error if unable to get the variable value
    static inline double GetNumericVariable(const char* name)
    {
        SpotPluginApi::msg_get_set_variable_t getVarMsg;
        getVarMsg.DataType = SpotPluginApi::msg_get_set_variable_t::Numeric;
        getVarMsg.VariableName = name;
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::GetVariable, 0, &getVarMsg))
            throw std::runtime_error(std::string("Error getting numeric macro variable named ") + name);
        return getVarMsg.NumericValue;
    }

    /// Summary:
    ///     Sets a global Boolean variable with a matching name to a new value.
    /// Arguments:
    ///     name  - A null terminated string of the name of the target variable
    ///     value - The value to set the variable to.
    /// Returns:
    ///     void
    /// Throws:
    ///     runtime_error if unable to set the variable value
    static inline void SetBoolVariable(const char* name, bool value)
    {
        SpotPluginApi::msg_get_set_variable_t setVarMsg;
        setVarMsg.DataType = SpotPluginApi::msg_get_set_variable_t::Bool;
        setVarMsg.VariableName = name;
        setVarMsg.BoolValue = value;
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::SetVariable, 0, &setVarMsg))
            throw std::runtime_error(std::string("Error setting Boolean macro variable named ") + name);
    }
    
    /// Summary:
    ///     Returns the current value of a global Boolean variable with a matching name.
    /// Arguments:
    ///     name - A null terminated string of the name of the target variable
    /// Returns:
    ///     A bool set to the current value of the global variable
    /// Throws:
    ///     runtime_error if unable to get the variable value
    static inline bool GetBoolVariable(const char* name)
    {
        SpotPluginApi::msg_get_set_variable_t getVarMsg;
        getVarMsg.DataType = SpotPluginApi::msg_get_set_variable_t::Bool;
        getVarMsg.VariableName = name;
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::GetVariable, 0, &getVarMsg))
            throw std::runtime_error(std::string("Error getting Boolean macro variable named ") + name);
        return getVarMsg.BoolValue != 0;
    }


    /// Summary:
    ///     Saves the current value of a global variable to a file.
    /// Arguments:
    ///     name - A null terminated string of the name of the target variable
    ///     fileName - A null terminated string for the target file path.
    /// Returns:
    ///     void
    /// Throws:
    ///     runtime_error if the host application was unable to save the variable value
    static inline void SaveVariable(const char* name, const char* fileName)
    {
        SpotPluginApi::msg_save_recall_variable_t saveMsg;
        saveMsg.FilePath = fileName;
        saveMsg.VariableName = name;
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::SaveVariable, 0, &saveMsg))
            throw std::runtime_error(std::string("Error saving variable (").append(name).append(") to the file ").append(fileName));
    }

    /// Summary:
    ///     Restores the value of a global variable from a file that was created previously.
    /// Arguments:
    ///     name - A null terminated string of the name of the target variable
    ///     fileName - A null terminated string for the target file path.
    /// Returns:
    ///     void
    /// Throws:
    ///     runtime_error if the host application was unable to restore the variable value
    static inline void RestoreVariableFromFile(const char* name, const char* fileName)
    {
        SpotPluginApi::msg_save_recall_variable_t restoreMsg;
        restoreMsg.FilePath = fileName;
        restoreMsg.VariableName = name;
        if (!PluginHost::DoAction( SpotPluginApi::HostActionRequest::RecallVariable, 0, &restoreMsg))
            throw std::runtime_error(std::string("Error reading variable (").append(name).append(") from file ").append(fileName));
    }
    
    // Helper class to make the retrieval of macro arguments
    // conform to an operational standard.
    struct Args
    {
        static std::string Text(int index)
        {
            static char name[] = "_argT*";
            if (index <= 0 || index > 5)
                throw std::logic_error("invalid macro stack index");
            name[5] = '0' + index;
            return GetTextVariable(name);
        }

        static bool Bool(int index)
        {
            static char name[] = "_argB*";
            if (index <= 0 || index > 5)
                throw std::logic_error("invalid macro stack index");
            name[5] = '0' + index;
            return GetBoolVariable(name);
        }

        static double Num(int index)
        {
            static char name[] = "_argN*";
            if (index <= 0 || index > 5)
                throw std::logic_error("invalid macro stack index");
            name[5] = '0' + index;
            return GetNumericVariable(name);
        }
    };

    // Helper class to make the macro return variables
    // conform to an operational standard.
    struct Returns
    {
        static void Text(const std::string& value)
        {
            Text(5, value);
        }

        static void Text(int index, const std::string& value)
        {
            static char name[] = "_argT*";
            if (index <= 0 || index > 5)
                throw std::logic_error("invalid macro stack index");
            name[5] = '0' + index;
            SetTextVariable(name, value);
        }

        static void Bool(bool value)
        {
            Bool(5, value);
        }

        static void Bool(int index, bool value)
        {
            static char name[] = "_argB*";
            if (index <= 0 || index > 5)
                throw std::logic_error("invalid macro stack index");
            name[5] = '0' + index;
            SetBoolVariable(name, value);
        }

        static void Num(double value)
        {
            Num(5, value);
        }

        static void Num(int index, double value)
        {
            static char name[] = "_argN*";
            if (index <= 0 || index > 5)
                throw std::logic_error("invalid macro stack index");
            name[5] = '0' + index;
            SetNumericVariable(name, value);
        }
    };

} // end namespace HostInterop

struct var_script_item_t  { bool readonly; const char* szName; HostInterop::VariableType type; HostInterop::ScopeFlags scope; };

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Summary:
/// Abstract base class for all host variables
class IVariable
{
protected:
    std::string name;
    std::shared_ptr<std::string> objectId; // The name of the object that owns the variable
    HostInterop::VariableType type;
    HostInterop::ScopeFlags scope;
    bool readOnly;

    IVariable(std::string name, std::shared_ptr<std::string> objectId, HostInterop::VariableType type, HostInterop::ScopeFlags scope, bool readOnly) :
        name(name), objectId(objectId), type(type), scope(scope), readOnly(readOnly) {}
public:
    virtual ~IVariable() {};
    
    /// Get the name of the variable.
    const std::string& Name() const { return name; }
    
    /// Summary:
    ///     Get the pointer to the named objectId
    /// Returns:
    ///     A shared_ptr to a string if the variable has an owning object associated to it.
    ///     If the value is equal to nullptr then there is no owning object.
    const std::shared_ptr<std::string> ObjectId() const { return objectId; }
    
    /// The variable data type.
    HostInterop::VariableType Type() const { return type;}
    
    /// Returns the general scope for which the variable is related to. 
    HostInterop::ScopeFlags Scope() const { return scope; }
    /// Return true if the variable can only be read.
    bool IsReadOnly() const { return readOnly; }
    /// Returns true if the variable has no owning object associated to it.
    bool IsGlobal() const { return nullptr == objectId; }
    /// Returns a human readable string describing the state of the object.
    /// The default implementation should be overridden by derived classes.
    virtual std::string ToString() { return std::string(name).append(", type:").append(std::to_string((int)type)).append(", {undefined value}"); }
};
    
template<typename T>
class Variable : public IVariable
{
public:
    Variable(std::string name, std::shared_ptr<std::string> objectId, HostInterop::VariableType type, HostInterop::ScopeFlags scope, bool readOnly) :
        IVariable(name, objectId, type, scope, readOnly)
    {}
    virtual ~Variable() {}
    virtual T Value() const = 0;
    virtual Variable& Value(const T& value) = 0;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Summary:
///
class BoolVariable : public Variable<bool>
{
public:
    BoolVariable(const char* name, HostInterop::ScopeFlags scope = HostInterop::ScopeFlags::Unknown, bool isReadOnly=false) :
        Variable<bool>(name, nullptr, HostInterop::VariableType::Text, scope, isReadOnly)
    {   }

    virtual bool Value() const { return HostInterop::GetBoolVariable(name.c_str()); }

    virtual Variable<bool>& Value(const bool& newValue)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        HostInterop::SetBoolVariable(name.c_str(), newValue);
        return *this;
    }

    virtual std::string ToString() { return Value() ? "true" : "false"; }
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Summary:
///
class TextVariable : public Variable<std::string>
{
public:
    TextVariable(const char* name, HostInterop::ScopeFlags scope = HostInterop::ScopeFlags::Unknown, bool isReadOnly=false) :
        Variable<std::string>(name, nullptr, HostInterop::VariableType::Text, scope, isReadOnly)
    {  }

    virtual std::string Value() const { return HostInterop::GetTextVariable(name.c_str()); }

    virtual Variable<std::string>& Value(const std::string& newValue)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        HostInterop::SetTextVariable(name.c_str(), newValue);
        return *this;
    }

    virtual std::string ToString() { return Value(); }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Summary:
///
class NumericVariable : public Variable<double>
{

public:
    NumericVariable(const char* name, HostInterop::ScopeFlags scope = HostInterop::ScopeFlags::Unknown, bool isReadOnly = false) :
        Variable(name, nullptr, HostInterop::VariableType::Numeric, scope, isReadOnly)
    { }

    virtual double Value() const { return HostInterop::GetNumericVariable(name.c_str());}

    virtual Variable<double>& Value(int newValue)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        HostInterop::SetNumericVariable(name.c_str(), newValue);
        return *this;
    }

    virtual Variable<double>& Value(const double& newValue)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        HostInterop::SetNumericVariable(name.c_str(), newValue);
        return *this;
    }

    virtual Variable<double>& Value(const std::string& textToParse)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        HostInterop::SetNumericVariable(name.c_str(), std::stod(textToParse));
        return *this;
    }

    virtual std::string ToString()
    {
        return std::to_string(Value()); 
    }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Summary:
///
class IntegerVariable : public Variable<int>
{

public:
    IntegerVariable(const char* name, HostInterop::ScopeFlags scope = HostInterop::ScopeFlags::Unknown, bool isReadOnly = false) :
        Variable(name, nullptr, HostInterop::VariableType::Integer, scope, isReadOnly)
    { }

    virtual int Value() const { return static_cast<int>(HostInterop::GetNumericVariable(name.c_str()));}

    virtual Variable<int>& Value(const int& newValue)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        double realVal = newValue;
        HostInterop::SetNumericVariable(name.c_str(), realVal);
        return *this;
    }

    virtual Variable<int>& Value(double newValue)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        newValue = round_to_nearest_awayzero(newValue);
        HostInterop::SetNumericVariable(name.c_str(), newValue);
        return *this;
    }

    virtual Variable<int>& Value(const std::string& textToParse, int base = 10)
    {
        if (IsReadOnly())
            throw std::runtime_error(std::string("Illegal operation. The variable (").append(name).append(") is a read only variable"));
        HostInterop::SetNumericVariable(name.c_str(), static_cast<double>(std::stoi(textToParse, nullptr, base)));
        return *this;
    }

    virtual std::string ToString()
    {
        return std::to_string(Value()); 
    }
};
