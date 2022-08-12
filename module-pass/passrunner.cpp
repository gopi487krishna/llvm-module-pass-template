#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "passrunner.hpp"
#include "modulepass.hpp"

#include "llvm/IR/InstIterator.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Bitcode/BitcodeWriter.h"

#include "utilities.hpp"

namespace mpass
{

  bool PassRunner::runOnModule(llvm::Module &M)
  {

    /* Set up module exclusion list and function exclusion list*/
    
    // Eg: Excludes entire folder
    //    addModuleToExclusionList(std::string(OS_DIR "/linux-5.16.5/arch/x86/kernel"));
    
    // EL Excludes a function while running the pass inside the module
    //    addFunctionToExclusionList("preempt_count");

    /* Check if the module needs to be excluded :( */
    if (inExclusionList(M))
    {
      /* Do not continue the pass as this module is excluded*/
      return false;
    }

    /* Do the call back registration here for the instructions*/
    // registerCallback(InstructionType::Load, OOBCheck::process_load_instruction);
    // registerCallback(InstructionType::Store, OOBCheck::process_store_instruction);


    /* Register pre and post processing hooks here */
    /* Run pre processing callbacks */

    if (!runPreProcessCallbacks(M))
    {
      llvm::errs() << __FUNCTION__ << " : Failed to run PreProcessing Callback for Module :" << M.getName() << '\n';
    }

    for (auto &func : M)
    {
      runOnFunction(func);
    }

    /* Run post processing callbacks */
    if (!runPostProcessCallbacks(M))
    {
      llvm::errs() << __FUNCTION__ << " : Failed to run PostProcessing Callback for Module :" << M.getName() << '\n';
    }

    return true;
  }

  void PassRunner::runOnFunction(llvm::Function &F)
  {
    if (inExclusionList(F))
    {
      /*Do not add any instrumentation code to this function*/
      return;
    }

    /* Run pre processing callbacks */
    if (!runPreProcessCallbacks(F))
    {
      llvm::errs() << __FUNCTION__ << " : Failed to run PreProcessing Callback inside Function :" << F.getName() << '\n';
    }

    for (auto instruction_it = llvm::inst_begin(F); instruction_it != llvm::inst_end(F); instruction_it++)
    {
      llvm::Instruction *current_instruction = &(*instruction_it);
      runOnInstruction(current_instruction);
    }

    if (!runPostProcessCallbacks(F))
    {
      llvm::errs() << __FUNCTION__ << " : Failed to run PostProcessing Callback for Function :" << F.getName() << '\n';
    }
  }

  void PassRunner::runOnInstruction(llvm::Instruction *instruction)
  {

    InstructionType instruction_type = getInstructionType(instruction);

    if (m_instruction_callback.count(instruction_type) != 0)
    {

      for (auto &callback_function : m_instruction_callback[instruction_type])
      {
        callback_function(instruction);
      }
    }
  }

  InstructionType PassRunner::getInstructionType(llvm::Instruction *instruction)
  {

    if (llvm::dyn_cast<llvm::LoadInst>(instruction))
    {
      return InstructionType::Load;
    }
    if (llvm::dyn_cast<llvm::StoreInst>(instruction))
    {
      return InstructionType::Store;
    }
    if (llvm::dyn_cast<llvm::AtomicCmpXchgInst>(instruction))
    {
      return InstructionType::AtomicCmpXchg;
    }
    if (llvm::dyn_cast<llvm::AtomicRMWInst>(instruction))
    {
      return InstructionType::AtomicRMW;
    }
    if (llvm::dyn_cast<llvm::AllocaInst>(instruction))
    {
      return InstructionType::Alloca;
    }
    return InstructionType::NotSupported;
  }

  void PassRunner::registerCallback(InstructionType instruction_type, CallBack_T callback_function)
  {
    m_instruction_callback[instruction_type].emplace_back(callback_function);
  }

  bool PassRunner::inExclusionList(llvm::Module &M)
  {
    llvm::SmallString<128> RealPath = getAbsoluteModulePath(M);
    for (auto &item : m_module_exclusion_list)
    {
      if (RealPath.startswith(item))
      {
        return true;
      }
    }

    return false;
  }

  llvm::SmallString<128> PassRunner::getAbsoluteModulePath(llvm::Module &M)
  {
    std::string relFilename = M.getSourceFileName();
    llvm::SmallString<128> FilenameVec = llvm::StringRef(relFilename);
    llvm::SmallString<128> absolute_path;
    llvm::sys::fs::real_path(FilenameVec, absolute_path);

    return absolute_path;
  }

  bool PassRunner::inExclusionList(llvm::Function &F)
  {

    for (auto &func_name : m_function_exclusion_list)
    {
      if (F.getName() == func_name)
      {
        return true;
      }
    }

    return false;
  }

}

void memorypass::PassRunner::addModuleToExclusionList(const std::string &path)
{
  m_module_exclusion_list.push_back(path);
}

void memorypass::PassRunner::addFunctionToExclusionList(const std::string &func_name)
{
  m_function_exclusion_list.push_back(func_name);
}

void memorypass::PassRunner::addModulePostProcessCallback(ModulePostProcessCallback_T post_processing_callback)
{
  m_module_post_process_callbacks.push_back(post_processing_callback);
}
void memorypass::PassRunner::addModulePreProcessCallback(ModulePreProcessCallback_T pre_processing_callback)
{
  m_module_pre_process_callbacks.push_back(pre_processing_callback);
}

void memorypass::PassRunner::addFunctionPostProcessCallback(FunctionPostProcessCallback_T post_processing_callback)
{
  m_function_post_process_callbacks.push_back(post_processing_callback);
}
void memorypass::PassRunner::addFunctionPreProcessCallback(FunctionPreProcessCallback_T pre_processing_callback)
{
  m_function_pre_process_callbacks.push_back(pre_processing_callback);
}

bool memorypass::PassRunner::runPostProcessCallbacks(llvm::Module &M)
{
  bool is_successful = true;

  for (auto &callback : m_module_post_process_callbacks)
  {
    is_successful &= callback(M);
  }

  return is_successful;
}

bool memorypass::PassRunner::runPreProcessCallbacks(llvm::Module &M)
{
  bool is_successful = true;

  for (auto &callback : m_module_pre_process_callbacks)
  {
    is_successful &= callback(M);
  }

  return is_successful;
}

bool memorypass::PassRunner::runPreProcessCallbacks(llvm::Function &F)
{
  bool is_successful = true;

  for (auto &callback : m_function_pre_process_callbacks)
  {
    is_successful &= callback(F);
  }

  return is_successful;
}

bool memorypass::PassRunner::runPostProcessCallbacks(llvm::Function &F)
{
  bool is_successful = true;

  for (auto &callback : m_function_post_process_callbacks)
  {
    is_successful &= callback(F);
  }

  return is_successful;
}

char memorypass::PassRunner::s_id = 0;


// To be used when compiling a normal program
 static RegisterStandardPasses Z(
     PassManagerBuilder::EP_EnabledOnOptLevel0,
     [](const PassManagerBuilder &Builder,
        legacy::PassManagerBase &PM) { PM.add(new MemoryPass()); });
