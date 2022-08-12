#pragma once

#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Bitcode/BitcodeWriter.h"

namespace utility
{
    void dumpModuleToFile(llvm::Module &M, std::string &path)
    {
        std::error_code EC;

        // Generate all the directories in path which are not generated
        auto filename_stripped_it = path.find_last_of('/');

        // Create directory if not exists
        std::string filename_stripped_path(path.begin(), path.begin() + filename_stripped_it);
        std::string dir_gen_command = "mkdir -p ";
        dir_gen_command += filename_stripped_path;
        std::system(dir_gen_command.c_str());

        // Write the file
        llvm::raw_fd_ostream file(path, EC, llvm::sys::fs::OF_None);
        llvm::WriteBitcodeToFile(M, file);
    }

    void insertWeakDefinition(llvm::Module &M, llvm::FunctionCallee &function_callee)
    {
        llvm::Function *function = llvm::cast<llvm::Function>(function_callee.getCallee());

        function->setCallingConv(llvm::CallingConv::C);
        function->setLinkage(llvm::GlobalValue::LinkageTypes::WeakAnyLinkage);
        function->addFnAttr(llvm::Attribute::OptimizeNone);
        function->addFnAttr(llvm::Attribute::NoInline);

        llvm::BasicBlock *block = llvm::BasicBlock::Create(M.getContext(), "entry", function);
        llvm::IRBuilder<> builder(block);

        builder.CreateRet(nullptr);
    }
}
