#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

#include <unordered_map>
#include <vector>

namespace mpass
{

    /**
     * @brief Represents the type of Instruction found while traversing through the code
     * @note The items in this enum represent the list of instructions into which modulepass may be inserted
     */
    enum class InstructionType
    {
        Load,
        Store,
        AtomicCmpXchg,
        AtomicRMW,
        Alloca,
        NotSupported
    };

    class PassRunner : public llvm::ModulePass
    {
        static char s_id;

        using CallBack_T = std::function<bool(llvm::Instruction *)>;
        
        /* Pre and Post Process Callback types for Modules*/
        using ModulePostProcessCallback_T = std::function<bool(llvm::Module&)>;
        using ModulePreProcessCallback_T = ModulePostProcessCallback_T;

        /* Pre and Post Process Callback types for Functions*/
        using FunctionPostProcessCallback_T = std::function<bool(llvm::Function&)>;
        using FunctionPreProcessCallback_T = FunctionPostProcessCallback_T;

        std::unordered_map<InstructionType, std::vector<CallBack_T>> m_instruction_callback;

        std::vector<std::string> m_function_exclusion_list;
        std::vector<std::string> m_module_exclusion_list;

        std::vector<ModulePostProcessCallback_T> m_module_post_process_callbacks;
        std::vector<ModulePreProcessCallback_T> m_module_pre_process_callbacks;

        std::vector<FunctionPostProcessCallback_T> m_function_post_process_callbacks;
        std::vector<FunctionPreProcessCallback_T> m_function_pre_process_callbacks;

    public:
        PassRunner() : ModulePass(s_id) {}

        virtual bool runOnModule(llvm::Module &M);

        void registerCallback(InstructionType instruction_type, CallBack_T callback_function);
        void runOnFunction(llvm::Function &F);
        void runOnInstruction(llvm::Instruction *instruction);
                
        bool inExclusionList(llvm::Module& M);
        bool inExclusionList(llvm::Function& F);
        void addModuleToExclusionList(const std::string& path);
        void addFunctionToExclusionList(const std::string& func_name);

        void addModulePostProcessCallback(ModulePostProcessCallback_T post_processing_callback);
        void addModulePreProcessCallback(ModulePreProcessCallback_T pre_processing_callback);

        void addFunctionPostProcessCallback(FunctionPostProcessCallback_T post_processing_callback);
        void addFunctionPreProcessCallback(FunctionPreProcessCallback_T pre_processing_callback);

    private:
        // Helpers
        InstructionType getInstructionType(llvm::Instruction *instruction);
        llvm::SmallString<128> getAbsoluteModulePath(llvm::Module& M);
        bool runPostProcessCallbacks(llvm::Module& M);
        bool runPostProcessCallbacks(llvm::Function& F);
        
        bool runPreProcessCallbacks(llvm::Module& M);
        bool runPreProcessCallbacks(llvm::Function& F);

    };

}
