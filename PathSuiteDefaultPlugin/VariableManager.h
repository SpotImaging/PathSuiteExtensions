#include <memory>
#include "StandardHostVariables.h"
#include "CppMacroTools.h"

class VariableManager
{
    typedef std::unordered_map<std::string, std::shared_ptr<IVariable>> ivar_collection_t;
    ivar_collection_t variableCollection;

public:
    VariableManager()
    {
    }

    ~VariableManager()
    {
    }

    size_t Size() const
    {
        return variableCollection.size();
    }

    void SaveAll(const std::string& fileName)
    {
        for(auto& item : variableCollection)
            HostInterop::SaveVariable(item.first.c_str(), fileName.c_str());
    }

    void RestoreAll(const std::string& fileName)
    {
        for(auto item : AllMutable())
        {
            HostInterop::RestoreVariableFromFile(item->Name().c_str(), fileName.c_str());           
        }
    }

    std::vector<IVariable*> MatchingAll(HostInterop::ScopeFlags withScope) const
    {
        std::vector<IVariable*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && item.second->Scope() == withScope)
                matching.push_back(item.second.get());
        }
        return matching;
    }

    template<typename T>
    std::vector<T*> MatchingAll(HostInterop::ScopeFlags withScope) const
    {
        std::vector<T*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && item.second->Scope() == withScope && dynamic_cast<T*>(item.second.get()))
                matching.push_back(static_cast<T*>(item.second.get()));
        }
        return matching;
    }

    std::vector<IVariable*> MatchingAny(HostInterop::ScopeFlags withScope) const
    {
        std::vector<IVariable*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && bool(item.second->Scope() & withScope))
                matching.push_back(item.second.get());
        }
        return matching;
    }

    template<typename T>
    std::vector<T*> MatchingAny(HostInterop::ScopeFlags withScope) const 
    {
        std::vector<T*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && bool(item.second->Scope() & withScope) && dynamic_cast<T*>(item.second.get()))
                matching.push_back(static_cast<T*>(item.second.get()));
        }
        return matching;
    }

    std::vector<IVariable*> AllMutable() const
    {
        std::vector<IVariable*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && !item.second->IsReadOnly())
                matching.push_back(item.second.get());
        }
        return matching;
    }

    template<typename T>
    std::vector<T*> AllMutable() const 
    {
        std::vector<T*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && !item.second->IsReadOnly() && dynamic_cast<T*>(item.second.get()))
                matching.push_back(static_cast<T*>(item.second.get()));
        }
        return matching;
    }

    std::vector<IVariable*> AllImmutable() const
    {
        std::vector<IVariable*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && item.second->IsReadOnly())
                matching.push_back(item.second.get());
        }
        return matching;
    }

    template<typename T>
    std::vector<T*> AllImmutable() const 
    {
        std::vector<T*> matching;
        for( auto& item : variableCollection)
        {
            if (item.second && item.second->IsReadOnly() && dynamic_cast<T*>(item.second.get()))
                matching.push_back(static_cast<T*>(item.second.get()));
        }
        return matching;
    }


    void Manage(IVariable* variable)
    {
        variableCollection[variable->Name()].reset(variable);
    }

    void Manage(const var_script_item_t& item)
    {
        switch(item.type)
        {
        case HostInterop::VariableType::Bool:
            Manage(new BoolVariable(item.szName, item.scope, item.readonly));
            break;
        case HostInterop::VariableType::Integer:
            Manage(new IntegerVariable(item.szName, item.scope, item.readonly));
            break;
        case HostInterop::VariableType::Numeric:
            Manage(new NumericVariable(item.szName, item.scope, item.readonly));
            break;
        case HostInterop::VariableType::Text:
            Manage(new TextVariable(item.szName, item.scope, item.readonly));
            break;
        default:
            assert(false); 
            throw std::logic_error("application logic error in file " AT_FILE_LOCATION);
        }
    }

    bool ContainsVariable(const std::string& name)
    {
        return variableCollection.find(name) != variableCollection.end();
    }

    template<typename T>
    bool ContainsVariable(const std::string& name)
    {
        auto item = variableCollection.find(name);
        return (item != variableCollection.end() && dynamic_cast<Variable<T*>>(item->second.get()) != nullptr);

    }

    IVariable& GetByName(const std::string& name) const
    {
        auto itemLocation = variableCollection.find(name);
        if (variableCollection.end() == itemLocation)
            throw std::invalid_argument(std::string("No variable with the name (").append(name).append(") exists"));
        return *(itemLocation->second.get());
    }

    template<typename T>
    T& GetByName(const std::string& name) const
    {
        auto var = dynamic_cast<T*>(&GetByName(name));
        if (nullptr == var)
            throw std::invalid_argument(std::string("No variable with the name (").append(name).append(") exists for type ").append(typeid(T).name()));
        return *var;
    }

    template<typename T>
    void SetValue(const std::string& name, const T& value)
    {
        GetByName<Variable<T>>(name).Value(value);
    }

    // Return a reference to a VariableManager that includes all the standard variables available by the host application.
    static VariableManager& StandardVars()
    {
        static VariableManager stdVars; // Singleton

        if (stdVars.Size() == 0)
        {
            for(auto &item : internal::std_vars_build_script)
            {
                try
                {
                    stdVars.Manage(item);
                }
                catch(const std::logic_error&)
                {
                    throw;
                }
                catch(std::runtime_error&)
                { /* ignore load failures */ }
            }
        }
        return stdVars;
    }

};

